#include "bridge.h"

static void UDPRecCallback(xUDPTaskHandle xUDPTask, void *data, void *friend);
static void UARTRecCallBack(xUARTTaskHandle xUARTTask, void *data, void *friend);

xBridgeTaskHandle xBridgeTaskCreate(xUARTTaskHandle xUARTTask, xUDPTaskHandle xUDPTask)
{
    xBridgeTaskHandle newObject = (xBridgeTaskHandle)malloc(sizeof(xBridgeTask_t));
    
    newObject->xUARTTask = xUARTTask;
    newObject->xUDPTask = xUDPTask;
    
    newObject->xUARTTask->RecCallback = UARTRecCallBack;
    newObject->xUDPTask->RecCallback = UDPRecCallback;
    
    newObject->xUARTTask->friend = (void*)newObject->xUDPTask;
    newObject->xUDPTask->friend = (void*)newObject->xUARTTask;

    return newObject;
}

void UDPRecCallback(xUDPTaskHandle xUDPTask, void *data, void *friend)
{
    xUARTTaskSend((xUARTTaskHandle)friend, (xMemoryBlockHandle)data);
}

void UARTRecCallBack(xUARTTaskHandle xUARTTask, void *data, void *friend)
{
    xUDPTaskSend((xUDPTaskHandle)friend, (xMemoryBlockHandle)data);
}

void xBridgeTaskStart(xBridgeTaskHandle xBridgeTask)
{
    xUARTTaskStart(xBridgeTask->xUARTTask);
    xUDPTaskStart(xBridgeTask->xUDPTask);
}