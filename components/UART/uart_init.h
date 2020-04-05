#ifndef _UART_INIT_H_
#define _UART_INIT_H_
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "driver/uart.h"

extern void uart_init(uart_port_t uart_num, int br, uart_word_length_t wl, uart_parity_t p, 
                      uart_stop_bits_t sb ,uart_hw_flowcontrol_t fc);

#endif