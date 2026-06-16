#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"

#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"
#include "freertos/task.h"

#include "mqtt_handler.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include "uart_handler.h"

#include <stddef.h>
#include <stdint.h>

#define MQTT_TX_QUEUE_LENGTH 10
#define DEFAULT_UART_TOPIC "/uuu/arrarte"

static const char *TAG = "MQTT_MAIN";

static QueueHandle_t s_mqtt_tx_queue = NULL;

static void mqtt_tx_task(void *pvParameters) {
    uart_mqtt_msg_t msg;

    while (1) {
        if (xQueueReceive(s_mqtt_tx_queue, &msg, portMAX_DELAY) == pdTRUE) {
            if (!mqtt_wait_for_connection(10000)) {
                ESP_LOGW(TAG, "MQTT no conectado. Mensaje descartado");
                continue;
            }

            int msg_id = mqtt_publish(msg.topic, msg.payload, msg.qos, msg.retain);

            ESP_LOGI(TAG, "Publicado MQTT. msg_id=%d topic=%s payload=%s",
                     msg_id, msg.topic, msg.payload);
        }
    }
}

void mi_procesador_de_mensajes(const char *topic, int topic_len,
                               const char *data, int data_len) {
    ESP_LOGI(TAG, "Got message in: %.*s", topic_len, topic);
    ESP_LOGI(TAG, "Data: %.*s", data_len, data);

    /*
     * Esto es MQTT -> UART.
     * O sea, si llega algo al topic suscrito,
     * tambien lo mostramos por la consola UART.
     */
    uart_write_mqtt_rx(topic, topic_len, data, data_len);
}

void app_main(void) {
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    s_mqtt_tx_queue = xQueueCreate(MQTT_TX_QUEUE_LENGTH, sizeof(uart_mqtt_msg_t));
    if (s_mqtt_tx_queue == NULL) {
        ESP_LOGE(TAG, "No se pudo crear la queue MQTT TX");
        return;
    }

    ESP_ERROR_CHECK(example_connect());

    mqtt_config_t config = {
        .broker_uri = "mqtt://mqtt-dashboard.com:1883",
        .username = "123",
        .password = "456",
        .on_message_callback = mi_procesador_de_mensajes
    };

    mqtt_app_start(&config);

    ESP_LOGI(TAG, "Connecting to broker...");

    if (mqtt_wait_for_connection(15000)) {
        ESP_LOGI(TAG, "MQTT conectado");

        mqtt_subscribe("/uuu/comandos", 1);

        mqtt_publish("/uuu/arrarte",
                     "{\"source\":\"boot\",\"message\":\"ESP32 conectado\"}",
                     1,
                     false);
    } else {
        ESP_LOGE(TAG, "Broker connection timeout");
    }

    static uart_mqtt_task_params_t uart_params;
    uart_params.tx_queue = s_mqtt_tx_queue;
    uart_params.default_topic = DEFAULT_UART_TOPIC;

    xTaskCreate(
        uart_mqtt_task,
        "uart_mqtt_task",
        4096,
        &uart_params,
        tskIDLE_PRIORITY + 3,
        NULL
    );

    xTaskCreate(
        mqtt_tx_task,
        "mqtt_tx_task",
        4096,
        NULL,
        tskIDLE_PRIORITY + 2,
        NULL
    );

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
