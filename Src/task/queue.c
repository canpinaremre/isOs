#include <stdint.h>
#include <stdlib.h>
#include "stm32f3xx_hal.h"



struct node 
{
    uint32_t data;
    struct node* prev;
    struct node* next;
};

struct queue
{
    struct node* head;
    struct node* tail;
    
};

void enqueue(uint32_t new_data, struct queue *q)
{
    struct node *new_node = (struct node *)malloc(sizeof(struct node));
    new_node->data = new_data;

    if(!q->head)
    {
        q->head = new_node;
        q->tail = new_node;
        new_node->next = NULL;
        new_node->prev = NULL;
        return;
    }
    
    q->tail->next = new_node;
    new_node->prev = q->tail;

    q->tail = new_node;
    new_node->next = NULL;

    return;
}

uint32_t dequeue(struct queue *q)
{
    uint32_t ret = -1;
    if(q->tail)
    {
        ret = q->tail->data;
    }
    else
    {
        return ret;
    }
    
    if(!q->tail->prev)
    {
        q->tail = NULL;
        q->head = NULL;
        return ret;
    }
    q->tail = q->tail->prev;
    free(q->tail->next);
    q->tail->next = NULL;
    return ret;
}

void enqueue_after(struct node *node, uint32_t new_data)
{
    struct node* new_node = new struct node;
    new_node->data = new_data;

    new_node->prev = node;
    new_node->next = node->next;
    node->next->prev = new_node;
    node->next = new_node;

}

uint32_t dequeue_node(struct node *node)
{
    uint32_t ret = -1;

    if(!node)
    {
        return ret;
    }

    ret = node->data;

    node->prev->next = node->next;
    node->next->prev = node->prev;
    free(node);



    return ret;
}