#include <stdint.h>
#include "stm32f3xx_hal.h"



struct node 
{
    uint32_t pid;
    struct node* prev;
    struct node* next;
};

struct queue
{
    struct node* head;
    struct node* tail;
    
};

void enqueue(struct node node, struct queue q)
{
    
}
