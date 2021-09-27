
#include "app_test.h"
#include "task.h"
#include "isoShell.h"

static bool task_running = false;
static taskid_t taskid;

static void app_test_thread(void)
{
    shellPrint("app_test Task Started!");
    int counter = 0;
    char sendBuffer[60];
    while(1)
    {
        counter++;

        sprintf(sendBuffer, "app_test_thread running! \n\r Counter: %d",counter);
	    shellPrint(sendBuffer);


        taskDelay(1500);
    }

}


int app_test_main(int argc,char argv[MAX_CMD_ARG][MAX_CMD_LENGHT])
{
    if(argc > 1)
    {
        if(!strcmp("start",argv[1]))
        {
            if(task_running)
            {
                shellPrint("Task Already Running!");
                return -1;
            }
            else
            {   
                task_running = true;
                taskid = TaskCreate("app_test",1024,app_test_thread,99);
                return 0;
            }
        }
        else if(!strcmp("stop",argv[1]))
        {
            if(!task_running)
            {
                shellPrint("Task Not Running!");
                return -1;
            }
            else
            {
                taskDelete(taskid);
                shellPrint("Task Stopped!");
                task_running = false;
                return 0;
            }
        }
        else if(!strcmp("help",argv[1]))
        {
            
            shellPrint("Commands: help, start, stop");

            return 0;
            
        }
        else
        {
            shellPrint("Command not found! try \"help\"");
            return -1;
        }
    }
    else
    {
        shellPrint("Test Application Missing Arguments!");
        return -1;
    }
    
    return 0;

}

