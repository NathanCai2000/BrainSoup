idf_component_register(SRCS "MC_Handler.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES driver
                    REQUIRES esp_driver_rmt
                    REQUIRES DShot_ESC
                    )
