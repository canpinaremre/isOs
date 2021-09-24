
#include "app_test.h"
#include "task.h"
#include "isoShell.h"


static void app_test_thread(void)
{
    while(1)
    {


        shellPrint("app_test_thread running!");
        taskDelay(100);
    }

}


int app_test_main(int arg)
{

    if (arg == 99)
    {
        TaskCreate("app_test",512,app_test_thread,10);
    }

    return arg + 5;

}

