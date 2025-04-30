#include "os_func.h"
#include "debug_log.h"

#define MS2TICK(ms)      ((ms)/portTICK_PERIOD_MS)
#define TICK2MS(tick)    ((tick)*portTICK_PERIOD_MS)

#define TIMER_OPT_BLOCK_MS_MAX		10*1000

static uint32_t tick_h = 0;
static bool over_half = false;

uint32_t tick_now( void )
{
	uint32_t tick = xTaskGetTickCount();

	if(tick > 0x7FFFFFFF)
		over_half = true;

	else if(true == over_half){
		over_half = false;
		tick_h++;
	}

	return tick = xTaskGetTickCount();;
}

void  tick_sleep( uint32_t tick )
{
	vTaskDelay( tick );
}

uint32_t tick_elapsed(uint32_t last_tick)
{
	uint32_t now = tick_now();
	
	if(last_tick  < now)
		return now - last_tick;
	else
		return 0xFFFFFFFF - last_tick + now + 1;
}

uint32_t ms_now( void )
{
    return (TICK2MS(tick_now()));
}


void  ms_sleep( uint32_t ms )
{
	vTaskDelay( WAIT_MS2TICK(ms) );
}

uint32_t ms_elapsed(uint32_t last_ms)
{
    return (TICK2MS(tick_elapsed(MS2TICK(last_ms))));
}

static int32_t time_tick_diff = 0;

//thread
int thread_create(thread_handle_t* pthread, const char* name, function_cb_t (*function)(void* arg), uint32_t stack_size, void* arg, int priority )
{
	if(PRIORITY_DEFAULT == priority){
		priority = PRIORITY_DEFAULT;
	}

	if( pdPASS == xTaskCreate( (pdTASK_CODE)function, (const char* const)name, (unsigned short) (stack_size/sizeof( portSTACK_TYPE )),
			arg, priority, pthread ) ){
		return 0;
	}

	return -1;
}


int thread_delete(thread_handle_t thread)
{
	if(NULL == thread)
		LOG_DEBUG("Task handle %x deleted", (uint32_t)xTaskGetCurrentTaskHandle());
	else
		LOG_DEBUG("Task handle %x deleted", (uint32_t)thread);

    vTaskDelete(thread);
	return 0;
}

int mbox_new(mbox_handle_t* pmbox, int size )
{
	*pmbox = xQueueCreate(size, (uint32_t)sizeof(void *) );
	if(NULL != *pmbox )
		return 0;
	else
		return -1;
}

int mbox_free(mbox_handle_t mbox )
{
	if(mbox){
		vQueueDelete(mbox);
	}

	return 0;
}

int mbox_post(mbox_handle_t mbox, void *msg, uint32_t wait_ms)
{
	if(NULL == mbox){
		return -1;
	}
	int ret = xQueueSend(mbox, &msg, WAIT_MS2TICK(wait_ms));
	return ret == pdTRUE ? 0 : -1;
}

int mbox_fetch(mbox_handle_t mbox, void **msg, uint32_t wait_ms)
{
	if(NULL == mbox){
		return -1;
	}

	int ret = xQueueReceive(mbox, msg, WAIT_MS2TICK(wait_ms) );

	if (pdPASS == ret )
		return 0;
	else if(wait_ms == 0)
		return -1;
	else
		return -1;
}

//semaphore
int semaphore_create(semaphore_handle_t* pmhandle)
{
	*pmhandle = xSemaphoreCreateMutex();
	if (*pmhandle) {
		return 0;
	}
	else {
		return -1;
	}
}

int semaphore_create_counting(semaphore_handle_t* pmhandle, uint32_t maxcount, uint32_t initcount)
{
	*pmhandle = xSemaphoreCreateCounting(maxcount, initcount);
	if (*pmhandle) {
		return 0;
	}
	else {
		return -1;
	}
}

int semaphore_get(semaphore_handle_t mhandle, uint32_t wait_ms)
{
	int ret = xSemaphoreTake(mhandle, WAIT_MS2TICK(wait_ms));

	return ret == pdTRUE ? 0 : -1;
}

int semaphore_put(semaphore_handle_t mhandle)
{
	int ret = xSemaphoreGive(mhandle);

	return ret == pdTRUE ? 0 : -1;
}

int semaphore_getcount(semaphore_handle_t mhandle)
{
	return uxQueueMessagesWaiting(mhandle);
}


int semaphore_delete(semaphore_handle_t mhandle)
{
	vSemaphoreDelete(mhandle);
	return 0;
}

//mutex
int mutex_create(mutex_handle_t* phandle)
{
	*phandle = xSemaphoreCreateMutex();
	if(*phandle){
		return 0;
	}
	else{
		return -1;
	}
}

int mutex_delete(mutex_handle_t handle)
{
	vSemaphoreDelete(handle);
	return 0;
}

int mutex_put(mutex_handle_t handle)
{
	int ret = xSemaphoreGive(handle);

	return ret == pdTRUE ? 0 : -1;
}

