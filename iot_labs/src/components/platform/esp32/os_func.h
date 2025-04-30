#ifndef _OS_H_
#define _OS_H_

#include "port.h"

#define MS2TICK(ms)       ((ms)/portTICK_PERIOD_MS)
#define TICK2MS(tick )    ((tick)*portTICK_PERIOD_MS)

#define PRIORITY_DEFAULT 			(-1)
#define WAIT_FOREVER				(0xFFFFFFFF)
#define NO_WAIT 					(0)
#define WAIT_MS2TICK(ms)			(((ms) == WAIT_FOREVER) ? WAIT_FOREVER : (((ms)/portTICK_PERIOD_MS)+((((ms)%portTICK_PERIOD_MS)+portTICK_PERIOD_MS-1)/portTICK_PERIOD_MS)))

typedef void * 						thread_handle_t;
typedef void *						queue_handle_t;
typedef void *		 				timer_handle_t;
typedef void *						semaphore_handle_t;
typedef void * 						mutex_handle_t;
typedef void *						mbox_handle_t;
typedef void *						function_cb_t;
#define FUNCTION_CB(v)  		((function_cb_t)(v))


#define thread_self() 				xTaskGetCurrentTaskHandle()

thread_handle_t thread_current(void);
/**
 * @brief Creates a thread
 * @param pthread handle for the thread created
 * @param name Name of the thread
 * @param function The function to run for the thread
 * @param stack_size Stack size for the thread
 * @param arg 
 * @param priority Thread's priority
 * @return 0 on success -1 on failure
 */
int thread_create(thread_handle_t* pthread, const char* name, function_cb_t (*function)(void*), uint32_t stack_size, void* arg, int priority);
/**
 * @brief Delete a thread
 * @param thread The thread to delete
 * @return 0 on success -1 on failure
 */
int thread_delete(thread_handle_t thread);
/**
 * @brief Safely exit a thread
 * @warning Not implemented
 */
void thread_finish(void);

/**
 * @brief Create a new queue (mailbox)
 * @param pmbox Pointer to the new mailbox
 * @param size Size of the new mailbox
 * @return 0 on success -1 on failure
 */
int mbox_new(mbox_handle_t* pmbox, int size );
/**
 * @brief Delete a mailbox
 * @param mbox Pointer to the mailbox
 * @return 0 on success -1 on failure
 */
int mbox_free(mbox_handle_t mbox );
/**
 * @brief Add message to mailbox
 * @param mbox Pointer to the mailbox
 * @param msg Message to add to the mailbox
 * @param wait_ms The maximum amount of time the task should block waiting for space to become available on the mailbox.
 * @return 0 on success -1 on failure
 */
int mbox_post(mbox_handle_t mbox, void *msg, uint32_t wait_ms);
/**
 * @brief Get message from mailbox
 * @param mbox Pointer to the mailbox
 * @param msg Pointer to the buffer into which the received item will be copied.
 * @param wait_ms The maximum amount of time the task should block waiting for data to become available in the mailbox.
 * @return 0 on success -1 on failure
 */
int mbox_fetch(mbox_handle_t mbox, void **msg, uint32_t wait_ms);

/**
 * @brief Enter critical section
 */
void enter_critical(void);
/**
 * @brief Exit critical section
 */
void exit_critical(void);

/**
 * @brief Create a new mutex
 * @param phandle Pointer to the mutex
 * @return 0 on success -1 on failure
 */
int mutex_create(mutex_handle_t* phandle);
/**
 * @brief Delete a mutex
 * @param handle Mutex to delete
 * @return 0 on success -1 on failure
 */
int mutex_delete(mutex_handle_t handle);
/**
 * @brief Release a mutex
 * @param handle Mutex to release
 * @return 0 on success -1 on failure
 */
int mutex_put(mutex_handle_t handle);
/**
 * @brief Get a mutex
 * @param handle Mutex to get
 * @param wait_ms The time in ticks to wait for the semaphore to become available
 * @return 0 on success -1 on failure
 */
int mutex_get(mutex_handle_t handle, uint32_t wait_ms);

/**
 * @brief Creates a new counting semaphore instance
 * @param pmhandle Pointer the the created semaphore
 * @param maxcount The maximum count value that can be reached
 * @param initcount The count value assigned to the semaphore when it is created
 * @return 0 on success -1 on failure
 */
