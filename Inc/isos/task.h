#pragma once

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include "stm32f3xx_it.h"
#include "stm32f3xx_hal.h"
#include "prioq.h"

// config defines:
#define USE_STACK_TASK
//#define WQUEUE
#define PRIORITY_SCHEDULER
// end of config defines


#ifndef MAX_TASKS
#define MAX_TASKS 15
#endif

#ifndef ROUND_ROBIN_SCHEDULER
# ifndef PRIORITY_SCHEDULER
#define ROUND_ROBIN_SCHEDULER
# endif
#endif

#ifdef ROUND_ROBIN_SCHEDULER
#undef PRIORITY_SCHEDULER
#endif

#define DEFAULT_TASK_SIZE 512

#ifdef USE_STACK_TASK
# ifndef STACK_TASK_SIZE
# define STACK_TASK_SIZE 16384 //16 kB
# endif
#endif

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
    TaskEmpty,
    TaskBlocked,
    TaskSuspend,
    TaskReady,
    TaskRunning,
    TaskDeleted
}taskState_t;

typedef uint32_t taskid_t;

struct task
{
    uint32_t stackPointer;
    taskid_t taskId;
    uint32_t stackSize;
    uint8_t priority;
    taskState_t taskState;
    uint32_t delayUntil;
    void * heapPtr;
    char taskName[20];
    #ifdef USE_STACK_TASK
    bool isStatic;
    #endif
};

#ifdef USE_STACK_TASK
taskid_t TaskCreateStatic(const char* name, uint32_t stackSize, void (*entrypoint)(), uint8_t priority);
#endif
taskid_t TaskCreate(const char* name, uint32_t stackSize, void (*entrypoint)(), uint8_t priority);
taskid_t getTaskId();
void taskDelete(taskid_t tid);
void KernelStart(void);
void KernelInit(void);
const char* return_task_name();
void taskDelay(uint32_t delayTime);
void switchTask(void);
void enter_critical_section(void);
void exit_critical_section(void);

#ifdef PRIORITY_SCHEDULER
void insert_queue(uint32_t pid,uint8_t prio);
void checkBlockedTasks(void);
#endif