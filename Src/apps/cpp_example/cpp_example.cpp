#include "cpp_example.hpp"


static bool task_running = false;
static taskid_t taskid;


static uint32_t led_delay = 100;

static void cpp_example_run(void)
{
    shellPrint("cpp_example Task Started!");

    while(1)
    {
       
        // Toggle Green LED
        HAL_GPIO_TogglePin(GPIOB,GPIO_PIN_0);


        taskDelay(led_delay);
    }

}




int cpp_example_main(int argc,char argv[MAX_CMD_ARG][MAX_CMD_LENGHT])
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
                    
                    led_delay = atoi(argv[2]);
                    if(led_delay < 1 || led_delay > __INT_MAX__)
                    {
                        led_delay = 100;
                        shellPrint("Undefined argument. Task started with LED delay = 100 ms");
                    }
                }   
                task_running = true;
                taskid = TaskCreate("cpp_example",1024,cpp_example_run,50);
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
                led_delay = atoi(argv[2]);
                if((led_delay < 1) || (led_delay > __INT_MAX__))
                {
                    shellPrint("Error: Need unsigned int as an argument!");
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
        shellPrint("cpp_example Missing Arguments!");
        return -1;
    }
    
    return 0;

}

