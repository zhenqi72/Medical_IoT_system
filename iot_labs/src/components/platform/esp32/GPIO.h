#ifndef __PLT_LED_H__
#define __PLT_LED_H__

#include "port.h"

#define LED1_BIT GPIO_SEL_17
#define LED2_BIT GPIO_SEL_18
#define LED3_BIT GPIO_SEL_19
#define LED4_BIT GPIO_SEL_23
#define LED5_BIT GPIO_SEL_5
#define LED6_BIT GPIO_SEL_26
#define LED7_BIT GPIO_SEL_27
#define LED8_BIT GPIO_SEL_0

#define SW1_BIT GPIO_SEL_34
#define SW2_BIT GPIO_SEL_35
#define SW3_BIT GPIO_SEL_32
#define SW4_BIT GPIO_SEL_33
#define SW5_BIT GPIO_SEL_25

#define LEFT GPIO_NUM_34
#define UP GPIO_NUM_35
#define RIGHT GPIO_NUM_32
#define DOWN GPIO_NUM_33
#define ENTER GPIO_NUM_25

typedef enum {
    plt_LED1 = GPIO_NUM_17,
    plt_LED2 = GPIO_NUM_18,
    plt_LED3 = GPIO_NUM_23,
    plt_LED4 = GPIO_NUM_19,
    plt_LED5 = GPIO_NUM_5,
    plt_LED6 = GPIO_NUM_26,
    plt_LED7 = GPIO_NUM_27,
    plt_LED8 = GPIO_NUM_0
} plt_led_t;

typedef enum {
    LED_OFF = 0,
    LED_ON
} ledState_t;

void plt_init_leds(void);
plt_led_t ledToPltLed(uint8_t);
void plt_init_leds(void);
void plt_led_off(uint8_t);
void plt_led_on(uint8_t);

// Switches
typedef enum {
    plt_SW1 = GPIO_NUM_34,
    plt_SW2 = GPIO_NUM_35,
    plt_SW3 = GPIO_NUM_32,
    plt_SW4 = GPIO_NUM_33,
    plt_SW5 = GPIO_NUM_25
} plt_sw_t;
void plt_init_sw(void);
void IRAM_ATTR plt_sw_isr_handler(void*);
extern void sw_isr_handler(void*);
void plt_sw_debounce(void*);
#endif