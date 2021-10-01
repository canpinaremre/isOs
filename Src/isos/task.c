#include "task.h"

static struct task tasks[MAX_TASKS];

#ifdef PRIORITY_SCHEDULER
static struct prioq priority_queue[MAX_TASKS];
static struct blockState block_queue[MAX_TASKS];
static uint32_t queued_tasks_count = 0;
static uint32_t queued_block_count = 0;
#endif

static uint32_t taskCount = 0;
static uint32_t nextTaskIndex = 0;
static uint32_t idleTaskIndex = 0;
static bool kernelStarted = false;
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

#ifdef WQUEUE
static void wq_scheduler()
{


}

#endif

#ifdef USE_STACK_TASK
taskid_t TaskCreateStatic(const char* name, uint32_t stackSize, void (*entrypoint)(), uint8_t priority)
{
    enter_critical_section();

    uint32_t t_id = taskCount;
    bool foundDeleted = false;
    for(int i= 0; i <= taskCount; i++)
    {
        if(tasks[i].taskState == TaskDeleted)
        {
            foundDeleted = true;
            t_id = i;
            break;
        }
    }

    if(taskCount >= MAX_TASKS)
    {
        return MAX_TASKS;
    }

    tasks[t_id].isStatic = true;
    tasks[t_id].stackSize = stackSize;
    tasks[t_id].heapPtr = NULL;
    tasks[t_id].priority = priority;
    strcpy(tasks[t_id].taskName,name);
    tasks[t_id].taskId = (taskid_t)t_id;
    tasks[t_id].taskState = TaskReady;
    tasks[t_id].delayUntil = 0;

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
    
    tasks[t_id].stackPointer = (uint32_t)stackPointer;
    
    
    if(!foundDeleted){
        taskCount++;
    }

    #ifdef PRIORITY_SCHEDULER
    insert_queue(tasks[t_id].taskId,tasks[t_id].priority);
    #endif

    exit_critical_section();
    return tasks[t_id].taskId;
}
#endif
// Create task using heap
taskid_t TaskCreate(const char* name, uint32_t stackSize, void (*entrypoint)(), uint8_t priority)
{
    enter_critical_section();
    uint32_t t_id = taskCount;
    bool foundDeleted = false;
    for(int i= 0; i <= taskCount; i++)
    {
        if(tasks[i].taskState == TaskDeleted)
        {
            foundDeleted = true;
            t_id = i;
            break;
        }
    }

    if(taskCount >= MAX_TASKS)
    {
        return MAX_TASKS;
    }

    #ifdef USE_STACK_TASK
    tasks[t_id].isStatic = false;
    #endif
    tasks[t_id].stackSize = stackSize;
    tasks[t_id].priority = priority;
    strcpy(tasks[t_id].taskName,name);
    tasks[t_id].taskId = (taskid_t)t_id;
    tasks[t_id].taskState = TaskReady;
    tasks[t_id].delayUntil = 0;

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
    tasks[t_id].heapPtr = heap_ptr;
    heap_ptr += stackSize;


    //make space for hardware stack frame
    uint32_t *stackPointer = (uint32_t *)heap_ptr;
    stackPointer -= sizeof(struct HardwareStackFrame) / sizeof(uint32_t); 
    memcpy(stackPointer, &hardwareStackFrame, sizeof(struct HardwareStackFrame));
    
    //make space for software stack frame
    stackPointer -= sizeof(struct SoftwareStackFrame) / sizeof(uint32_t); 
    memcpy(stackPointer, &softwareStackFrame, sizeof(struct SoftwareStackFrame));
    
    tasks[t_id].stackPointer = (uint32_t)stackPointer;
    
    if(!foundDeleted){
        taskCount++;
    }

    #ifdef PRIORITY_SCHEDULER
    insert_queue(tasks[t_id].taskId,tasks[t_id].priority);
    #endif

    exit_critical_section();
    return tasks[t_id].taskId;
}

//delete dynamicly allocated task
void taskDelete(taskid_t tid)
{
    #ifdef USE_STACK_TASK
    if(tasks[tid].isStatic == true)
    {
        //TODO: return errno
        tasks[tid].taskState = TaskSuspend;
        return;
    }
    #endif // USE_STACK_TASK

    if(tasks[tid].taskState == TaskDeleted || TaskEmpty)
    {
        return;
    }
    
    enter_critical_section();

    //set task state as deleted
    tasks[tid].taskState = TaskDeleted;

 

    //free allocated memory
    free(tasks[tid].heapPtr);

    // If we are deleting running task
    if(tid == nextTaskIndex)
    {
        switchTask();
        //Call SVC handler beacuse we do not need to switch context
        //just load nextTask contex
        __asm("SVC #0");
    }

    exit_critical_section();
}

void enter_critical_section(void)
{
    __ASM("cpsid i"); //disable irq
}