int semaphore_create_counting(semaphore_handle_t* pmhandle, uint32_t maxcount, uint32_t initcount);
/**
 * @brief Create a new mutex
 * @param phandle Pointer to the mutex
 * @return 0 on success -1 on failure
 */
int semaphore_create(semaphore_handle_t* mhandle);
/**
 * @brief Get semaphore
 * @param mhandle A handle to the semaphore being taken - obtained when the semaphore was created
 * @param wait_ms The time in ticks to wait for the semaphore to become available
 * @return 0 on success -1 on failure
 */
int semaphore_get(semaphore_handle_t mhandle, uint32_t wait_ms);
/**
 * @brief Release semaphore
 * @param mhandle A handle to the semaphore being released - obtained when the semaphore was created
 * @return 0 on success -1 on failure
 */
int semaphore_put(semaphore_handle_t mhandle);
/**
 * @brief Delete semaphore
 * @param mhandle A handle to the semaphore being deleated.
 * @return 0 on success -1 on failure
 */
int semaphore_delete(semaphore_handle_t mhandle);
/**
 * @brief Returns milliseconds elapsed sense last_ms
 * @param last_ms Time to calculate difference from
 * @return \p last_ms - current time
 */
uint32_t ms_elapsed(uint32_t last_ms);

/**
 * @brief Starts a timer
 * @param timer The timer to start
 * @return 0 on success -1 on failure
 */
int timer_activate(timer_handle_t timer);
/**
 * @brief Changes the period of the timer
 * @param timer The timer to change
 * @param ms The new period value in milliseconds
 * @return 0 on success -1 on failure
 */
int timer_change(timer_handle_t timer, uint32_t ms);
/**
 * @brief Checks to see if the timer is active
 * @param timer The timer to check
 * @return 0 if active -1 if inactive
 */
int timer_is_active(timer_handle_t timer);
/**
 * @brief Get ID assigned to timer
 * @param timer The timer to query
 * @return Timer's ID
 */
void *timer_get_context(timer_handle_t timer);
/**
 * @brief Re-starts a timer that was previously created
 * @param timer Timer to re-start
 * @return 0 on success -1 on failure
 */
int timer_reset(timer_handle_t timer);
/**
 * @brief Stops a timer that was previously started
 * @param timer Timer to stop
 * @return 0 on success -1 on failure
 */
int timer_deactivate(timer_handle_t timer);
/**
 * @brief Deletes a timer that was previously created
 * @param timer The handle of the timer being deleted
 * @return 0 on success -1 on failure
 */
int timer_delete(timer_handle_t timer);
/**
 * @brief Creates a new queue
 * 
 * @param pqhandle Handle to the created queue
 * @param q_len Size of the queue
 * @param item_size Size of the items stored in the queue
 * @return int 0 on Success -1 on Failure
 */
int queue_create(queue_handle_t* pqhandle, uint32_t q_len, uint32_t item_size);
/**
 * @brief Get a message from the queue
 * @param qhandle Queue to get message from
 * @param msg Pointer to copy data into
 * @param wait_ms Amount of time to wait for information to become available in the queue
 * @return 0 on success -1 on failure
 */
int queue_recv(queue_handle_t qhandle, void *msg, uint32_t wait_ms);
/**
 * @brief Delete a queue
 * @param qhandle Handle of queue to delete
 * @return 0 on success -1 on failure
 */
int queue_delete(queue_handle_t qhandle);
/**
 * @brief Post data to queue
 * @param qhandle Handle of queue to post data to
 * @param msg Pointer to the data to post
 * @param wait_ms Time to wait in milliseconds for the queue to be able to accept the data
 * @return 0 on success -1 on failure
 */
int queue_send(queue_handle_t qhandle, const void *msg, uint32_t wait_ms);
/**
 * @brief Post data to queue overwrite if full
 * Only for use with queues that have a length of one - so the queue is either empty or full.
 * Post an item on a queue. If the queue is already full then overwrite the value held in the queue. The item is queued by copy, not by reference.
 * This function must not be called from an interrupt service routine.
 * @param qhandle Handle of queue to post data to
 * @param msg Pointer to the data to post
 * @return int 0 on success -1 on failure
 */
int queue_overwrite(queue_handle_t qhandle, const void* msg);
/**
 * @brief Get the number of messages waiting in the queue
 * @param qhandle Handle of queue to check
 * @return Number of messages in the queue
 */
