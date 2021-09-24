#include "app.h"

int handle_app_command(int argc,char argv[MAX_CMD_ARG][MAX_CMD_LENGHT])
{
    int ret;
    if(!strcmp("help",argv[0]))
    {
        shellPrint("**************");
        shellPrint(" APP HELP ");
        shellPrint(" app_test:            app test ");
        shellPrint(" hello:               Hello from app ");
        shellPrint("**************");
        return 0;
    }
    else if(!strcmp("app_test",argv[0]))
    {
        shellPrint("app test in action");
        ret = app_test_main();
        return ret;
    }
    else if(!strcmp("hello",argv[0]))
    {
        shellPrint("Hello from app interface");
    }
    else
    {
        return -1;
    }



    // Success
    return 0;
}