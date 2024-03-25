#ifndef _HELPER_H_
#define _HELPER_H_


#include "structure.h"



int findPID();

// search the PCB base on pid and return the pointer to the PCB; return NULL if not found
PCB* findPCB(int pid);

// display info of the PCB
void total_info_helper(PCB *pcb);

message* allocate_message(char *msg);

PCB* allocateProcess(int priority);

bool add_to_priority(int priority, PCB* item);

bool remove_from_queue(int pid );

const char *getStateName ( enum processState);

const char *getPriorityName ( int priority);

const char *getQueueName ( int n);

#endif




