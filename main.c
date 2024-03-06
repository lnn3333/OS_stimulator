#include "main.h"

runningP = NULL;
initP = NULL;


// Define helper function to find an available PID
int findPID()
{
    static int availablePID = 0;
    return availablePID++;
};

//comparator function
bool pComparator(void *pItem, void *pComparisonArg){
    const PCB *PCB_pItem = (PCB *)pItem;
    const int *int_pComparisonArg = (int *)pComparisonArg;

    return (PCB_pItem->pid == *int_pComparisonArg);
};

//Search and Kill function
bool searchAndKill (List *PCBlist, int pidSearch){
    PCB *pSearch = List_search(PCBlist,pComparator, pidSearch);
    if (pSearch == NULL){
        fprintf(stderr, "Error: PID %d does not exist\n", pidSearch);
        return false;
    }
    else{
        if (!List_remove(PCBlist)) {
            fprintf(stderr, "Error: Couldn't remove process from the queue\n");
            return false;
        } else {
            printf("Success: Removed process with PID %d from the queue\n", pidSearch);
            return true;
        }
    }
}

// Functions


int createProcess(int priority)
{

    // Allocate memory for new P
    PCB *newPCB = (PCB *)malloc(sizeof(PCB));
    if (newPCB == NULL)
    {
        printf("Error create Process\n");
        return 0;
    }

    // set Process Pid, state, priority
    newPCB->pid = findPID();
    newPCB->priority = priority;
    newPCB->state = READY;
    newPCB->messages = List_create();
    
    if(newPCB->messages==NULL)
    {
        printf("Error create messages list\n");
        return 0;
    }

    // Add Process to queue based on priority
    switch (newPCB->priority)
    {
    case (0):
        List_append(lowPriority, newPCB);
        break;
    case (1):
        List_append(mediumPriority, newPCB);
        break;
    case (2):
        List_append(highPriority, newPCB);
        break;

    default:
        printf("Error appending process to queue");
        break;
    }
    printf("Process created with PID: %d\n", newPCB->pid);

    return newPCB->pid;
};


bool forkProcess()
{
    // check if Process is init
    if (runningP == initP){
        printf("Error: Cannot fork initP");
        return false;
    }

    //copy the runningP, add to readyQ
    PCB* runningPCopy = (PCB*)(malloc(sizeof(PCB)));
    if (runningPCopy == NULL)
    {
        printf("Error create Process\n");
        return;
    }
    memcpy(runningPCopy, runningPCopy, sizeof(PCB));
    runningPCopy->pid = findPID();
    List_append(readyQ, runningPCopy);

    printf("Process forked with PID: %d\n", runningPCopy->pid);
    return true;


};


bool kill(int pid){
    //search for pid in readyQ
    int pPriority = -1;
    if(List_search(readyQ,pComparator,pid) != NULL){
        PCB* pSearch = (PCB*) List_search(readyQ,pComparator,pid);
        searchAndKill(readyQ, pid);
        pPriority = pSearch->pid;
        
    };
    searchAndKill(readyQ,pid);
    if(pPriority!= -1){
        switch (pPriority)
        {
        case 0:
            searchAndKill(lowPriority, pid);
            break;
        case 1:
            searchAndKill(mediumPriority, pid);
            break;
        case 2:
            searchAndKill(highPriority, pid);
            break;
        
        default:
            break;
        }
    }
    else{
        // Search and remove from priority queues only
        if (List_search(lowPriority, pComparator, &pid) != NULL) {
            pPriority = 0;
            searchAndKill(lowPriority, pid);
        } else if (List_search(mediumPriority, pComparator, &pid) != NULL) {
            pPriority = 1;
            searchAndKill(mediumPriority, pid);
        } else if (List_search(highPriority, pComparator, &pid) != NULL) {
            pPriority = 2;
            searchAndKill(highPriority, pid);
        } else {
            printf("Error: Process with PID %d does not exist\n", pid);
            return false;
        }
    }
    return true;
};