void exit_critical_section(void)
{
    __ASM("cpsie i"); //reenable irq
}

const char* return_task_name()
{
    return tasks[nextTaskIndex].taskName;
}

taskid_t getTaskId()
{
    return nextTaskIndex;
}

#ifdef PRIORITY_SCHEDULER
void taskDelay(uint32_t delayTime)
{   
    enter_critical_section();
    
    // Set task state as blocked
    tasks[nextTaskIndex].taskState = TaskBlocked;

    // Set delayUntil time so we can check if the task should run
    tasks[nextTaskIndex].delayUntil = HAL_GetTick() + delayTime;

    struct blockState item;
    item.wakeUpTime = HAL_GetTick() + delayTime;
    item.pid = nextTaskIndex;

    // Insert to the block queue
    insertMinHeap(block_queue,item,queued_block_count);
    queued_block_count++;

    // As this task is blocked now on, need to switch task
    switchTask();

    exit_critical_section();
    
    
    
}

void checkBlockedTasks(void)
{   
    // If block queue is empty, return
    if(queued_block_count == 0)
    {
        return;
    }

    uint32_t now = HAL_GetTick();

    // check block time is passed or not
    while(now >= peek_min(block_queue))
    {
        uint32_t pid = extract_min(block_queue,queued_block_count);
        queued_block_count--;

        struct prioq item;
        item.prio = tasks[pid].priority;
        item.pid = pid;
        // task can be deleted or suspended.
        // extract them too but do not queue
        if(tasks[pid].taskState == TaskBlocked)
        {
            tasks[pid].taskState = TaskReady;
            insertHeap(priority_queue,item,queued_tasks_count);
            queued_tasks_count++;
        }

        // Check count before start loop again
        if(queued_block_count == 0)
        {
            break;
        }
    }

}

void insert_queue(uint32_t pid,uint8_t prio)
{
    struct prioq item;
    item.prio = prio;
    item.pid = pid;
    insertHeap(priority_queue,item,queued_tasks_count);
    queued_tasks_count++;
}

void switchTask(void)
{
   

    // Check if current task is running. We can switch task from task delay
    // or task delete.
    if(tasks[nextTaskIndex].taskState == TaskRunning)
    {
        // Set currently running task as ready
        tasks[nextTaskIndex].taskState = TaskReady;
    }

    // Save currently running task's pointer
    currentTask = nextTask;

    // Check blocked task
    // if they should wake up, add them to the priority queue
    checkBlockedTasks();

    if(queued_tasks_count == 0)
    {
        nextTaskIndex = idleTaskIndex;
    }
    // Check if we have any task in the priority queue
    // We can use IF here but in rare cases queued task can be deleted 
    // or suspeded. So we should switch to next ready task
    while(queued_tasks_count > 0)
    {
        // Extract pid from queue and set as nextTaskIndex
        nextTaskIndex = extract_maximum(priority_queue,queued_tasks_count);
        queued_tasks_count--;

        // Task can be deleted or suspended after it is blocked
        // If the task is not ready set nextTask as idle
        // in case queued_tasks_count can be 0 now
        if(tasks[nextTaskIndex].taskState != TaskReady)
        {
            nextTaskIndex = idleTaskIndex;
        }
        else
        {
            // If task is ready break the loop
            break;
        }

    }

    // It is possible that only currently running task is ready
    // and there are not any ready task
    // in this case we do not want to switch to idle task
    // Check if the next task is idle
    if(nextTaskIndex == idleTaskIndex)
    {   
        // If then, check current task state
        if(currentTask->taskState == TaskReady)
        {
            // Do not switch to idle from current task
            nextTaskIndex = currentTask->taskId;
        }
    }
    
    // In this point nextTaskIndex already determined
    // We will check if we should switch task or continue with the same task

    // If we are in same task (can be idle)
    if(nextTaskIndex == currentTask->taskId)
    {
        // Do not switch and
        // do not add to the queue
        // Just as state as running again
        tasks[nextTaskIndex].taskState = TaskRunning;
        nextTask = &tasks[nextTaskIndex];
    }
    // We are switching to diffrent task now
    else
    {  
        // Reqeueue current task if it is ready and not NULL
        if((currentTask->taskState == TaskReady) && (currentTask))
        {   
            struct prioq item;
            item.prio = currentTask->priority;
            item.pid = currentTask->taskId;

            insertHeap(priority_queue,item,queued_tasks_count);
            queued_tasks_count++;
        }

        // Set nextTask pointer and set state as running
        nextTask = &tasks[nextTaskIndex];
        tasks[nextTaskIndex].taskState = TaskRunning;

        // Set interrupt to switch task
        // If this is first task switch currentTask is NULL
        // continue without interrupt, SVC will handle the
        // first contex switch
        if(currentTask){
            SCB->ICSR |= (1<<28);
        }
    }


    
}

#endif // PRIORITY_SCHEDULER

