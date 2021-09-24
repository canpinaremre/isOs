#pragma once

#include <stdint.h>
#include "stm32f3xx_hal.h"


struct prioq
{
    uint32_t pid;
    uint8_t prio;
};

void insertHeap(struct prioq arr[], struct prioq task,int n);
void heapify(struct prioq arr[], int n, int i);
void buildHeap(struct prioq arr[], int n);
int extract_maximum (struct prioq arr[],int n);
void increase_value (struct prioq arr[], int i, struct prioq val);

struct blockState
{
    uint32_t pid;           // task id
    uint32_t wakeUpTime;    // task unblock time
};

void insertMinHeap(struct blockState arr[], struct blockState task,int n);
void heapifyMin(struct blockState arr[], int n, int i);
int extract_min (struct blockState arr[],int n);
void decrease_value (struct blockState arr[], int i, struct blockState val);
uint32_t peek_min(struct blockState arr[]);