void exit(){
    // check if Process is init
    if (runningP == initP){
        printf("Error: Cannot kill initP");
        return false;
    }
    //find P in readyQ
    int runningPid = runningP->pid;
    if(runningPid < 0){
        printf("Error: Cannot get PID of the running process\n");
        return false;
    }
    //Search and Kill running P
    searchAndKill(readyQ, runningPid);
    
    return true;

};


int quantum();


int send(int pid, char *msg){
    //add msg to list messages
    message *new_msg = (message *)malloc(sizeof(message));
    if (new_msg == NULL) {
        printf("Error: Memory allocation failed for message\n");
        return false;
    }

    //populate new_msg
    new_msg->pid = pid;
    strncpy(new_msg->msg, msg, MAX_MSG_LENGTH);
    new_msg->msg[MAX_MSG_LENGTH]='\0';

    //find pid , append msg to messages
    PCB *PCBReceiver; 
    if (List_search(lowPriority, pComparator, &pid) != NULL) {
        PCBReceiver = List_curr(lowPriority);
        if(List_append(PCBReceiver->messages, new_msg)==-1){
            printf("Error: Failed to append message to list\n");
            free(new_msg); // Free the allocated memory
            return 0;
        }        
    } else if (List_search(mediumPriority, pComparator, &pid) != NULL) {
        PCBReceiver = List_curr(mediumPriority);
        if(List_append(PCBReceiver->messages, new_msg)==-1){
            printf("Error: Failed to append message to list\n");
            free(new_msg); // Free the allocated memory
            return 0;
        }
        
    } else if (List_search(highPriority, pComparator, &pid) != NULL) {
        PCBReceiver = List_curr(highPriority);
        if(List_append(PCBReceiver->messages, new_msg)==-1){
            printf("Error: Failed to append message to list\n");
            free(new_msg); // Free the allocated memory
            return 0;
        }
    } else {
        printf("Error: Process with PID %d does not exist\n", pid);
        return false;
    }
    //block P , wait until get reply
    printf("Success send Process with PID %d \n", pid);
    runningP->state = BLOCKED;
    return 1;

};


int receive(){

};

int reply(int pid, char *msg); // null terminate string, MAX lenght of msg = 40 char


int new_sem(int semID, int value){
    if(value < 0 || value > 4){
        printf("Error: value is not in range");
        return 0;
    }

    sem * newSem = (sem *) malloc(sizeof(sem));
    if (newSem == NULL){
        printf("Error: failure create SEM");
        return 0;
    }

    newSem->semID = semID;
    newSem->value = value;
    newSem->pList = List_create();
    if (newSem->pList == NULL){
        printf("Error: failure create Process List for SEM");
        free(newSem); 
        return 0;
    }

    return 1;
};


int P(int semID){

    if (semID < 0 || semID > 4) {
        printf("Error: Semaphore ID out of range\n");
        return 0;
    }
    sem* getSem;
    getSem = &semList[semID];
    getSem->value -- ;
     // If semaphore value is negative, block the process
    if (getSem->value < 0) {
        // Block the process by adding it to the semaphore's process list
        if (!List_append(getSem->pList, runningP)) {
            printf("Error: Failed to add process to semaphore's process list\n");
            return false;
        }

        // Suspend the running process
        suspend();
    }
    return true;



}; //(semID range 0-4)


int V(int semID){
     if (semID < 0 || semID > 4) {
        printf("Error: Semaphore ID out of range\n");
        return 0;
    }
    sem* getSem;
    getSem = &semList[semID];
    getSem->value ++;
    if (getSem->value <= 0) {
            // Check if there are processes waiting on the semaphore
            if (List_count(getSem->pList) > 0) {
                // Get the first process waiting on the semaphore
                PCB *waitingProcess = (PCB *)List_remove(getSem->pList);

                resume(waitingProcess);
            }
        }

        return 1; // Operation succeeded
}


void proc_info(int pid);


void total_info(void);
