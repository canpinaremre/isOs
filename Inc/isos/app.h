#ifndef __APP_H__
#define __APP_H__


#include "stm32f3xx_hal.h"
#include "isoShell.h"
#include "task.h"
#include <stdint.h>

//Apps Inc:
#include "led_example.h"
#include "app_test.h"
#include "cpp_example.hpp"



// App Command Handler:
int handle_app_command(int argc,char argv[MAX_CMD_ARG][MAX_CMD_LENGHT]);


#endif // __APP_H__