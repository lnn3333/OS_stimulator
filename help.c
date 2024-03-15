#include "help.h"

int findPID()
{
    static int availablePID = 0;
    return availablePID++;
};

// search the PCB base on pid and make current item the found item; return -1 if not found; return the priority
PCB *findPCB(int pid)
{
   
    if (runningP!=NULL && pid == runningP->pid){
        return runningP;
    }

    if (initP!= NULL && pid == initP->pid){
        return initP;
    }

    PCB *PCBSender = NULL; // Initialize to NULL to handle cases where PCB is not found

    void *ptr_pid = &pid; // Pointer to the process ID

    // Find sender ID across different priority levels
    if ((PCBSender = (PCB *)List_search(highPriority, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = (PCB *)List_search(mediumPriority, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = (PCB *)List_search(lowPriority, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = (PCB *)List_search(blockQ, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = (PCB *)List_search(&semList[0].pList, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = (PCB *)List_search(&semList[1].pList, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = (PCB *)List_search(&semList[3].pList, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = (PCB *)List_search(&semList[4].pList, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else
    {
        fprintf(stderr, "Error: Sender with PID %d does not exist\n", pid);
        return NULL; // Return NULL if PCB is not found
    }
}

// allocat new message

message *allocate_message(char *msg)
{
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
        printf("The priority of PCB is %s\n", getPriorityName(pcb->priority));
        printf("The state of PCB is %s\n", getStateName(pcb->state)); 
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

PCB *allocateProcess(int priority)
{
    PCB *newPCB = (PCB *)malloc(sizeof(PCB));
    if (newPCB == NULL)
    {
        printf("Error: cannot create newPCB\n");
        free(newPCB);
        return NULL;
    }

    newPCB->pid = findPID();
    newPCB->priority = priority;
    newPCB->state = READY;
    newPCB->proc_message = List_create();
    if (newPCB->proc_message == NULL)
    {
        printf("Error: cannot create list for PCB\n");
        free(newPCB);
        return NULL;
    }
    return newPCB;
}

void printState(PCB *process)
{
    printf("State of PID %d is %d:", process->pid, process->state);
}

void getRunningP()
{
    printf("PID running is %d\n", runningP->pid);
}

bool add_to_priority(int priority, PCB *item)
{
    if (item == NULL)
    {
        printf("Item is NULL \n");
        return false;
    }
    switch (priority)
    {
    case 0:
        if (List_append(highPriority, item) == -1)
        {
            printf("Error cant add to highPriority Q\n");
        }
        break;
    case 1:
        if (List_append(mediumPriority, item) == -1)
        {
            printf("Error cant add to mediumPriority Q\n");
        }
        break;
    case 2:
        if (List_append(lowPriority, item) == -1)
        {
            printf("Error cant add to lowPriority Q");
        }
        break;

    default:
        printf("Invalid priority number\n");
        if (item != NULL)
        {
            free(item);
        }
        return false;
        break;
    }
    printf("Success add to priorityQ\n");

    return true;
};

bool remove_from_queue(PCB*process, List* pList, enum processState state ){
    if(state == READY){
        switch (process->priority)
    {
    case 0:
        if (List_remove(highPriority) == NULL)
        {
            printf("Error removing from queue \n");
            return false;
        }
        break;
    case 1:
        if (List_remove(mediumPriority) == NULL)
        {
            printf("Error removing from queue \n");
            return false;
        }
        break;
    case 2:
        if (List_remove(lowPriority) == NULL)
        {
            printf("Error removing from queue \n");
            return false;
        }
        break;

    default:
        printf("Error invalid priority \n");
        break;
    }
    }
    else if (state == BLOCKED)
    {
        /* remove from blockQ, then search for block Q in sem bloack List */
    }
    
    
};

const char *getStateName ( enum processState state){
    switch (state)
    {
    case READY: return "READY";
        break;
    case RUNNING: return "RUNNING";
        break;
    case BLOCKED: return "BLOCKED";
        break;
    case DEADLOCK: return "DEADLOCK";
        break;
    
    default:
        return "invalid state";
        break;
    }
};

const char *getPriorityName ( int priority){
    switch (priority)
    {
    case 0: return "High";
        break;
    case 1: return "Medium";
        break;
    case 2: return "Low";
        break;   
    default:
        return "invalid priority";
        break;
    }
};