#include "prioq.h"


void swap(struct prioq* x,struct prioq* y){
    int temp_prio = y->prio;
    int temp_pid = y->pid;
    y->prio = x->prio;
    y->pid = x->pid;
    x->prio = temp_prio;
    x->pid = temp_pid;
}

void swapBlock(struct blockState* x,struct blockState* y){
    int wakeUpTime = y->wakeUpTime;
    int temp_pid = y->pid;
    y->wakeUpTime = x->wakeUpTime;
    y->pid = x->pid;
    x->wakeUpTime = wakeUpTime;
    x->pid = temp_pid;
}

// To heapify a subtree rooted with node i which is
// an index in arr[]. N is size of heap
void heapify(struct prioq arr[], int n, int i)
{
    int largest = i; // Initialize largest as root
    int l = 2 * i + 1; // left = 2*i + 1
    int r = 2 * i + 2; // right = 2*i + 2
  
    // If left child is larger than root
    if (l < n && arr[l].prio > arr[largest].prio)
        largest = l;
  
    // If right child is larger than largest so far
    if (r < n && arr[r].prio > arr[largest].prio)
        largest = r;
  
    // If largest is not root
    if (largest != i) {
        swap(&arr[i], &arr[largest]);
  
        // Recursively heapify the affected sub-tree
        heapify(arr, n, largest);
    }
}
  
// Function to build a Max-Heap from the given array
void buildHeap(struct prioq arr[], int n)
{
    // Index of last non-leaf node
    int startIdx = (n / 2) - 1;
  
    // Perform reverse level order traversal
    // from last non-leaf node and heapify
    // each node
    for (int i = startIdx; i >= 0; i--) {
        heapify(arr, n, i);
    }
}

void insertHeap(struct prioq arr[], struct prioq task,int n){
    increase_value(arr, n, task);
}

int extract_maximum (struct prioq arr[],int n)
{
    int pid = arr[0].pid;
    arr[0] = arr[n-1];
    arr[n-1].prio = 0;
    arr[n-1].pid = 0;
    heapify(arr, n-1, 0);
    return pid;
}

void increase_value (struct prioq arr[], int i, struct prioq val)
{
    arr[i].prio = val.prio;
    arr[i].pid = val.pid;
    while( i > 0 && arr[(i-1)/2].prio < arr[i].prio)
    {
        swap(&arr[(i-1)/2], &arr[i]);
        i = (i-1)/2;
    }
}



// Min heap functions
//*****************************************************************************

void insertMinHeap(struct blockState arr[], struct blockState task,int n)
{
    decrease_value(arr, n, task);
}
void heapifyMin(struct blockState arr[], int n, int i)
{
    int largest = i; // Initialize largest as root
    int l = 2 * i + 1; // left = 2*i + 1
    int r = 2 * i + 2; // right = 2*i + 2
  
    // If left child is larger than root
    if (l < n && arr[l].wakeUpTime < arr[largest].wakeUpTime)
        largest = l;
  
    // If right child is larger than largest so far
    if (r < n && arr[r].wakeUpTime < arr[largest].wakeUpTime)
        largest = r;
  
    // If largest is not root
    if (largest != i) {
        swapBlock(&arr[i], &arr[largest]);
  
        // Recursively heapify the affected sub-tree
        heapifyMin(arr, n, largest);
    }
}
int extract_min (struct blockState arr[],int n)
{
    int pid = arr[0].pid;
    arr[0] = arr[n-1];

    heapifyMin(arr, n-1, 0);
    return pid;
}

uint32_t peek_min(struct blockState arr[])
{
    return arr[0].wakeUpTime;
}

void decrease_value (struct blockState arr[], int i, struct blockState val)
{
    arr[i].wakeUpTime = val.wakeUpTime;
    arr[i].pid = val.pid;
    while( i > 0 && arr[(i-1)/2].wakeUpTime > arr[i].wakeUpTime)
    {
        swapBlock(&arr[(i-1)/2], &arr[i]);
        i = (i-1)/2;
    }
}
