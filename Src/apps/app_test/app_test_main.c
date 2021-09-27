
#include "app_test.h"
#include "task.h"
#include "isoShell.h"

static bool task_running = false;

static void app_test_thread(void)
{
    // int counter = 0;
    while(1)
    {
        // counter++;
        // char str[20];
        // sprintf(str, "Counter = %d", counter);
        // shellPrint("app_test_thread running!");
        // shellPrint(str);
        taskDelay(500);
    }

}


int app_test_main()
{

    if(task_running)
    {
        shellPrint("Task Already Running!");
        return -1;
    }
    else
    {   
        task_running = true;
        TaskCreate("app_test",512,app_test_thread,99);
    }
    
    return 0;

}

