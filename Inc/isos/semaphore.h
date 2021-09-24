#pragma once

#include "stm32f3xx_hal.h"


typedef struct
{
    uint8_t val;

}Semaphore;


void sem_init(Semaphore *s,int value)
{
    s->val = value;
}

void sem_post(Semaphore *s)
{
    s->val += 1;
}

void sem_wait(Semaphore *s){
    while (s->val == 0);
    s->val -=1;
}