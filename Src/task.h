#include "main.h"
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "stm32f3xx_it.h"
#include "stm32f3xx_hal.h"

#define MAX_TASKS 15
#define DEFAULT_TASK_SIZE 512
#define STACK_SIZE 16384 //16 kB

#define MAIN_RETURN 0xFFFFFFF9  //Tells the handler to return using the MSP
#define THREAD_RETURN 0xFFFFFFFD //Tells the handler to return using the PSP

struct HardwareStackFrame
{
    uint32_t R0;
    uint32_t R1;
    uint32_t R2;
    uint32_t R3;
    uint32_t R12;
    uint32_t LR;
    uint32_t PC;
    uint32_t xPSR;
};

struct SoftwareStackFrame
{
    uint32_t R4;
    uint32_t R5;
    uint32_t R6;
    uint32_t R7;
    uint32_t R8;
    uint32_t R9;
    uint32_t R10;
    uint32_t R11;
};

typedef enum{
    TaskBlocked,
    TaskSuspend,
    TaskReady,
    TaskRunning
}taskState_t;

typedef uint8_t taskid_t;

struct task
{
    uint32_t stackPointer;
    taskid_t taskId;
    uint32_t stackSize;
    uint8_t priority;
    taskState_t taskState;
    uint32_t delayUntil;
    char taskName[20];
};

taskid_t TaskCreate(const char* name, uint32_t stackSize, void (*entrypoint)(), uint8_t priority);
void KernelStart(void);
const char* return_task_name();
void taskDelay(uint32_t delayTime);
void switchTask(void);
