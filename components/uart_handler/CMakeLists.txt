idf_component_register(SRCS "uart_handler.c"
                    PRIV_REQUIRES esp_driver_uart 
                       INCLUDE_DIRS "include"
                     )
