#ifndef _HELPER_H_
#define _HELPER_H_


#include "structure.h"



int findPID();

// search the PCB base on pid and return the pointer to the PCB; return NULL if not found
Node* findPCB(int pid);

// display info of the PCB
void total_info_helper(PCB *pcb);

message* allocate_message(char *msg);

void displayMenu();

PCB* allocateProcess(int priority);




#endif




