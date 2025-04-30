/**
 * File Name: led.c
 * Description: This file contains source code for the LED driver.
 */

/***************************************************************************************************/
/* Include Files */
/***************************************************************************************************/
#include "led.h"
/***************************************************************************************************/
/* Private Data Types */
/***************************************************************************************************/

/***************************************************************************************************/
/* Private Variables(static) */
/***************************************************************************************************/
/*This is an example description for constants and variables. Delete this if it is unused here and copy and paste it to where it is needed. */
/**
 * @brief This is a brief description
 */

/***************************************************************************************************/
/* Public Variable Definitions */
/***************************************************************************************************/

/***************************************************************************************************/
/* Private Function Prototypes(static) */
/***************************************************************************************************/
/*This is an example description for function prototypes. Delete this if it is unused here and copy and paste it to where it is needed. */
/**
 * @brief This is a brief description
 * @param Input_Param_Name Description of input parameter
 * @return Description of return value
 */

/***************************************************************************************************/
/* Public Function Definitions */
/***************************************************************************************************/
void led_init()
{
    plt_init_leds();
}

void led_display_num(uint8_t num)
{
    for (size_t i = 0; i < 8; i++)
    {
        if(num & (1 << i))
        {
            plt_led_on(i);
        }
        else
        {
            plt_led_off(i);
        }
    }
    
}
void led_on(led_t led)
{
    plt_led_on((uint8_t)led);
}
void led_off(led_t led)
{
    plt_led_off((uint8_t)led);
}

/***************************************************************************************************/
/* Private Function Definitions */
/***************************************************************************************************/