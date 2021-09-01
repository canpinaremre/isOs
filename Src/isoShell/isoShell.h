#include <stdint.h>
#include "stm32f3xx_hal.h"
#include "main.h"
#include <stdio.h>
#include <string.h>

#define MAX_CMD_LENGHT 50

void isoShell_main();
void isoShell_init(UART_HandleTypeDef *uartHandle);
void handleInput(uint8_t *cmd);
void handleCommand(char *cmd);
void shellPrint(const char *val);
void printHelp(void);