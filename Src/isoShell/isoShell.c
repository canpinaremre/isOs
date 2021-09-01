#include "isoShell.h"

static uint8_t rxBuffer;
static uint8_t dmabuff[10];
uint8_t enter[] = "\r\nisoShell-> ";
static uint8_t cmdBuffer[MAX_CMD_LENGHT];
static uint8_t command[MAX_CMD_LENGHT];
static uint8_t cmdPtr = 0;
static UART_HandleTypeDef *uart_handle;

void isoShell_main()
{

    rxBuffer = dmabuff[0];
    if(rxBuffer != 0)
    {
        if(rxBuffer == '\r')
        {
            handleInput(cmdBuffer);
        }
        else if(rxBuffer == 127)//delete
        {
            // Invalid input for now
        }
        else if(rxBuffer < 32 || rxBuffer > 126)
        {
            // Invalid
            rxBuffer = 0;
            dmabuff[0] = 0;
        }
        else
        {   
            cmdBuffer[cmdPtr] = rxBuffer;
            cmdPtr++;
            if(cmdPtr == MAX_CMD_LENGHT)
            {
                //TODO: too long error
                cmdPtr = 0;
            }

            HAL_UART_Transmit(uart_handle,&rxBuffer,1,5);
        }


        dmabuff[0] = 0;
        HAL_UART_Receive_DMA(uart_handle,dmabuff,10);
    }
    

}

void isoShell_init(UART_HandleTypeDef *uartHandle)
{
    uart_handle = uartHandle;
    shellPrint("Welcome to isoShell!");
    HAL_UART_Receive_DMA(uart_handle,dmabuff,1);
}

void handleInput(uint8_t *cmd)
{
    
    strncpy((char*)command,(char*)cmdBuffer,cmdPtr);
    command[cmdPtr] = '\0';

    handleCommand((char*)command);

    
    cmdPtr = 0;
}

void handleCommand(char *cmd)
{
    if(cmdPtr == 0)
    {
        HAL_UART_Transmit(uart_handle,enter,sizeof(enter),15);
    }
    else if (!strcmp("Hello",cmd))
    {
        shellPrint("Hi!");
    }
    else if (!strcmp("help",cmd))
    {
        printHelp();
    }
    else
    {
        shellPrint("Unknown Command. Try \"help\" ");
    }

}

void shellPrint(const char *val)
{
    HAL_UART_Transmit(uart_handle,enter,sizeof(enter),sizeof(enter));
    HAL_UART_Transmit(uart_handle,(uint8_t *)val,strlen(val),strlen(val));
    HAL_UART_Transmit(uart_handle,enter,sizeof(enter),sizeof(enter));
}

void printHelp(void)
{   shellPrint(" ");
    shellPrint("********************************");
    shellPrint("isoShell Usage:");
    shellPrint("help:       This page");
    shellPrint("Hello:      Prints \"Hi!\"");    
    shellPrint("********************************");
}
