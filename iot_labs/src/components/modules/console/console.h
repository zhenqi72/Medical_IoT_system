#ifndef __CONSOLE_H__
#define __CONSOLE_H__

#include "labs_platform.h"
#include "labs_defines.h"
#include "debug_log.h"

#define CMD_SIZE_MAX                        1024
#define CMD_NUM_MAX                         20
#define DEFAULT_CONSOLE_TASK_STACK          2048

#define CONSOLE_PWD                         "I0t"

typedef enum {
    CONSOLE_PWD_REQUIRED = 0,   /**< If a password is required 1 else 0 */
    CONSOLE_ACTIVE,             /**< If the console is active */
    CONSOLE_CLOSED              /**< If the console is closed */
}console_state_t;

typedef struct {
    const char *name;
    const char *help;
    void (*function)(int argc, char **argv);
}cmd_entry;

typedef struct {
    int state;
    thread_handle_t thread;
    char cmd_buf[CMD_SIZE_MAX];
    mutex_handle_t cmd_buf_mutex;//currently unused
    bool echo;//unused
    int uart_num;
}console_t;

/**
 * @brief Initialize a console
 * @param console Handle to the initialized console
 * @param uart_num UART to initialize the console on
 * @return 0 on success -1 on failure
 */
int console_init(console_t *console, int uart_num);
/**
 * @brief Deinitialize a console
 * @param console Handle to console to deinitialize
 * @return null
 */
void console_deinit(console_t* console);

#endif
