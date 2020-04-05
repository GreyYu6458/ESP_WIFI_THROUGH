#include "uart_init.h"

#define BUF_SIZE (1024)

void uart_init(uart_port_t uart_num, int br, uart_word_length_t wl, uart_parity_t p, 
               uart_stop_bits_t sb ,uart_hw_flowcontrol_t fc)
{
    // Configure parameters of an UART driver,
    // communication pins and install the driver
    uart_config_t uart_config = {
        .baud_rate = br,
        .data_bits = wl,
        .parity    = p,
        .stop_bits = sb,
        .flow_ctrl = fc
    };
    uart_param_config(uart_num, &uart_config);
    uart_driver_install(uart_num, BUF_SIZE * 2, 0, 0, NULL, 0);
}