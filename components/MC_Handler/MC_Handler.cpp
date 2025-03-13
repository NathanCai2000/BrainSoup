#include <stdio.h>
#include "MC_Handler.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

Motor::Motor() {

}

void Motor::init(gpio_num_t gpio, char *_TAG) {

    pin = gpio;

    ESP_LOGI(_TAG, "Create RMT TX channel");
    rmt_channel_handle_t esc_chan = NULL;
    rmt_tx_channel_config_t tx_chan_config = {
        .gpio_num = pin,
        .clk_src = RMT_CLK_SRC_DEFAULT, // select a clock that can provide needed resolution
        .resolution_hz = DSHOT_ESC_RESOLUTION_HZ,
        .mem_block_symbols = 48,
        .trans_queue_depth = 10, // set the number of transactions that can be pending in the background
    };
    ESP_ERROR_CHECK(rmt_new_tx_channel(&tx_chan_config, &esc_chan));

    ESP_LOGI(_TAG, "Install Dshot ESC encoder");
    rmt_encoder_handle_t dshot_encoder = NULL;
    dshot_esc_encoder_config_t encoder_config = {
        .resolution = DSHOT_ESC_RESOLUTION_HZ,
        .baud_rate = 600000, // DSHOT600 protocol
        .post_delay_us = 50, // extra delay between each frame
    };
    ESP_ERROR_CHECK(rmt_new_dshot_esc_encoder(&encoder_config, &dshot_encoder));

    ESP_LOGI(_TAG, "Enable RMT TX channel");
    ESP_ERROR_CHECK(rmt_enable(esc_chan));

    rmt_transmit_config_t tx_config = {
        .loop_count = -1, // infinite loop
    };
    dshot_esc_throttle_t throttle = {
        .throttle = 0,
        .telemetry_req = false, // telemetry is not supported in this example
    };

    _esc_chan = esc_chan;
    _dshot_encoder = dshot_encoder;
    _tx_config = tx_config;
    _throttle = throttle;

    ESP_LOGI(_TAG, "Start ESC by sending zero throttle for a while...");
    ESP_ERROR_CHECK(rmt_transmit(_esc_chan, _dshot_encoder, &_throttle, sizeof(_throttle), &_tx_config));
    vTaskDelay(5000/portTICK_PERIOD_MS);

}

void Motor::set_power(int power) {
    _throttle.throttle = power;
    ESP_ERROR_CHECK(rmt_transmit(_esc_chan, _dshot_encoder, &_throttle, sizeof(_throttle), &_tx_config));
    ESP_ERROR_CHECK(rmt_disable(_esc_chan));
    ESP_ERROR_CHECK(rmt_enable(_esc_chan));
    
}