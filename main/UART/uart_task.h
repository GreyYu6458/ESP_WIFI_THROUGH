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

typedef struct
{
UARTObject* this;
const char* task_name;
int huart;

xQueueHandle uart_rec_queue;
xQueueHandle uart_send_queue;

void (*rec_task_start)(UARTObject* this);
inline void (*write)(const indefiniteData* data, UARTObject* this);
inline void (*send)(const indefiniteData* data, UARTObject* this);
inline void (*rec_callback)(void* data, UARTObject* this);

}UARTObject;

extern UARTObject* UARTObject_Construct(int u, const char *task_name);


#endif