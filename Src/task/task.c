#include "task.h"

static struct task tasks[MAX_TASKS];

static uint32_t taskCount = 0;
static uint32_t nextTaskIndex = 0;
static uint32_t idleTaskIndex = 0;

struct task *currentTask = NULL;
struct task *nextTask = NULL;

#ifdef USE_STACK_TASK
uint8_t mainStack[STACK_TASK_SIZE]; //Allocate memory in stack
uint8_t *stack_ptr = mainStack; //pointer to stack
#endif




static void TaskReturn()
{
    while(1);
}
static void idleTask()
{
    while (1);  
}

#ifdef USE_STACK_TASK
taskid_t TaskCreateStatic(const char* name, uint32_t stackSize, void (*entrypoint)(), uint8_t priority)
{
    tasks[taskCount].stackSize = stackSize;
    tasks[taskCount].priority = priority;
    strcpy(tasks[taskCount].taskName,name);
    tasks[taskCount].taskId = (taskid_t)taskCount;
    tasks[taskCount].taskState = TaskReady;
    tasks[taskCount].delayUntil = 0;

    struct HardwareStackFrame hardwareStackFrame;
    struct SoftwareStackFrame softwareStackFrame;
    
    hardwareStackFrame.R0 = 0;
    hardwareStackFrame.R1 = 1;
    hardwareStackFrame.R2 = 2;
    hardwareStackFrame.R3 = 3;
    hardwareStackFrame.R12 = 12;
    hardwareStackFrame.LR = (uint32_t)TaskReturn;
    hardwareStackFrame.PC = (uint32_t)entrypoint;
    hardwareStackFrame.xPSR = 0x01000000;
    
    softwareStackFrame.R4 = 4;
    softwareStackFrame.R5 = 5;
    softwareStackFrame.R6 = 6;
    softwareStackFrame.R7 = 7;
    softwareStackFrame.R8 = 8;
    softwareStackFrame.R9 = 9;
    softwareStackFrame.R10 = 10;
    softwareStackFrame.R11 = 11;
	
    stack_ptr += stackSize;

    //make space for hardware stack frame
    uint32_t *stackPointer = (uint32_t *)stack_ptr;
    stackPointer -= sizeof(struct HardwareStackFrame) / sizeof(uint32_t); 
    memcpy(stackPointer, &hardwareStackFrame, sizeof(struct HardwareStackFrame));
    
    //make space for software stack frame
    stackPointer -= sizeof(struct SoftwareStackFrame) / sizeof(uint32_t); 
    memcpy(stackPointer, &softwareStackFrame, sizeof(struct SoftwareStackFrame));
    
    tasks[taskCount].stackPointer = (uint32_t)stackPointer;
    
    
    taskCount++; 
    return tasks[taskCount -1].taskId;
}
#endif
// Create task using heap
taskid_t TaskCreate(const char* name, uint32_t stackSize, void (*entrypoint)(), uint8_t priority)
{
    tasks[taskCount].stackSize = stackSize;
    tasks[taskCount].priority = priority;
    strcpy(tasks[taskCount].taskName,name);
    tasks[taskCount].taskId = (taskid_t)taskCount;
    tasks[taskCount].taskState = TaskReady;
    tasks[taskCount].delayUntil = 0;

    struct HardwareStackFrame hardwareStackFrame;
    struct SoftwareStackFrame softwareStackFrame;
    
    hardwareStackFrame.R0 = 0;
    hardwareStackFrame.R1 = 1;
    hardwareStackFrame.R2 = 2;
    hardwareStackFrame.R3 = 3;
    hardwareStackFrame.R12 = 12;
    hardwareStackFrame.LR = (uint32_t)TaskReturn;
    hardwareStackFrame.PC = (uint32_t)entrypoint;
    hardwareStackFrame.xPSR = 0x01000000;
    
    softwareStackFrame.R4 = 4;
    softwareStackFrame.R5 = 5;
    softwareStackFrame.R6 = 6;
    softwareStackFrame.R7 = 7;
    softwareStackFrame.R8 = 8;
    softwareStackFrame.R9 = 9;
    softwareStackFrame.R10 = 10;
    softwareStackFrame.R11 = 11;
	
    void * heap_ptr = malloc(stackSize);
    heap_ptr += stackSize;


    //make space for hardware stack frame
    uint32_t *stackPointer = (uint32_t *)heap_ptr;
    stackPointer -= sizeof(struct HardwareStackFrame) / sizeof(uint32_t); 
    memcpy(stackPointer, &hardwareStackFrame, sizeof(struct HardwareStackFrame));
    
    //make space for software stack frame
    stackPointer -= sizeof(struct SoftwareStackFrame) / sizeof(uint32_t); 
    memcpy(stackPointer, &softwareStackFrame, sizeof(struct SoftwareStackFrame));
    
    tasks[taskCount].stackPointer = (uint32_t)stackPointer;
    
    
    taskCount++; 
    return tasks[taskCount -1].taskId;
}


