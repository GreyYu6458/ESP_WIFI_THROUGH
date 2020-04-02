#ifndef _UART_H_
#define _UART_H_
#include "main.h"
#include "driver/uart.h"
#define UART0_BUF_SIZE  (1024)
#define UART0_BAUDRATE  (921600)

#define UART1_BUF_SIZE  (1024)
#define UART1_BAUDRATE  (115200)

extern void uart0_init();
extern void uart1_init();

#endif