int mutex_get(mutex_handle_t handle, uint32_t wait_ms)
{
	int ret = xSemaphoreTake(handle, WAIT_MS2TICK(wait_ms));

	return ret == pdTRUE ? 0 : -1;
}

//queue
int queue_create(queue_handle_t *pqhandle, uint32_t q_len, uint32_t item_size)
{
    *pqhandle = xQueueCreate(q_len, item_size);
    if(*pqhandle){
        return 0;
    }
    else{
        return -1;
    }
}

int queue_delete(queue_handle_t qhandle)
{
    vQueueDelete(qhandle);
    return 0;
}

int queue_send(queue_handle_t qhandle, const void *msg, uint32_t wait_ms)
{
    int ret = xQueueSendToBack(qhandle, msg, WAIT_MS2TICK(wait_ms));

    return ret == pdTRUE ? 0 : -1;
}

int queue_overwrite(queue_handle_t qhandle, const void* msg)
{
	int ret = xQueueOverwrite(qhandle, msg);
	return ret == pdTRUE ? 0 : -1;
}

int queue_recv(queue_handle_t qhandle, void *msg, uint32_t wait_ms)
{
    int ret = xQueueReceive(qhandle, msg, WAIT_MS2TICK(wait_ms));

    return ret == pdTRUE ? 0 : -1;
}

int queue_get_msgs_waiting(queue_handle_t qhandle)
{
    int nmsg = 0;
    nmsg = uxQueueMessagesWaiting(qhandle);
    return nmsg;
}
int queue_peek(queue_handle_t qhandle, void* msg, uint32_t wait_ms)
{
	int ret = xQueuePeek(qhandle, msg, WAIT_MS2TICK(wait_ms));
	return ret == pdTRUE ? 0 : -1;
}
//timer
int timer_create(timer_handle_t* ptimer, const char *name, uint32_t ms,
		    void (*call_back)(timer_handle_t), void *context,
		    timer_reload_t reload,
		    timer_activate_t activate)
{
	int auto_reload = (reload == TIMER_ONE_SHOT) ? pdFALSE : pdTRUE;
	uint32_t ticks = WAIT_MS2TICK(ms);

	*ptimer = xTimerCreate((const char * const)name, ticks,
				      auto_reload, context, call_back);
	if (*ptimer == NULL)
		return -1;

	if (activate == TIMER_AUTO_ACTIVATE){
		if(pdTRUE != xTimerStart(*ptimer, ticks)){
			xTimerDelete(*ptimer, MS2TICK(TIMER_OPT_BLOCK_MS_MAX));
			*ptimer = NULL;
			return -1;
		}
	}

	return 0;
}

int timer_activate(timer_handle_t timer)
{
	int ret = xTimerStart(timer, MS2TICK(TIMER_OPT_BLOCK_MS_MAX));

	return ret == pdPASS ? 0 : -1;
}

int timer_change(timer_handle_t timer, uint32_t ms)
{
	int ret;
	uint32_t ticks = WAIT_MS2TICK(ms);
	ret = xTimerChangePeriod(timer, ticks, MS2TICK(TIMER_OPT_BLOCK_MS_MAX));

	return ret == pdPASS ? 0 : -1;
}

int timer_is_active(timer_handle_t timer)
{
	int ret;

	ret = xTimerIsTimerActive(timer);

	return ret == pdPASS ? 0 : -1;
}

void *timer_get_context(timer_handle_t timer)
{
	return (void*)pvTimerGetTimerID(timer);
}

int timer_reset(timer_handle_t timer)
{
	int ret = xTimerReset(timer, MS2TICK(TIMER_OPT_BLOCK_MS_MAX));

	return ret == pdPASS ? 0 : -1;
}

int timer_deactivate(timer_handle_t timer)
{
	int ret = xTimerStop(timer, MS2TICK(TIMER_OPT_BLOCK_MS_MAX));

	return ret == pdPASS ? 0 : -1;
}


int timer_delete(timer_handle_t timer)
{
	int ret;
	ret = xTimerDelete(timer, MS2TICK(TIMER_OPT_BLOCK_MS_MAX));
	if(pdPASS != ret){
	}

	return ret == pdPASS ? 0 : -1;
}

static void aysnc_callback(timer_handle_t timer)
{
	struct{
		void (*cb)(void*);
		void *cb_arg;
	}*async_ctx = timer_get_context(timer);

	if(async_ctx){
		async_ctx->cb(async_ctx->cb_arg);
		free(async_ctx);
	}

	timer_delete(timer);
}

