#include "switches.h"

void sw_init(void)
{
    plt_init_sw();
}

void sw_isr_handler(void* button)
{
    // LOG_DEBUG("Button %d pressed.", (uint8_t)button);
}