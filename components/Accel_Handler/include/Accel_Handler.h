#include <stdint.h>
#include "driver/i2c_master.h"

#define ACCEL_RANGE LIS331::HIGH_RANGE
#define ACCEL_MAX_SCALE 400
#define ACCEL_I2C_ADDRESS 0x19

void init_accel();

void get_g(float &x, float &y, float &z);

void calibrate(int num);