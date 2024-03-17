#include "main.h"
#include <unistd.h>

// Define the variables
List *lowPriority = NULL;
List *mediumPriority = NULL;
List *highPriority = NULL;
List *blockQ = NULL;

PCB *runningP = NULL;
PCB *initP = NULL;
sem *semList = NULL; // Assuming semList is an array of semaphores
// Define helper function to find an available PID

int createProcess(int priority)
{
//allocate memory of Process
    PCB *new_process = allocateProcess(priority);
    if (new_process == NULL)
    {
        printf("Failed to create process.\n");
        return 0;
    }

// add to ready Q
    add_to_priority(priority,new_process);
    printf("Sucess create process\n");
    proc_info(new_process->pid);
    return new_process->pid;
}

bool forkProcess()
{
// check if Process is init
    if (runningP == initP)
    {
        printf("Error: Cannot fork initP");
        return false;
    }

// copy the runningP, add to ready queue
    PCB *runningPCopy = (PCB *)(malloc(sizeof(PCB)));
    if (runningPCopy == NULL)
    {
        printf("Error create Process\n");
        return false;
    }
    memcpy(runningPCopy, runningP, sizeof(PCB));
    runningPCopy->pid = findPID();

//add to readyQ
    if(add_to_priority(runningPCopy->priority,runningPCopy)==false){
       return false; 
    };

    printf("Process forked with PID: %d\n", runningPCopy->pid);
    total_info_helper(runningPCopy);
    return true;
};

bool kill(int pid)
{
// check if it is a initP
    if (pid == initP->pid)
    {
        printf("Error cannot remove initP\n");
        return false;
    }

    if (runningP!=NULL && pid == runningP->pid){
        free(runningP);
        runningP = NULL;
    }
// Search and remove from priority queues only
    remove_from_queue(pid);
    printf("Suceed removing PID %d\n", pid);
    return true;
};

void exitProcess()
{
    // check if all the ready queues are empy
    // If yes, kill the init P
   
    if (List_count(lowPriority) == 0 && List_count(mediumPriority) == 0 && List_count(highPriority) == 0)
    {
        free(initP);
        return;
    }
    else
    {
        if (runningP != NULL){
            int runningP_pid = runningP->pid;
            if (!kill(runningP_pid))
            {
                printf("Error: cannot kill the running PID %d\n", runningP_pid);
                return;
            }
        }

    }
    quantum();
};

int quantum()
{
    bool res = cpu_scheduler();
    if (res == false)
    {
        printf("Error from cpu scheduling\n");
        return 0;
    }
    total_info_helper(runningP);
    return 1;
};

bool send(int pid, char *msg)
{
// allocate message package
    if (runningP == NULL){
        printf("Error: there is no running P\n");
        return false;
    }
    if(pid == runningP->pid){
        printf("Error: cannot send message to sender\n");
        return false;
    }
    message *create_msg = allocate_message(msg);
    if (create_msg == NULL)
    {
        printf("Error create new message\n");
        return false;
    }

// find pid of receiverP
    PCB *receiverP = (PCB *)findPCB(pid);
    if (receiverP == NULL){
        printf("Error: receiver ID is invalid\n");
        return false;
    }
// append message to receiverP

    if (List_append(receiverP->proc_message, create_msg) == -1)
    {
        printf("Error: Failed to append message to list\n");
        free(create_msg); // Free the allocated memory
        return false;
    }

    // block P
    runningP->state = BLOCKED;
    // add block P to blockQ
    if (List_append(blockQ, runningP) == -1)
    {
        printf("Error: Failed to append blockP to list\n");
        return false;
    }

    runningP = NULL;
    printf("Sucess send message to PID %d with msg: %s\n", pid, msg);
    printf("senderID: %d\n", create_msg->senderPid);

    return true;
};

