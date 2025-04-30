#ifndef __DEBUG_LOG_H__
#define __DEBUG_LOG_H__

#include "../../platform/esp32/port.h"

#define LOG_LV		LOG_LV_DEBUG

#define LOG_LV_DEBUG						0
#define LOG_LV_INFO							1
#define LOG_LV_WARNING						2
#define LOG_LV_ERROR						3
#define LOG_LV_FATAL						4
#define LOG_LV_OFF							5

typedef int log_lv_t;

#define LOG_PRINTF(_fmt_, ...)				printf(_fmt_"\r\n", ##__VA_ARGS__)

void log_init(void);
void log_begin(void);
void log_end(void);

#if LOG_LV <= LOG_LV_INFO
#define LOG_INFO(_fmt_, ...) do{extern log_lv_t g_log_level; if(g_log_level <= LOG_LV_INFO) {log_begin(); printf("[info] "_fmt_, ##__VA_ARGS__); log_end();}}while(0)
#else
#define LOG_INFO(_fmt_, ...) do{}while(0)
#endif

#if LOG_LV <= LOG_LV_WARNING
#define LOG_WARN(_fmt_, ...) do{extern log_lv_t g_log_level; if(g_log_level <= LOG_LV_WARNING) {log_begin(); printf("[warning] "_fmt_" (%s, line %d)", ##__VA_ARGS__, __FUNCTION__, __LINE__); log_end();}}while(0)
#else
#define LOG_WARN(_fmt_, ...) do{}while(0)
#endif

#if LOG_LV <= LOG_LV_ERROR
#define LOG_ERROR(_fmt_, ...) do{extern log_lv_t g_log_level; if(g_log_level <= LOG_LV_ERROR) {log_begin(); printf("[error] "_fmt_" (%s, line %d)", ##__VA_ARGS__, __FUNCTION__, __LINE__); log_end();}}while(0)
#else
#define LOG_ERROR(_fmt_, ...) do{}while(0)
#endif

#if LOG_LV <= LOG_LV_FATAL
#define LOG_FATAL(_fmt_, ...) do{extern log_lv_t g_log_level; if(g_log_level <= LOG_LV_FATAL) {log_begin(); printf("[fatal] "_fmt_" (%s, line %d)", ##__VA_ARGS__, __FUNCTION__, __LINE__); log_end();}}while(0)
#else
#define LOG_FATAL(_fmt_, ...) do{}while(0)
#endif

#if LOG_LV <= LOG_LV_DEBUG
#define LOG_DEBUG(_fmt_, ...) do{extern log_lv_t g_log_level; if(g_log_level <= LOG_LV_DEBUG) {log_begin(); printf("[debug] "_fmt_, ##__VA_ARGS__); log_end();}}while(0)
#else
#define LOG_DEBUG(_fmt_, ...) do{}while(0)
#endif
#endif
