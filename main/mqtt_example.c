#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_handler.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "uart_handler.h"
#include <stddef.h>

static const char *TAG = "MQTT_MAIN";
static const char *TOPIC = "device/messages";
static const char *BROKER_URI = "mqtt://mqtt-dashboard.com:1883";

void meu_callback(const char *topic, int topic_len, const char *data, int data_len) {
    ESP_LOGI(TAG, "Got message in: %.*s", topic_len, topic);
    ESP_LOGI(TAG, "MESSAGE: %.*s", data_len, data);
}

void meu_processador_de_mensagens(void *pvParameters) {
    char line[MAX_LINE_LEN];
    int pos = 0;

    while (1) {
        uint8_t byte;

        int n = uart_read(&byte);
        if (n <= 0) {
            continue;
        }

        if (byte == '\n' || byte == '\r') {

            /* Ignore empty lines */
            if (pos == 0) {
                continue;
            }

            line[pos] = '\0';
            pos = 0;

            if (!mqtt_wait_for_connection(1000)) {
                static const char msg[] = "ERROR: MQTT disconnected\n";

                ESP_LOGW(TAG, "MQTT not connected");
                uart_write((void *)msg, sizeof(msg) - 1);
                continue;
            }

            ESP_LOGI(TAG, "Publishing: %s", line);

            int msg_id = mqtt_publish(TOPIC, line, 1, false);

            if (msg_id < 0) {
                static const char msg[] = "ERROR: publish failed\n";

                ESP_LOGE(TAG, "Failed to publish");
                uart_write((void *)msg, sizeof(msg) - 1);
            }
        } else {
            if (pos < (MAX_LINE_LEN - 1)) {
                line[pos++] = (char)byte;
            } else {
                ESP_LOGW(TAG, "UART line too long, discarding"); // mmmmm

                static const char msg[] = "ERROR: line too long\n";

                uart_write((void *)msg, sizeof(msg) - 1);
                pos = 0;
            }
        }
    }
}

void app_main(void) {
    // Es buena práctica inicializar NVS primero antes de cualquier delay
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(example_connect());
    vTaskDelay(pdMS_TO_TICKS(2000));
    uart_init();

    mqtt_config_t config = {
        .broker_uri = BROKER_URI,
        .username = NULL,
        .password = NULL,
        .on_message_callback = meu_callback,
    };

    mqtt_app_start(&config);
    ESP_LOGI(TAG, "Connecting to MQTT broker...");

    if (mqtt_wait_for_connection(15000)) {
        ESP_LOGI(TAG, "Connected to broker");
        mqtt_subscribe(TOPIC, 1);
    } else {
        ESP_LOGE(TAG, "MQTT connection timeout");
    }

    xTaskCreate(meu_processador_de_mensagens, "uart_to_mqtt", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "UART to MQTT bridge started");

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