bool receive()
{
    // block P

    PCB *receiveP = runningP;
    receiveP->state = BLOCKED;

    // dequeye message from list of messages, check if has message or not
    message *receive_msg = List_trim(runningP->proc_message);

    if (receive_msg == NULL)
    {
        printf("Error: cannot get the receive msg");
        // add to blockQ
        PCB *blockP = runningP;
        blockP->state = BLOCKED;
        runningP = NULL;
        if (List_append(blockQ, blockP) == -1)
        {
            printf("Error: Failed to append blockP to list\n");
            return false;
        }

        return false;
    }

    // get sender id
    int senderID = receive_msg->senderPid;

    // reply to sender
    char *reply_msg = "Reply message\n";
    int resultReply = reply(senderID, reply_msg);
    if (resultReply == 0)
    {
        printf("Error: cannot reply to sender with pid %d \n", senderID);
        return 0;
    }
    // unblock P
    receiveP->state = READY;
    if(add_to_priority(receiveP->priority,receiveP)==false){
        return 0;
    };
    return true;
};

bool reply(int pid, char *msg)
{

    printf("%s\n", msg);
    PCB *PCB_sender = (PCB *) findPCB(pid);

    if (PCB_sender == NULL)
    {
        printf("Error: cannot get the Sender PCB\n");
        return false;
    }
    else
    {
        PCB_sender->state = READY;
        if (add_to_priority( PCB_sender->priority, PCB_sender) == false)
        {
            return false;
        };
        List_remove(blockQ);
        printf("Unblock Process pid %d \n", PCB_sender->pid);
    }
    return true;
};

bool new_sem(int semID)
{
    if (semID < 0 || semID > 4)
    {
        printf("Error: value is not in range\n");
        return false;
    }

    sem* semItem = &semList[semID];
    semItem->value = 1;
    semItem->pList = List_create();
    if (semItem->pList == NULL)
    {
        printf("Error: failure create Process List for SEM\n");
        free(semItem);
        return false;
    }

    return true;
};

bool P(int semID)
{

    if (semID < 0 || semID > 4)
    {
        printf("Error: Semaphore ID out of range\n");
        return 0;
    }
    sem *getSem;
    getSem = &semList[semID];
    getSem->value--;

// If semaphore value is negative, block the process
    if (getSem->value < 0)
    {
// Block the process by adding it to the semaphore's process list

        PCB *blockP = runningP;
        if (!List_append(getSem->pList, blockP))
        {
            printf("Error: Failed to add process to semaphore's process list\n");
            return false;
        }
        blockP->state = BLOCKED;
    }
    printf("Sucess Block the process on sem ID %d\n", semID);
    return true;

}; 

bool V(int semID)
{
    PCB *waitingProcess;

    if (semID < 0 || semID > 4)
    {
        printf("Error: Semaphore ID out of range\n");
        return false;
    }
    sem *getSem;
    getSem = &semList[semID];
    getSem->value++;
    if (getSem->value <= 0)
    {
        // Check if there are processes waiting on the semaphore
        if (List_count(getSem->pList) > 0)
        {
            // Get the first process waiting on the semaphore
            List_first(getSem->pList);
            waitingProcess = (PCB *)List_remove(getSem->pList);

            //unblock the process, add back to readyQ
            waitingProcess->state = READY;
            if ( add_to_priority(waitingProcess->priority, waitingProcess) == false)
            {
                return false;
            };

            //print info of the wakeup P
            printf("Wake up Process PID %d\n",waitingProcess->pid);
        }
    }

    return true; // Operation succeeded
}

void proc_info(int pid)
{
    PCB *process = (PCB *) findPCB(pid);

    if (process == NULL)
    {
        printf("Error: PCB pointer is NULL for PID %d\n", pid);
        return; // Return early to avoid dereferencing NULL pointer
    }
    total_info_helper(process);
}

