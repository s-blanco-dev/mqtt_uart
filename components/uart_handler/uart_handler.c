#include "include/uart_handler.h"
#include "driver/uart.h"

#define RXD1 6   // UART1 RX
#define TXD1 7   // UART1 TX
#define UART_BUF_SIZE 1024

static void uart_chorro() {
    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE
    };

    uart_driver_install(UART_NUM_0, UART_BUF_SIZE * 2, 0, 0, NULL, 0);
    uart_param_config(UART_NUM_0, &uart_config);

}
