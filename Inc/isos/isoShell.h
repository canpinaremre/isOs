#define MAX_CMD_LENGHT 50
#define MAX_CMD_ARG 5

#pragma once

#include <stdint.h>
#include "stm32f3xx_hal.h"
#include "main.h"
#include <stdio.h>
#include <string.h>
#include "app.h"




void isoShell_main();
void isoShell_init(UART_HandleTypeDef *uartHandle);
void handleInput(uint8_t *cmd);
void handleCommand(char *cmd);
void shellPrint(const char *val);
void printHelp(void);