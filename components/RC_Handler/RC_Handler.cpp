#include <stdio.h>
#include "RC_Handler.h"

#define mapRange(a1,a2,b1,b2,s) (b1 + (s-a1)*(b2-b1)/(a2-a1))

void init_RC(uint8_t tx, uint8_t rx) {

    RC_count = 0;

    crsf_config_t RCconfig = {
        .uart_num = UART_NUM_1,
        .tx_pin = tx,
        .rx_pin = rx
    };

    CRSF_init(&RCconfig);

}

crsf_channels_t get_RC() {
    crsf_channels_t channels = {0};
    CRSF_receive_channels(&channels);
    return channels;
}

bool RC_Health() {
    bool state = CRSF_connected();

    if (state) {
        RC_count = 0;
        return true;
    }
    else if (RC_count < 5) {
        RC_count++;
        return true;
    }
    else {
        return false;
    }

}
