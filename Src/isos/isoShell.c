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
        char * token = strtok(cmd, " ");
        int argc = 0;
        char argv[MAX_CMD_ARG][MAX_CMD_LENGHT];
        memset(argv, 0, sizeof(argv));
        if(!strcmp("app",token))
        {
           while (token != NULL) 
           {
                token = strtok(NULL, " ");
                if(argc > MAX_CMD_ARG)
                {
                   shellPrint("Error: Too Many Arguments.");
                   return;
                }
                if(token)
                {
                    strncat(argv[argc], token, strlen(token));
                }
               argc++;  
            }
            argc--;
            if(!argc)
            {
                shellPrint("Missing arguments. Try \"app help\" ");
            }
            
            //Handle with app_shell from here
            int ret = handle_app_command(argc,argv);
            if(ret == 0) return;
            else shellPrint("App returned with error");

        }
        else
        {
            shellPrint("Unknown Command. Try \"help\" ");
        }
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
    shellPrint("help:               This page");
    shellPrint("Hello:              Prints \"Hi!\"");
    shellPrint("app $app_name:      Start app $app_name");
    shellPrint("app help:           See apps");            
    shellPrint("********************************");
}
