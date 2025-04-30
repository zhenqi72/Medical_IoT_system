#include "debug_log.h"
#include "os_func.h"
#include <time.h>
//TODO: add lock on log output
log_lv_t g_log_level = LOG_LV_DEBUG;
static mutex_handle_t s_log_mutex = NULL;

void log_init(void)
{
	mutex_create(&s_log_mutex);
}

void log_begin(void)
{
    struct tm tm;
    time_t current;

    current = time(NULL);
    gmtime_r((const time_t *) &current, &tm);

    if(s_log_mutex) mutex_get(s_log_mutex, WAIT_FOREVER);

   	printf("%02d:%02d:%02d.%03d ", tm.tm_hour, tm.tm_min, tm.tm_sec, ms_now()%1000);
}

void log_end(void)
{
	printf("\r\n");

	if(s_log_mutex) mutex_put(s_log_mutex);
}