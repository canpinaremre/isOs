#ifndef __APP_HPP__
#define __APP_HPP__

#include "stm32f3xx_hal.h"
#include "isoShell.h"


#ifdef __cplusplus
extern "C" {
#endif
void shellPrint(const char *val);
int cpp_example_main(int argc,char argv[MAX_CMD_ARG][MAX_CMD_LENGHT]);

#ifdef __cplusplus
}
#endif

#endif //__APP_HPP__