int queue_get_msgs_waiting(queue_handle_t qhandle);
/**
 * @brief Read message in queue without removing it from the queue
 * 
 * @param qhandle handle of the queue to check
 * @param msg message in the queue
 * @return int 0 on success -1 on failure
 */
int queue_peek(queue_handle_t qhandle, void* msg, uint32_t wait_ms);
/**
 * @brief Create a asynchronus thread
 * @param cb Function to be called
 * @param cb_arg Pointer the the callback arguments
 * @param delay_ms Time to wait inbetween cb calls
 * @return 0 on success -1 on failure
 */
int async_call(void (*cb)(void* cb_arg), void *cb_arg, int delay_ms);
/**
 * @brief restarts the CPU
 * @details Peripherials (except for WiFi, BT, UART0, SPI1, and legacy timers) are not reset. This function does not return.
 */
void reboot(void);
/**
 * @brief restarts the CPU after a defined amount of time
 * @details Peripherials (except for WiFi, BT, UART0, SPI1, and legacy timers) are not reset. This function does not return.
 * @param delay_ms Time to wait until restarting the CPU
 */
void reboot_async(int delay_ms);

typedef enum os_timer_reload {
	TIMER_ONE_SHOT,
	TIMER_PERIODIC,
} timer_reload_t;

typedef enum os_timer_activate {
	TIMER_AUTO_ACTIVATE,
	TIMER_NO_ACTIVATE,
} timer_activate_t;

/**
 * @brief Create a timer
 * @param ptimer Pointer to the timer that is created
 * @param name NULL terminated char arry (string) That holds the name for the timer
 * @param ms Period of the timer
 * @param call_back Pointer to the function to call at the end of the timer's period
 * @param context An identifier that is assigned to the timer being created.
 * Typically this would be used in the timer callback function to identify which
 * timer expired when the same callback function is assigned to more than one
 * timer.
 * @param reload What kind of timer to create (TIMER_ONE_SHOT, TIMER_PERIODIC)
 * @param activate (TIMER_AUTO_ACTIVATE TIMER_NO_ACTIVATE)
 * @return 0 on success -1 on failure
 */
int timer_create(timer_handle_t *ptimer, const char *name, uint32_t ms,
		    void (*call_back)(timer_handle_t), void *context,
		    timer_reload_t reload,
		    timer_activate_t activate);

/**
 * @brief Return the local time
 * @return A struct containing the time
 */
 struct tm local_now(void);
/**
 * @brief Change the time
 * @param std_time Amount of time to offset the current time
 * @return null
 */
void time_tune(uint32_t std_time);

/**
 * @brief Get the current time in UTC
 * @return The UTC time
 */
struct tm utc_now();
/**
 * @brief Initialize system time
 */
void utc_set(void);
/**
 * @brief Given a time calculate the date down to the second
 * @param ts Time to calculate from
 * @param year The year that is calculated
 * @param month The month that is calculated
 * @param day The day that is calculated 
 * @param hour The hour that is calculated
 * @param min The minute that is calculated
 * @param sec The second that is calculated
 * @return 0 on success -1 on failure
 */
int utc_parse(uint32_t ts, uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec);

/**
 * @brief Get the current tick
 * @param null
 * @return The tick value
 */
uint32_t tick_now( void );
/**
 * @brief Sleep the task for the specifed number of ticks
 * @param tick The amount of ticks to sleep the task for
 * @return null
 */
void  tick_sleep( uint32_t tick );
/**
 * @brief Get the difference between ticks
 * @param last_tick The tick to subract from the current tick
 * @return The difference between now and \a last_tick
 */
uint32_t tick_elapsed(uint32_t last_tick);

/**
 * @brief Get the amount of miliseconds from ticks
 * @param null
 * @return The amount of miliseconds
 */
uint32_t ms_now( void );
/**
 * @brief Sleep the task for the specfed number of milliseconds
 * @param ms Amount of milliseconds to sleep
 * @return null
 */
void  ms_sleep( uint32_t ms );
/**
 * @brief Get the amount of elapsed time in milliseconds
 * @param last_ms The ms to subract from the current ms
 * @return The difference between now and last_ms
 */
uint32_t ms_elapsed(uint32_t last_ms);

#endif
