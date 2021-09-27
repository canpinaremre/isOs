#include "led_example.h"


static bool task_running = false;
static taskid_t taskid;


static uint32_t led_delay = 100;

static void led_example_run(void)
{
    shellPrint("led_example Task Started!");

    while(1)
    {
       
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_14);
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);


        taskDelay(led_delay);
    }

}


int led_example_main(int argc,char argv[MAX_CMD_ARG][MAX_CMD_LENGHT])
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
                if(argc > 2)
                {
                    char * tmp;
                    led_delay = strtoul(argv[2], &tmp, 10);
                    if(led_delay < 1 || led_delay > __INT_MAX__)
                    {
                        led_delay = 100;
                    }
                }   
                task_running = true;
                taskid = TaskCreate("led_example",1024,led_example_run,99);
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
            
            shellPrint("Commands: help, start (delay: optional), stop, delay");

            return 0;
            
        }
        else if(!strcmp("delay",argv[1]))
        {
            if(argc > 2)
            {
                char * tmp;
                led_delay = strtoul(argv[2], &tmp, 10);
                if((led_delay < 1) || (led_delay > __INT_MAX__))
                {
                    led_delay = 100;
                    shellPrint("Error: Need uint as an argument! Delay = 100");
                    return -1;
                }
                else
                {
                    shellPrint("Delay Set Successful");
                    return 0;
                }
            }   

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
        shellPrint("led_example Missing Arguments!");
        return -1;
    }
    
    return 0;

}
