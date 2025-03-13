extern "C" {
    #include "ESP_CRSF.h"
}

void init_RC(uint8_t tx, uint8_t rx);
crsf_channels_t get_RC();
bool RC_Health();
static int RC_count;