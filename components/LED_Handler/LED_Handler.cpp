#include "LED_Handler.h"

LED::LED() {

}

void LED::init_LED(int pin, uint8_t chan) {
    LEDC_CHANNEL = (ledc_channel_t) chan;
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .duty_resolution  = LEDC_TIMER_13_BIT,
        .timer_num        = LEDC_TIMER_0,
        .freq_hz          = 4000,  // Set output frequency at 4 kHz
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    ledc_channel_config_t ledc_channel = {
        .gpio_num       = pin,
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER_0,
        .duty           = 0, // Set duty to 0%
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void LED::LED_ON() {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 8192);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void LED::LED_OFF() {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}

void LED::LED_BLINK(int num, int period) {
    for (int i = 0; i < num; i++) {
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 8192);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(period/portTICK_PERIOD_MS);
        ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
        ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
        vTaskDelay(period/portTICK_PERIOD_MS);
    }
}

void LED::control(int precent) {
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 8192 * precent/100);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}