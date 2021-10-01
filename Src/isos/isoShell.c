#include "isoShell.h"


uint8_t enter[] = "\r\nisoShell-> ";
char errTooLong[] = "\r\nisoShell-> Command Too Long!";
static uint8_t cmdBuffer[MAX_CMD_LENGHT];
static uint8_t command[MAX_CMD_LENGHT];
static uint8_t cmdPtr = 0;
static UART_HandleTypeDef *uart_handle;
static uint8_t last_command[MAX_CMD_LENGHT] ="";
static uint8_t last_cmdPtr = 0;
static bool isArrow = false;

void isoShell_main(uint8_t rxBuffer[4])
{

    // TODO: up-down arrow using stack to recover all commands
    if(rxBuffer[0] != 0)
    {
        if(rxBuffer[0] == '\r')
        {
            if(cmdPtr > 1)
            {
                for(int i = 0; i < cmdPtr; i++)
                {
                    last_command[i] = cmdBuffer[i];
                }
                last_cmdPtr = cmdPtr;
            }
            
            handleInput(cmdBuffer);
        }
        else if((rxBuffer[0] == 127)&&(cmdPtr))//delete
        {
            //TODO: No newline but need better approach
            uint8_t emptyline[] = "\r                                                            ";
            HAL_UART_Transmit(uart_handle,emptyline,sizeof(emptyline),15);
            HAL_UART_Transmit(uart_handle,(uint8_t *)"\risoShell-> ",sizeof("\risoShell-> "),15);
            cmdPtr--;
            HAL_UART_Transmit(uart_handle,&cmdBuffer[0],cmdPtr,10);
            exit_critical_section();
        }
        else if(rxBuffer[0] == 91)
        {
            // TODO: fix arrow direction currently only Up arrow
            isArrow = true;
        }
        else if(isArrow)
        {
            for(int i = 0; i < last_cmdPtr; i++)
            {
                cmdBuffer[i] = last_command[i];
            }

            enter_critical_section();
            HAL_UART_Transmit(uart_handle,&last_command[0],last_cmdPtr,2+last_cmdPtr);
            exit_critical_section();
            cmdPtr = last_cmdPtr;
            isArrow = false;
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
        return;
    }
   
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
        else
        {
            char sendBuffer[60];
            sprintf(sendBuffer, "App returned with error: %d",ret);
	        shellPrint(sendBuffer);
        }
    }
    else if (!strcmp("Hello",token))
    {
        shellPrint("Hi!");
    }
    else if (!strcmp("help",token))
    {
        printHelp();
    }
    else if (!strcmp("top",token))
    {
        top_tasks();
    }
    else if (!strcmp("kill",token))
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
            shellPrint("Missing arguments. Try \"help\" ");
            return;
        }
        char * tmp;
        uint32_t pid = strtoul(argv[0], &tmp, 10);
        if((pid < 0 ) || (pid > MAX_TASKS))
        {
            shellPrint("Error: pid out of range!");
            return;
        }
        int ret = kill_task(pid);
        if(ret == 0)
        {
            char sendBuffer[60];
            sprintf(sendBuffer, "Task killed. pid: %ld",pid);
	        shellPrint(sendBuffer);
            return;    
        }
        else
        {
            char sendBuffer[60];
            sprintf(sendBuffer, "App returned with error: %d",ret);
	        shellPrint(sendBuffer);
            return;
        }
    }
    else
    {
        shellPrint("Unknown Command. Try \"help\" ");
    }
    
}

void shellPrint(const char *val)
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
    shellPrint("top:                Running threads");   
    shellPrint("kill $pid:          Kill thread with pid:$pid");           
    shellPrint("********************************");
}