void total_info(void)
{
    // Define an array of pointers to the priority queues
    List *priorityQueues[4] = {highPriority, mediumPriority, lowPriority, blockQ};
    // array of pList of Semaphore

    // Loop through each priority queue
    for (int i = 0; i < 4; i++)
    {
        List *priorityQueue = priorityQueues[i];
        // Check if the priority queue is not empty
        if (List_count(priorityQueue) > 0)
        {
            Node *node_ptr = priorityQueue->pFirstNode;

            // Loop through each node in the priority queue
            while (node_ptr != NULL)
            {
                PCB *PCB_ptr = (PCB *)node_ptr->pItem;
                total_info_helper(PCB_ptr);
                node_ptr = node_ptr->pNext;
            }
        }
    }
    if(runningP!=NULL){
        total_info_helper(runningP);
    }
    
    return;
}

void Init()
{
    // Initialis priority queues
    lowPriority = List_create();
    if (lowPriority == NULL)
    {
        printf("Error: cannot create lowPriority\n");
    }

    mediumPriority = List_create();
    if (mediumPriority == NULL)
    {
        printf("Error: cannot create mediumPriority\n");
    }

    highPriority = List_create();
    if (highPriority == NULL)
    {
        printf("Error: cannot create highP\n");
    }

    blockQ = List_create();
    if (blockQ == NULL)
    {
        printf("Error: cannot create highP\n");
    }

    // Initialize List for the semaphore
   semList = (sem*) malloc(sizeof(sem)*5);

    // Initialize initP
    initP = allocateProcess(3);
    runningP = initP;

}

// Function to process the user command
// first arg
void processCommand()
{
    while (1)
    {
        // Get user input (command and parameters)
        char command;
        int priority, id;
        char *msg = (char *)malloc(MAX_MSG_LENGTH * sizeof(char)); // Allocate memory for msg

        // Check if memory allocation was successful
        if (msg == NULL)
        {
            // Handle memory allocation failure
            printf("Error: Unable to allocate memory for message.\n");
            exit(EXIT_FAILURE); // Or take appropriate action
        }

        printf("Enter command and parameter (C <priority>, F, K <pid>, E, T): ");
        scanf(" %c", &command);

        // Process user input
        switch (toupper(command))
        {
        case 'C':
        {
            scanf("%d", &priority);
            createProcess(priority);
            break;
        }
        case 'F':
            forkProcess();
            break;
        case 'K':
            scanf("%d", &id);
            kill(id);
            break;
        case 'E':
            exitProcess();
            break;
        case 'Q':
            quantum();
            break;
        case 'S':
            scanf("%d", &id);
            scanf("%s", msg);
            send(id, msg);
            break;
        case 'R':
            receive();
            break;
        case 'Y':
            scanf("%d", &id);
            scanf("%s", msg);
            reply(id, msg);
            break;
        case 'N':
            scanf("%d", &id);
            new_sem(id);
            break;
        case 'P':
            scanf("%d", &id);
            P(id);
            break;
        case 'V':
            scanf("%d", &id);
            V(id);
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


bool cpu_scheduler()
{
    if(runningP != initP && runningP != NULL && runningP->state != BLOCKED){
        PCB *resumeP = runningP;
        resumeP->priority = 2;
        if (List_append(lowPriority, resumeP) == -1)
        {
            printf("Error adding process to low priority \n");
            return false;
        }
    }
    

    PCB *nextP;
    if (List_count(highPriority) > 0)
    {
        List_first(highPriority);
        nextP = (PCB *)List_remove(highPriority);
    }
    else if (List_count(mediumPriority) > 0)
    {
        List_first(mediumPriority);
        nextP = (PCB *)List_remove(mediumPriority);
    }
    else if (List_count(lowPriority) > 0)
    {
        List_first(lowPriority);
        nextP = (PCB *)List_remove(lowPriority);
    }
    else
    {
        nextP = initP;
    }

    if(nextP == NULL){
        printf("Error: getting next P\n");
    }
    runningP = nextP;
    runningP->state = RUNNING;
    return true;
};

int main(int argc, char *argv[])
{
    Init();

    //    displayMenu();
    while (1)
    {
        processCommand();
    }

    return 0;
}
