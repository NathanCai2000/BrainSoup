#include "driver/ledc.h"
#include "driver/rmt_tx.h"
extern "C" 
{
    #include "dshot_esc_encoder.h"
}

class Motor
{
    public:

    #define DSHOT_ESC_RESOLUTION_HZ 40000000 // 40MHz resolution, DSHot protocol needs a relative high resolution
    
    Motor();
    void init(gpio_num_t gpio, char *_TAG);
    void set_power(int precent);

    gpio_num_t pin;
    rmt_channel_handle_t _esc_chan;
    rmt_encoder_handle_t _dshot_encoder;
    rmt_transmit_config_t _tx_config;
    dshot_esc_throttle_t _throttle;

};