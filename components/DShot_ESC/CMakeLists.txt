idf_component_register(SRCS "dshot_esc_encoder.c"
                    INCLUDE_DIRS "include"
                    REQUIRES driver
                    REQUIRES esp_driver_rmt
                    )
