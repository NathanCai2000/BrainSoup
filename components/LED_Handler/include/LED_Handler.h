#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/ledc.h"

#define LEDC_MODE       LEDC_LOW_SPEED_MODE

class LED
{
    public:
    LED();
    void init_LED(int pin, uint8_t chan);
    void LED_ON();
    void LED_OFF();
    void LED_BLINK(int num, int period);
    void control(int precent);

    ledc_channel_t LEDC_CHANNEL;

};