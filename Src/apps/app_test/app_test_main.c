
#include "app_test.h"
#include "task.h"
#include "isoShell.h"

static bool task_running = false;

static void app_test_thread(void)
{
    while(1)
    {


        shellPrint("app_test_thread running!");
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
        TaskCreate("app_test",512,app_test_thread,10);
    }
    
    return 0;

}