int async_call(void (*cb)(void* cb_arg), void *cb_arg, int delay_ms)
{
	if(NULL == cb)
		return -1;

	struct{
		void (*cb)(void* cb_arg);
		void *cb_arg;
	}*async_ctx = malloc(2*sizeof(void *));

	async_ctx->cb = cb;
	async_ctx->cb_arg = cb_arg;

	int ret;
	timer_handle_t async_timer;
	ret = timer_create(&async_timer, "asyncCallTimer", delay_ms, aysnc_callback, async_ctx, TIMER_ONE_SHOT, TIMER_AUTO_ACTIVATE);

	if(0 != ret){
		free(async_ctx);
	}

	return ret;
}

void reboot(void)
{
	esp_restart();
}

static void _reboot_async(timer_handle_t timer)
{
	timer_delete(timer);
	esp_restart();
}

void reboot_async(int delay_ms)
{
	timer_handle_t async_timer;

	if(0 != timer_create(&async_timer, "asyncRebootTimer", delay_ms, _reboot_async, NULL, TIMER_ONE_SHOT, TIMER_AUTO_ACTIVATE)){
		esp_restart();
	}
}

struct tm local_now(void)
{
	struct tm tm;
    struct timeval now;
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;
    localtime_r(&t, &tm);
	return tm;
}

void time_tune(uint32_t std_time)
{
	// uint32_t now = time_now();
	uint32_t now = 0;
	int32_t diff = 0;

	if(std_time > now){	
		uint32_t slow = std_time - now;	
		diff = (slow * configTICK_RATE_HZ / now) * (-1);
		if(diff == 0)diff = -2;
	}
	else{
		uint32_t quick = now - std_time;
		diff = (quick * configTICK_RATE_HZ / now);
		if(diff == 0)diff = 2;
	}

	time_tick_diff += diff/2;

}

static uint32_t utc_offset = 0;

struct tm utc_now(void)
{
	time_t rawtime;
	struct tm* info;
	time(&rawtime);
	info = gmtime(&rawtime);
	return *info;
}

void utc_set()
{
	setenv("TZ", "EST5EDT,M3.2.0/2,M11.1.0", 1);
    tzset();
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    sntp_setservername(0, "pool.ntp.org");
    sntp_init();

}

int utc_parse(uint32_t ts, uint16_t* year, uint8_t* month, uint8_t* day, uint8_t* hour, uint8_t* min, uint8_t* sec)
{
#define SECONDS_IN_365_DAY_YEAR  (31536000)
#define SECONDS_IN_A_DAY         (86400)
#define SECONDS_IN_A_HOUR        (3600)
#define SECONDS_IN_A_MINUTE      (60)
	uint32_t secondsPerMonth[12] = {
		31*SECONDS_IN_A_DAY,
		28*SECONDS_IN_A_DAY,
		31*SECONDS_IN_A_DAY,
		30*SECONDS_IN_A_DAY,
		31*SECONDS_IN_A_DAY,
		30*SECONDS_IN_A_DAY,
		31*SECONDS_IN_A_DAY,
		31*SECONDS_IN_A_DAY,
		30*SECONDS_IN_A_DAY,
		31*SECONDS_IN_A_DAY,
		30*SECONDS_IN_A_DAY,
		31*SECONDS_IN_A_DAY,
	};

	uint32_t time = ts;

	/* Calculate year */
	uint32_t tmp = 1970 + time/SECONDS_IN_365_DAY_YEAR;
	uint32_t number_of_leap_years = (tmp - 1968 - 1)/4;
	time = time - ((tmp-1970)*SECONDS_IN_365_DAY_YEAR);
	if ( time > (number_of_leap_years*SECONDS_IN_A_DAY))
	{
		time = time - (number_of_leap_years*SECONDS_IN_A_DAY);
	}
	else
	{
		time = time - (number_of_leap_years*SECONDS_IN_A_DAY) + SECONDS_IN_365_DAY_YEAR;
		--tmp;
	}

	if(year)
		*year = tmp;

	/* Remember if the current year is a leap year */
	uint8_t  is_a_leap_year = ((tmp - 1968)%4 == 0);

	/* Calculate month */
	tmp = 1;
	int a;
	for (a = 0; a < 12; ++a){

		uint32_t seconds_per_month = secondsPerMonth[a];
		/* Compensate for leap year */
		if ((a == 1) && is_a_leap_year)
			seconds_per_month += SECONDS_IN_A_DAY;

		if (time > seconds_per_month){
			time -= seconds_per_month;
			tmp += 1;
		}else break;
	}

	if(month)*month = tmp;

	/* Calculate day */
	tmp = time/SECONDS_IN_A_DAY;
	time = time - (tmp*SECONDS_IN_A_DAY);
	++tmp;
	if(day)*day = tmp;


	/* Calculate hour */
	tmp = time/SECONDS_IN_A_HOUR;
	time = time - (tmp*SECONDS_IN_A_HOUR);
	if(hour)*hour = tmp;

	/* Calculate minute */
	tmp = time/SECONDS_IN_A_MINUTE;
	time = time - (tmp*SECONDS_IN_A_MINUTE);
	if(min)*min = tmp;

	/* Calculate seconds */
	if(sec)*sec = time;

	return 0;
}
