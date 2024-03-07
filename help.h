#ifndef _HELPER_H_
#define _HELPER_H_

#include "list.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int findPID();
// Search and Kill function
bool searchAndKill(List *PCBlist, int pidSearch);
// search the PCB base on pid and return the pointer to the PCB; return NULL if not found
Node* findPCB(int pid);

// display info of the PCB
void total_info_helper(PCB *pcb);

message* allocate_message(char *msg);

void displayMenu();

void processCommand();

PCB* allocateProcess(int priority);




#endif




