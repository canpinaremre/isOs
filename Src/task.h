#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f3xx_it.h"
#include "stm32f3xx_hal.h"

#define MAX_TASKS 5
#define TASK_SIZE 512

#define MAIN_RETURN 0xFFFFFFF9  //Tells the handler to return using the MSP
#define THREAD_RETURN 0xFFFFFFFD //Tells the handler to return using the PSP

void systick_handle(void);
void TaskCreate(uint8_t *stack, uint32_t stackSize, void (*entrypoint)());
void KernelStart(void);