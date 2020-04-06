#ifndef _BRIDGE_H_
#define _BRIDGE_H_
#include "uart_task.h"
#include "udp_task.h"

typedef struct xBridgeTask_t
{
    xUARTTaskHandle xUARTTask;
    xUDPTaskHandle xUDPTask;
}xBridgeTask_t;
typedef xBridgeTask_t* xBridgeTaskHandle;

extern xBridgeTaskHandle xBridgeTaskCreate(xUARTTaskHandle xUARTTask, xUDPTaskHandle xUDPTask);

extern void xBridgeTaskStart(xBridgeTaskHandle xBridgeTask);

#endif