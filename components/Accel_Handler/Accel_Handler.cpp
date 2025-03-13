#include <stdio.h>
#include "Accel_Handler.h"
#include "LIS331.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"

LIS331 x1;
float X_OFFSET = 0;
float Y_OFFSET = 0;
float Z_OFFSET = 0;

void init_accel() {
    i2c_master_bus_config_t i2c_mst_config = {
        .i2c_port = I2C_NUM_0,
        .sda_io_num = GPIO_NUM_0,
        .scl_io_num = GPIO_NUM_1,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
    };
    i2c_mst_config.flags.enable_internal_pullup = true;

    i2c_master_bus_handle_t bus_handle;
    
    i2c_new_master_bus(&i2c_mst_config, &bus_handle);

    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = ACCEL_I2C_ADDRESS,
        .scl_speed_hz = 100000,
    };
    
    i2c_master_dev_handle_t dev_handle;
    i2c_master_bus_add_device(bus_handle, &dev_cfg, &dev_handle);

    x1.setI2CAddr(ACCEL_I2C_ADDRESS);
    x1.setI2CHandler(dev_handle);

    x1.begin(LIS331::USE_I2C);

    x1.setFullScale(ACCEL_RANGE);
}

void get_g(float &x, float &y, float &z) {

    int16_t xt, yt, zt;
    x1.readAxes(xt, yt, zt);

    x = x1.convertToG(ACCEL_MAX_SCALE, xt) - X_OFFSET;
    y = x1.convertToG(ACCEL_MAX_SCALE, yt) - Y_OFFSET;
    z = x1.convertToG(ACCEL_MAX_SCALE, zt) - Z_OFFSET;

}

void calibrate(int num) {
    float xt, yt, zt;
    float x =0, y =0, z =0;

    for (int i = 0; i < num; i++) {
        get_g(xt, yt, zt);
        x += xt;
        y += yt;
        z += zt;
    }

    x /= num;
    y /= num;
    z /= num;

    z += -1;

    X_OFFSET = x;
    Y_OFFSET = y;
    Z_OFFSET = z;

}