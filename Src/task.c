#include "task.h"


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

static void TaskReturn()
{
    while(1);
}

struct task
{
    uint32_t stackPointer;
};
static uint32_t lastTick = 0;
static struct task tasks[10];

static uint32_t taskCount = 0;
static uint32_t nextTaskIndex = 0;

struct task *currentTask;
struct task *nextTask;


void TaskCreate(uint8_t *stack, uint32_t stackSize, void (*entrypoint)())
{
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
	
    stack += stackSize;
    uint32_t *stackPointer = (uint32_t *)stack;
    stackPointer -= sizeof(struct HardwareStackFrame) / sizeof(uint32_t); //make space for hardware stack frame
    memcpy(stackPointer, &hardwareStackFrame, sizeof(struct HardwareStackFrame));
    
    stackPointer -= sizeof(struct SoftwareStackFrame) / sizeof(uint32_t); //make space for software stack frame
    memcpy(stackPointer, &softwareStackFrame, sizeof(struct SoftwareStackFrame));
    
    tasks[taskCount].stackPointer = (uint32_t)stackPointer;
    
    taskCount++;    
}

void switchTask(void)
{

	__asm("cpsid i"); //disable irq
	currentTask = nextTask;
    nextTaskIndex++;
    nextTaskIndex %= taskCount;
    
    nextTask = &tasks[nextTaskIndex];
    __ASM("cpsie i"); //reenable irq

    
}


void TaskYield(void)
{
	switchTask();
}

__attribute__((naked)) 
void SVC_Handler(void){

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

	currentTask = nextTask;
    nextTaskIndex++;
    nextTaskIndex %= taskCount;
    
    nextTask = &tasks[nextTaskIndex];


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
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 2);
    nextTask = &tasks[nextTaskIndex];
    __asm("SVC #0");

    while(1);
}

void SysTick_Handler(void)
{

    HAL_IncTick();
    if(!(HAL_GetTick() - lastTick >= 5)){
        
        return;
    }
    lastTick = HAL_GetTick();

    SCB->ICSR |= (1<<28);

}