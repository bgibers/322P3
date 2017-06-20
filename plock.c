/*
Brendan Giberson
June 19 2017
CPSC 3220
Project 3


compile your source file with my test driver "main.c" using

    gcc -Wall plock.c main.c -pthread
    ./a.out
    valgrind --tool=helgrind ./a.out

*/
#include "plock.h"
#include <stdlib.h>

void reorder(plock_t *plock, node_t *node);

/*
This function calls malloc() to allocate space for an instance of the
plock_t data type, initializes the plock value to FREE, initializes
the mutex mlock using the appropriate pthread library call,
appropriately initializes the waiting list head pointer, and returns
a pointer to the instance.
*/
plock_t *plock_create()
{
    plock_t* plock = malloc(sizeof(plock_t));//init and malloc
    //node_t* node; //does this work?
    plock->head = malloc(sizeof(node_t));//init head ptr 
    pthread_mutex_init(&plock->mlock,NULL); //mutex init call
    plock->head->priority = 0;
    plock->value = FREE;
    plock->head->next = plock->head; //create circular list

    return plock;
    
}

/*
This function first garbage collects any nodes remaining on the waiting
list by destroying the condition variables using the appropriate pthread
library call and freeing the nodes. It then destroys the mutex mlock
using the appropriate pthread library call and finally frees the plock
data structure itself.
*/
void plock_destroy(plock_t *plock)
{
    
    while(plock->head->next != plock->head)
    {
        //pthread_cond_destroy(&plock->head->next);
        node_t* node = plock->head->next;
        plock->head->next = plock->head->next->next;
        free(node);
    }//garbage collect

    if(plock->head->next == plock->head)
    {
        pthread_mutex_destroy(&plock->mlock);
        free(plock);
    } //destroy mutex mlock and free structure when no nodes are waiting

}

/*
This function checks the state variables of the plock data structure to
determine if the calling thread can proceed or instead must be added to
a waiting list. If the thread must be added, a node instance is created
and the condition variable within the node must be initialized using the
appropriate pthread library call. (The creation and addition could be
structured as a private helper function if you wish. Helper functions
should be contained in the plock.c source file.) The thread can then
wait on the condition variable in the node.
*/
void plock_enter(plock_t *plock, int priority)
{
    if(plock->value != BUSY)
    {
        plock->value = BUSY;
    }//can proceed

    else if(plock->value == BUSY)
    {
        node_t* node = malloc(sizeof(node_t));//node instance created
        node->priority = priority;
        pthread_cond_init(&node->waitCV, NULL);//init the thread

        reorder(plock,node); //reorder and set wait condition

    }//needs to be waitlisted

    else
    {
        printf("Something went wrong");
    }
}

/*Helper function of plock_enter to ensure proper priority ordering.*/
void reorder(plock_t *plock, node_t *node)
{
    node_t* temp_head = plock->head;
    node_t* nextNode = plock->head->next;

    while(nextNode->priority >= node->priority)
    {
        if(nextNode == plock->head){break;}//terminate loop next is pointing to head

        temp_head = temp_head->next; //iterate nodes until priority is correct
        nextNode = nextNode->next;
    }
        
    temp_head->next = node;
    node->next = nextNode;
    pthread_cond_wait(&node->waitCV,&plock->mlock); //set wait 
}

/*
This function checks the state variables of the plock data structure to
determine what update action to take. For example, it may need to signal
a waiting thread using the appropriate pthread library call.
*/
void plock_exit(plock_t *plock)
{
    pthread_mutex_unlock(&plock->mlock);

    if(plock->head->next == plock->head)
    {
        plock->value = FREE;
    } //simply free the value

    else if(plock->head->next != plock->head)
    {
        node_t* node = plock->head->next; //temp holder for signal
        plock->head->next = plock->head->next->next; //incr heads next
        pthread_cond_signal(&node->waitCV); //notify threads 
    }

}



