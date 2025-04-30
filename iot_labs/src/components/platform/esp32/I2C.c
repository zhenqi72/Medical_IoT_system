#include "I2C.h"


int i2c_init(int num)
{
    switch (num)
    {
    case I2C_NUM_0:
    {
        i2c_config_t i2c_config = {
            .mode = I2C_MODE_MASTER,
            .sda_io_num = GPIO_NUM_21,
            .scl_io_num = GPIO_NUM_22,
            .scl_pullup_en = false,
            .sda_pullup_en = false,
            .master = {
                .clk_speed = 100e3,
            },
        };
        if(ESP_OK != i2c_param_config(num, &i2c_config))
        {
            LOG_ERROR("Failed to configure i2c driver.");
            return -1;
        }
        if(ESP_OK != i2c_driver_install(num, I2C_MODE_MASTER, 0, 0, 0))
        {
            LOG_ERROR("Failed to install i2c driver.");
            return -1;
        }
    }
        break;
    case I2C_NUM_1:
    {
        i2c_config_t i2c_config = 
        {
            .mode 				= I2C_MODE_MASTER,
            .sda_io_num			= GPIO_NUM_4,
            .sda_pullup_en		= GPIO_PULLUP_ENABLE,
            .scl_io_num			= GPIO_NUM_2,
            .scl_pullup_en		= GPIO_PULLUP_ENABLE,
            .master.clk_speed	= 400000,
        };
        if(ESP_OK != i2c_param_config(num, &i2c_config))
        {
            LOG_ERROR("Failed to configure i2c driver.");
            return -1;
        }
        if(ESP_OK != i2c_driver_install(num, I2C_MODE_MASTER, 0, 0, 0))
        {
            LOG_ERROR("Failed to install i2c driver.");
            return -1;
        }
    }
        break;
    
    default:
        break;
    }
    return 0;
}