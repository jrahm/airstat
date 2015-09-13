#include "blocking_queue.h"

#include <stdlib.h>
#include <stdio.h>

#include <sys/time.h>
#include <assert.h>

#ifdef __MACH__
#include <mach/clock.h>
#include <mach/mach.h>
#endif

extern FILE* logfile ;
#ifdef DEBUG
#define lprintf( fmt, ... ) \
	if( logfile ) { fprintf( logfile, fmt, ##__VA_ARGS__ ) ; fflush( logfile ) ; }
#else
#define lprintf( fmt, ... )
#endif	

#define sassert( expr, message, ... ) \
	if( ! (expr) ) { \
		fprintf( stderr, message, ##__VA_ARGS__ ) ; \
		}

static void millis_in_future( struct timespec* ts, long millis ) {
#ifdef __MACH__
	clock_serv_t cclock;
	mach_timespec_t mts;
	host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
	clock_get_time(cclock, &mts);
	mach_port_deallocate(mach_task_self(), cclock);
	ts->tv_sec = mts.tv_sec;
	ts->tv_nsec = mts.tv_nsec;
#else
	clock_gettime(CLOCK_REALTIME, ts);
#endif

/* Add 50 ms */
ts->tv_nsec += millis * 1000000;
ts->tv_sec += ts->tv_nsec / 1000000000;
ts->tv_nsec %= 1000000000;
}

/* creates a new list node that
 * has the initial data value set and
 * the next node set to null */
static struct list* new_list_node( void* data ) {
	struct list* ret = (struct list*)calloc( sizeof(struct list), 1 ) ;
	ret->data = data ;
	return ret ;
}

/*
 * construct and return a new blocking queue
 */
blocking_queue_t* new_blocking_queue( void )
{
	blocking_queue_t* ret = (blocking_queue_t*)calloc( sizeof(blocking_queue_t), 1 ) ;
	pthread_mutex_init( &ret->_m_mutex, NULL ) ;
    pthread_cond_init( &ret->_m_condition, NULL ) ;
    pthread_cond_init( &ret->_m_digest_condition, NULL ) ;

    ret->_m_head = new_list_node(NULL);
    ret->_m_tail = ret->_m_head;
	return ret ;
}

/*
 * append to the blocking queue.
 */
void blocking_queue_add( blocking_queue_t* queue, void* data )
{
	pthread_mutex_lock ( &queue->_m_mutex ) ;

    queue->_m_tail->data = data;
    queue->_m_tail->next = new_list_node(NULL);
    queue->_m_tail = queue->_m_tail->next;

    pthread_cond_signal(&queue->_m_condition);

	pthread_mutex_unlock ( &queue->_m_mutex ) ;
}

/*
 * take the next item from the queue, or wait until
 * an item comes
 */
int blocking_queue_take( blocking_queue_t* queue, void** into, uint64_t timeout )
{
	struct timespec ts;
    int rc;
    void* garbage = NULL;

	pthread_mutex_lock( &queue->_m_mutex ) ;

	if(queue->_m_head == queue->_m_tail) {
        /* There is nothing in the blocking queue right
         * now; wait until there is something */

		millis_in_future( &ts, timeout );

		/* wait for the queue to be filled again */
        rc = pthread_cond_timedwait(&queue->_m_condition, &queue->_m_mutex, &ts);
        if(rc == ETIMEDOUT) {
            rc = BQ_TIMEOUT;
            goto cleanup_and_return;
        } else if(rc != 0) {
            perror("WARN: error on timedwait");
            fprintf(stderr, "Timeout=%llu Error code: %d\n", timeout, rc);
            goto cleanup_and_return;
        }
	}
    
    assert(queue->_m_head != queue->_m_tail);

	*into = queue->_m_head->data ;

	garbage = queue->_m_head ;
    queue->_m_head = queue->_m_head->next;

    if(queue->_m_head == queue->_m_tail) {
        /* Signal that the queue is fully digested */
        pthread_cond_signal(&queue->_m_digest_condition);
    }

    rc = BQ_OK;

cleanup_and_return:
	pthread_mutex_unlock( &queue->_m_mutex ) ;
	free( garbage ) ;
	return rc ;
}

int blocking_queue_wait_digest( blocking_queue_t* queue, uint64_t timeout ) {
	struct timespec ts;
	millis_in_future( &ts, timeout );

    int ret = BQ_OK ;
    
    pthread_mutex_lock( &queue->_m_mutex ) ;
    lprintf( __FILE__ ":%d locking mutex\n", __LINE__ ) ;

    if( pthread_cond_timedwait( &queue->_m_digest_condition, &queue->_m_mutex, &ts ) == ETIMEDOUT ) {
        ret = BQ_TIMEOUT ;
    }

    lprintf( __FILE__ ":%d unlocking mutex\n", __LINE__ ) ;
    pthread_mutex_unlock( &queue->_m_mutex ) ;

    return ret ;
}
