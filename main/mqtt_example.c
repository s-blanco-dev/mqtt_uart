#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqtt_handler.h"
#include "nvs_flash.h"
#include "protocol_examples_common.h"
#include <stddef.h>
#include <stdint.h>

static const char *TAG = "MQTT_MAIN";

void mi_procesador_de_mensajes(const char *topic, int topic_len,
                               const char *data, int data_len) {
  ESP_LOGI(TAG, "Got message in: %.*s", topic_len, topic);
  ESP_LOGI(TAG, "Data: %.*s", data_len, data);
}

void app_main(void) {
  // Es buena práctica inicializar NVS primero antes de cualquier delay
  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_netif_init());
  ESP_ERROR_CHECK(esp_event_loop_create_default());

  ESP_ERROR_CHECK(example_connect());

  mqtt_config_t config = {.broker_uri = "mqtt://mqtt-dashboard.com:1883",
                          .username = "123",
                          .password = "456",
                          .on_message_callback = mi_procesador_de_mensajes};

  mqtt_app_start(&config);

  ESP_LOGI(TAG, "Connectin to broker :(");
  if (mqtt_wait_for_connection(15000)) {
    ESP_LOGI(TAG, "SUCCESS! Let's send some messages.");

    mqtt_subscribe("/uuu/comandos", 1);
    // vTaskDelay(pdMS_TO_TICKS(100));
    mqtt_publish("/uuu/arrarte", "ho ho hi", 1, false);
  } else {
    ESP_LOGE(TAG, "FAIL. Broker connection timeout");
  }

  while (1) {
    vTaskDelay(pdMS_TO_TICKS(1000));
  }
}
