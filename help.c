#include "list.h"
#include "help.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


int findPID()
{
    static int availablePID = 0;
    return availablePID++;
};



// Search and Kill function
bool searchAndKill(List *PCBlist, int pidSearch)
{
    void *pidSearch_ptr = (void *)&pidSearch;

    PCB *pSearch = List_search(PCBlist, pComparator, pidSearch_ptr);
    if (pSearch == NULL)
    {
        fprintf(stderr, "Error: PID %d does not exist\n", pidSearch);
        return false;
    }
    else
    {
        if (!List_remove(PCBlist))
        {
            fprintf(stderr, "Error: Couldn't remove process from the queue\n");
            return false;
        }
        else
        {
            printf("Success: Removed process with PID %d from the queue\n", pidSearch);
            return true;
        }
    }
}

// search the PCB base on pid and make current item the found item; return -1 if not found; return the priority 
Node* findPCB(int pid)
{

    Node *PCBSender;
    
    void *ptr_pid = &pid; // Assuming pid is an integer variable

    // find sender ID, unblock sender
    if (List_search(lowPriority, pComparator, ptr_pid) != NULL)
    {
        PCBSender = List_curr(lowPriority);
    }
    else if (List_search(mediumPriority, pComparator, ptr_pid) != NULL)
    {
        PCBSender = List_curr(mediumPriority);
    }
    else if (List_search(highPriority, pComparator, ptr_pid) != NULL)
    {
        PCBSender = List_curr(highPriority);
    }
    else
    {
        printf("Error: SenderID with PID %d does not exist\n", pid);
        return NULL;
    }

    // error handle
    if (PCBSender == NULL)
    {
        printf("Error: cannot locate the sender\n");
        return NULL; // Corrected to return NULL instead of 0
    }

    return PCBSender;
}

//allocat new message

message* allocate_message(char *msg){
    message *new_msg = (message *)malloc(sizeof(message));
    if (new_msg == NULL)
    {
        printf("Error: Memory allocation failed for message\n");
        free(new_msg);
        return NULL;
    }
    // populate new_msg
    new_msg->senderPid = runningP->pid;
    strncpy(new_msg->msg, msg, MAX_MSG_LENGTH);
    new_msg->msg[MAX_MSG_LENGTH] = '\0';
    return new_msg;
}


// display info of the PCB
void total_info_helper(PCB *pcb)
{

    if (pcb == NULL)
    {
        printf("Error: cannot get the Sender PCB\n");
    }
    else // display info
    {
        printf("The pid of PCB is %d\n", pcb->pid);
        printf("The priority of PCB is %d\n", pcb->priority);
        printf("The state of PCB is %d\n", pcb->state);
    }
}

// Process cmd from the terminal
void displayMenu()
{
    printf("\nSimulation Menu:\n");
    printf("C: Create a process\n");
    printf("F: Fork a process\n");
    printf("K: Kill a process\n");
    printf("E: Exit the simulation\n");
    printf("Q: Quantum the simulation\n");
    printf("S: Send message to Process pid\n");
    printf("R: Receive message\n");
    printf("Y: Reply message to Process pid\n");
    printf("N: Create new SEM\n");
    printf("P: Operate sem P operation\n");
    printf("V: Operate sem V operation\n");
    printf("I: Print process info\n");
    printf("T: Print all info\n");
}

// Function to process the user command
// first arg
void processCommand()
{
   while (1) {
        // Get user input (command and parameters)
        char command;
        int priority, pid;
        printf("Enter command and parameter (C <priority>, F, K <pid>, E): ");
        scanf(" %c", &command);

        // Process user input
        switch (toupper(command)) {
            case 'C': {
                scanf("%d", &priority);
                createProcess(priority);
                break;
            }
            case 'F':
                forkProcess();
                break;
            case 'K':
                scanf("%d", &pid);
                kill(pid);
                break;
            case 'E':
                exitProcess();
                break;
            case 'T':
                total_info();
                break;
            default:
                printf("Invalid command.\n");
                break;
        }
    }
}

PCB* allocateProcess(int priority){
    PCB *newPCB = (PCB*) malloc(sizeof(PCB));
    if (newPCB == NULL){
        printf("Error: cannot create newPCB\n");
        free(newPCB);
        return NULL;
    }
    
    newPCB->pid = findPID();
    newPCB->priority = priority;
    newPCB->state = READY;
    newPCB->proc_message = List_create();
    if (newPCB->proc_message == NULL){
        printf("Error: cannot create list for PCB\n");
        free(newPCB);
        return NULL;
    }
    return newPCB;

}