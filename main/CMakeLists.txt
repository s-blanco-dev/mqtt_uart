idf_component_register(SRCS "mqtt_example.c"
                    PRIV_REQUIRES mqtt nvs_flash esp_netif mqtt_handler uart_handler
                    INCLUDE_DIRS ".")
