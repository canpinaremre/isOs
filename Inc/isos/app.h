#ifndef __APP_H__
#define __APP_H__


#include "stm32f3xx_hal.h"
#include "isoShell.h"
#include "task.h"
#include <stdint.h>

// Apps:
#include "led_example.h"
#include "app_test.h"




int handle_app_command(int argc,char argv[MAX_CMD_ARG][MAX_CMD_LENGHT]);


#endif // __APP_H__