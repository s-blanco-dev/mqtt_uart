#include "include/uart_handler.h"
#include "driver/uart.h"
#include <stddef.h>
#include <stdint.h>

#define UART_TX_PIN 17
#define UART_RX_PIN 18
#define UART_BUF_SIZE 2048
#define UART_NUM UART_NUM_0

static const char *TAG = "UART";

void uart_init(void) {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config));
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, UART_BUF_SIZE, UART_BUF_SIZE, 10, NULL, 0));
    ESP_LOGI(TAG, "UART initialized");
}

// wrapper
int uart_read(uint8_t *byte) {
    return uart_read_bytes(UART_NUM, byte, 1, portMAX_DELAY);
}

// wrapper
int uart_write(void *msg, size_t size) {
    return uart_write_bytes(UART_NUM, msg, size);
}
