#include "isoShell.h"


uint8_t enter[] = "\r\nisoShell-> ";
char errTooLong[] = "\r\nisoShell-> Command Too Long!";
static uint8_t cmdBuffer[MAX_CMD_LENGHT];
static uint8_t command[MAX_CMD_LENGHT];
static uint8_t cmdPtr = 0;
static UART_HandleTypeDef *uart_handle;

void isoShell_main(uint8_t rxBuffer[4])
{

    // TODO: up-down arrow using stack to recover all commands
    if(rxBuffer[0] != 0)
    {
        if(rxBuffer[0] == '\r')
        {
            handleInput(cmdBuffer);
        }
        else if((rxBuffer[0] == 127)&&(cmdPtr))//delete
        {
            // TODO: Try with no newline
            enter_critical_section();
            HAL_UART_Transmit(uart_handle,enter,sizeof(enter),15);
            cmdPtr--;
            HAL_UART_Transmit(uart_handle,&cmdBuffer[0],cmdPtr,10);
            exit_critical_section();
        }
        else if(rxBuffer[0] < 32 || rxBuffer[0] > 126)
        {
            // Invalid
            rxBuffer[0] = 0;
        }
        else
        {   
            cmdBuffer[cmdPtr] = rxBuffer[0];
            cmdPtr++;
            
            enter_critical_section();
            HAL_UART_Transmit(uart_handle,&rxBuffer[0],1,5);
            exit_critical_section();

            if(cmdPtr == MAX_CMD_LENGHT)
            {
                shellPrint(errTooLong);
                cmdPtr = 0;
            }
        }

    }
    

}

void isoShell_init(UART_HandleTypeDef *uartHandle)
{
    uart_handle = uartHandle;
    shellPrint("Welcome to isoShell!");
    //HAL_UART_Receive_DMA(uart_handle,dmabuff,1);
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
        enter_critical_section();
        HAL_UART_Transmit(uart_handle,enter,sizeof(enter),15);
        exit_critical_section();
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

void shellPrint(char *val)
{
    enter_critical_section();
    HAL_UART_Transmit(uart_handle,enter,sizeof(enter),sizeof(enter));
    HAL_UART_Transmit(uart_handle,(uint8_t *)val,strlen(val),strlen(val));
    HAL_UART_Transmit(uart_handle,enter,sizeof(enter),sizeof(enter));
    exit_critical_section();
}

void printHelp(void)
{   
    shellPrint("********************************");
    shellPrint("isoShell Usage:");
    shellPrint("help:               This page");
    shellPrint("Hello:              Prints \"Hi!\"");
    shellPrint("app $app_name:      Start app $app_name");
    shellPrint("app help:           See apps");            
    shellPrint("********************************");
}