#ifdef ROUND_ROBIN_SCHEDULER
void switchTask(void)
{


    currentTask = nextTask;

    // Set currently running task as ready
    if(tasks[nextTaskIndex].taskState == TaskRunning)
    {
        tasks[nextTaskIndex].taskState = TaskReady;
    }

    // Search for the next ready task. 
    // In round-robin we do not want to lose time in idle
    while(1)
    {
        nextTaskIndex++;
        nextTaskIndex %= taskCount;
        if(tasks[nextTaskIndex].taskState == TaskReady )
        {
            break;
        }
    }

    // Set selected task as running
    tasks[nextTaskIndex].taskState = TaskRunning;
    // Set pointer for task switching 
    nextTask = &tasks[nextTaskIndex];

    // Switch if this is not first task switch
    // and if we are not in the same task
    if(currentTask && currentTask != nextTask){
       SCB->ICSR |= (1<<28);
    }
    

    
}

void taskDelay(uint32_t delayTime)
{
    HAL_Delay(delayTime);
}

#endif //ROUND_ROBIN_SCHEDULER

void TaskYield(void)
{
    enter_critical_section();
	switchTask();
    exit_critical_section();
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
    

    // Set PendSV priority lower than SysTick
    // Because we need to switch task
    // after Systick return we should enter to contex switching
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 15);

    // Swith to first task with highest priority
    enter_critical_section();
    switchTask();
    exit_critical_section();

    // Set SVC interrupt for first context switch
    kernelStarted = true;
    __asm("SVC #0");

    // This function should never return
    // We should never be here
    while(1);
}

void KernelInit(void)
{
    for(int i = 0; i < MAX_TASKS; i++)
    {
        tasks[i].taskState = TaskEmpty;
    }

    idleTaskIndex = TaskCreate("IDLE_TASK",64,idleTask,0);
}

void SysTick_Handler(void)
{
    HAL_IncTick();

    // Switch task in every tick
    if(kernelStarted)
    {
        enter_critical_section();
        switchTask();
        exit_critical_section();
    }
    
}

void top_tasks(void)
{
#ifdef ROUND_ROBIN_SCHEDULER
    char sendBuffer[60];
    uint32_t counter = 0;
    shellPrint("pid      stackSize      task_name");
    for(int i = 0; i < taskCount; i++)
    {
        if((tasks[i].taskState == TaskReady) || (tasks[i].taskState == TaskRunning))
        {
            memset(sendBuffer, 0, sizeof(sendBuffer));
            sprintf(sendBuffer, "%d             %ld           ",i, tasks[i].stackSize);
            strncat(sendBuffer, tasks[i].taskName, strlen(tasks[i].taskName));
            shellPrint(sendBuffer);
            counter++;
        }
    }
    sprintf(sendBuffer,"Total running threads : %ld",counter);
    shellPrint(sendBuffer);

#endif //ROUND_ROBIN_SCHEDULER

#ifdef PRIORITY_SCHEDULER
    taskid_t pid_array[MAX_TASKS];
    int j= 0;
    enter_critical_section();
    for(int i = 0; i < queued_tasks_count; i++)
    {
        pid_array[j] = priority_queue[i].pid;
        j++;
    }
    for(int i = 0; i < queued_block_count; i++)
    {
        pid_array[j] = block_queue[i].pid;
        j++;
    }
    exit_critical_section();


    char sendBuffer[70];
    shellPrint("pid  priority      stackSize      task_name");
    for(int i = 0; i < j; i++)
    {
        memset(sendBuffer, 0, sizeof(sendBuffer));
        sprintf(sendBuffer, "%ld     %d            %ld           ",pid_array[i],tasks[pid_array[i]].priority, tasks[pid_array[i]].stackSize);
        strncat(sendBuffer, tasks[pid_array[i]].taskName, strlen(tasks[pid_array[i]].taskName));
        shellPrint(sendBuffer);
    }
    sprintf(sendBuffer,"Total running threads : %d",j);
    shellPrint(sendBuffer);


#endif //PRIORITY_SCHEDULER
}

int kill_task(taskid_t pid)
{
    if (pid == idleTaskIndex)
    {
        shellPrint("Error: idleTask can not be killed!");
        return -1;
    }
#ifdef USE_STACK_TASK
    if(tasks[pid].isStatic)
    {
        shellPrint("Error:Task is static. Can not kill. It is suspended");
        tasks[pid].taskState = TaskSuspend;
        return -1;
    }
#endif //USE_STACK_TASK
    if( (tasks[pid].taskState == TaskDeleted)||
    (tasks[pid].taskState == TaskEmpty))
    {
        char sendBuffer[60];
        sprintf(sendBuffer, "Error:Task with pid: %ld do not exist!",pid);
	    shellPrint(sendBuffer);
        return -1;
    }
    taskDelete(pid);

    return 0;
}
