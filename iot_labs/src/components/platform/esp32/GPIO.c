#include "GPIO.h"
#include "../../debug/debug_log.h"

void plt_init_leds(void)
{
    gpio_config_t config = {
        .pin_bit_mask = LED1_BIT,
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&config);
    config.pin_bit_mask = LED2_BIT;
    gpio_config(&config);
    config.pin_bit_mask = LED3_BIT;
    gpio_config(&config);
    config.pin_bit_mask = LED4_BIT;
    gpio_config(&config);
    config.pin_bit_mask = LED5_BIT;
    gpio_config(&config);
    config.pin_bit_mask = LED6_BIT;
    gpio_config(&config);
    config.pin_bit_mask = LED7_BIT;
    gpio_config(&config);
    config.pin_bit_mask = LED8_BIT;
    gpio_config(&config);
}
void plt_led_on(uint8_t led)
{
    gpio_set_level(ledToPltLed(led), LED_ON);
}
void plt_led_off(uint8_t led)
{
    gpio_set_level(ledToPltLed(led), LED_OFF);
}
plt_led_t ledToPltLed(uint8_t led)
{
    switch (led)
    {
    case 0:
        return plt_LED1;
    case 1:
        return plt_LED2;
    case 2:
        return plt_LED3;
    case 3:
        return plt_LED4;
    case 4:
        return plt_LED5;
    case 5:
        return plt_LED6;
    case 6:
        return plt_LED7;
    case 7:
        return plt_LED8;
    default:
        break;
    }
    return 0;
}

// Switches

void plt_init_sw(void)
{
    gpio_config_t config;
    //interrupt of rising edge
    config.intr_type = GPIO_PIN_INTR_POSEDGE;
    //bit mask of the pins
    config.pin_bit_mask = SW1_BIT;
    //set as input mode    
    config.mode = GPIO_MODE_INPUT;
    //disable pull-up mode
    config.pull_up_en = 0;
    gpio_config(&config);
    config.pin_bit_mask = SW2_BIT;
    gpio_config(&config);
    config.pin_bit_mask = SW3_BIT;
    gpio_config(&config);
    config.pin_bit_mask = SW4_BIT;
    gpio_config(&config);
    config.pin_bit_mask = SW5_BIT;
    gpio_config(&config);
    //install gpio isr service
    gpio_install_isr_service(0);
    //hook isr handler for each gpio pin
    gpio_isr_handler_add(plt_SW1, plt_sw_isr_handler, (void*) plt_SW1);
    gpio_isr_handler_add(plt_SW2, plt_sw_isr_handler, (void*) plt_SW1);
    gpio_isr_handler_add(plt_SW3, plt_sw_isr_handler, (void*) plt_SW3);
    gpio_isr_handler_add(plt_SW4, plt_sw_isr_handler, (void*) plt_SW4);
    gpio_isr_handler_add(plt_SW5, plt_sw_isr_handler, (void*) plt_SW5);
}

void IRAM_ATTR plt_sw_isr_handler(void* button)
{
    sw_isr_handler(button);
}