const char* return_task_name()
{
    __asm("cpsid i"); //disable irq
    return tasks[nextTaskIndex].taskName;
    __ASM("cpsie i"); //reenable irq
}


void taskDelay(uint32_t delayTime)
{
    __asm("cpsid i"); //disable irq

    // Set task state as blocked
    tasks[nextTaskIndex].taskState = TaskBlocked;

    // Set delayUntil time so we can check if the task should run
    tasks[nextTaskIndex].delayUntil = HAL_GetTick() + delayTime;

    __ASM("cpsie i"); //reenable irq
    
    // This task is not blocked. Need to switch task
    switchTask();
    
}

void switchTask(void)
{
	__asm("cpsid i"); //disable irq


	// If current task is not idle task and
    // task is not blocked 
    // we need to return this task. So set state as Ready
    if(tasks[nextTaskIndex].taskState == TaskRunning)
    {
        tasks[nextTaskIndex].taskState = TaskReady;
    }
        
    int basePrio = -1;

    currentTask = nextTask;

    bool switchIdleTask =true;

    uint32_t index = nextTaskIndex;

    // Search through all tasks
    for(uint32_t i = index;;){
        i++;
        i %= taskCount;

        if(i == index)
            //end of tasks
            //If we can not find nextTask,
            //switchIdleTask is true and switch to it.
            break;

        //If task is blocked check if we pass delayUntil time.
        //If so check priority and set as nextTask.
        if(tasks[i].taskState == TaskBlocked)
        {
            if(tasks[i].delayUntil <= HAL_GetTick())
            {
                switchIdleTask =false;
                
                if(tasks[i].priority > basePrio)
                {
                    basePrio = tasks[i].priority;
                    nextTaskIndex = i;
                }
            }
    
        }
        //If task is not blocked it should be suspended or ready
        //schedule only if ready (not suspended)
        else if(tasks[i].taskState != TaskSuspend)
        {
            switchIdleTask =false;
            if(tasks[i].priority > basePrio)
            {
                basePrio = tasks[i].priority;
                nextTaskIndex = i;
            }
        }
    }
    // All tasks are blocked or suspended so switch to idle task
    if(switchIdleTask){
        nextTask = &tasks[idleTaskIndex];
        nextTaskIndex = idleTaskIndex;

        // If current task is the only task ready,
        // do not switch to idle
        // continue with current task
        if(currentTask->taskState == TaskReady)
        {
            nextTask = currentTask;
            nextTaskIndex = currentTask->taskId;
            tasks[nextTaskIndex].taskState = TaskRunning;
        }
    }
    // Switch to next task
    else{
        tasks[nextTaskIndex].taskState = TaskRunning;
        nextTask = &tasks[nextTaskIndex];
    }

    // If we are in the same task and
    // do not switch.
    // If this is the first taskSwitch and currentTask is NULL
    // continue, SVC_Handler will take care of the first contex switch
    if((nextTask != currentTask) && (currentTask)){
        //Set PendSV interrupt for contex switching.
        SCB->ICSR |= (1<<28);
    }

    __ASM("cpsie i"); //reenable irq
    
}


void TaskYield(void)
{
	switchTask();
}

__attribute__((naked)) 
void SVC_Handler(void)
{
    __asm ("cpsid i");
    __asm("ldr r0, =nextTask");
    __asm("ldr r0, [r0]");
    __asm("ldr r0, [r0]");
    __asm("ldmfd r0!, {r4-r11} ");
    __asm("msr psp, r0             ");
    __asm("cpsie i                 ");
    __asm("MOV r0, #0xFFFFFFFD");
    __asm("bx r0");
}


__attribute__((naked)) 
void PendSV_Handler(void)
{
    __asm("cpsid i"); //disable irq

    //store
    __asm ("mrs r0, psp");
    __asm("ldr r1, =currentTask");
    __asm("ldr r1, [r1]");
    __asm("stmfd r0!, {r4-r11}");
    __asm("str r0, [r1]");

    //load
    __asm("ldr r0, =nextTask ");
    __asm("ldr r0, [r0]");
    __asm("ldr r0, [r0]");
    __asm("ldmfd r0!, {r4-r11}");
    __asm("msr psp, r0 ");

    //return
    __asm("cpsie i   ");
    __asm("MOV r0, #0xFFFFFFFD");
    __asm("bx r0");
    __asm("nop");
}


void KernelStart(void)
{
    idleTaskIndex = TaskCreate("IDLE_TASK",64,idleTask,0);
    tasks[idleTaskIndex].taskState = TaskSuspend; //do not get schedule like other tasks, NEVER resume

    // Set PendSV priority lower than SysTick
    // Because we need to switch task
    // after Systick return we should enter to contex switching
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 2);

    //swith to first task with highest priority
    switchTask();

    //set SVC interrupt for first context switch
    __asm("SVC #0");

    // This function should never return
    // We should never be here
    while(1);
}

void SysTick_Handler(void)
{
    HAL_IncTick();

    //Switch task in every tick
    switchTask();
    
}

