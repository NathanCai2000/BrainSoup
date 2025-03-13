idf_component_register(SRCS "LED_Handler.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_driver_ledc
                    REQUIRES driver
                    )
