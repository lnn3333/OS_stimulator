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

    List *queues[9] = {highPriority, mediumPriority, lowPriority, blockQ, semList[0].pList, semList[1].pList, semList[2].pList, semList[3].pList, semList[4].pList};
    
    for (int i = 0; i < 9; i++)
    {
        List *queue = queues[i];
    
        if ((PCBSender = (PCB *)List_search(queue, pComparator, ptr_pid)) != NULL)
        {
            return PCBSender;
        }
    }
    return NULL;
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
    else if (pcb->pid == initP->pid) // display info
    {
        printf("IinitP with pid %d\n",pcb->pid); 
    }
    else // display info
    {
        printf("The pid of PCB is %d\n", pcb->pid);
        printf("The priority of PCB is %s\n", getPriorityName(pcb->priority));
        printf("The state of PCB is %s\n", getStateName(pcb->state)); 
    }
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
    newPCB->proc_message = NULL;
    return newPCB;
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

    return true;
};

bool remove_from_queue(int pid ){

    PCB *PCBSender = NULL; // Initialize to NULL to handle cases where PCB is not found
    // Find sender ID across different priority levels

     List *queues[9] = {highPriority, mediumPriority, lowPriority, blockQ, semList[0].pList, semList[1].pList, semList[2].pList, semList[3].pList, semList[4].pList};
    
    for (int i = 0; i < 9; i++)
    {
        List *queue = queues[i];
    
        PCBSender = (PCB *)List_search(queue, pComparator, (void *)&pid);
        if (PCBSender != NULL)
        {
            List_remove(queue);
            free(PCBSender);
            return true;
        }
    
    }

    return false; 
    
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

const char *getQueueName ( int n){
    switch (n)
    {
    case 0: return "High Priority Queue";
        break;
    case 1: return "Medium Priority Queue";
        break;
    case 2: return "Low  Priority Queue";
        break;   
    case 3: return "Block Process by Send() or Receive()";
        break;   
    case 4: return "Block Process by Semaphore 0 Queue";
        break;   
    case 5: return "Block Process by Semaphore 1 Queue";
        break;   
    case 6: return "Block Process by Semaphore 2 Queue";
        break;   
    case 7: return "Block Process by Semaphore 3 Queue";
        break;   
    case 8: return "Block Process by Semaphore 4 Queue";
        break;   
    default:
        return "\n";
        break;
    }
};