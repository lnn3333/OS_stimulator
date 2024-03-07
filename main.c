#include "main.h"


// Define the variables
List *highPriority = NULL;
List *mediumPriority = NULL;
List *lowPriority = NULL;

PCB *runningP = NULL;
PCB *initP = NULL;
sem semList[5]; // Assuming semList is an array of semaphores
// Define helper function to find an available PID

int createProcess(int priority)
{

    PCB *new_process = allocateProcess(priority);
    if (new_process == NULL)
    {
        printf("Failed to create process.\n");
        return 0;
    }

    // add to ready Q
    switch (priority)
    {
    case 0:
        if (List_append(lowPriority, new_process) == -1)
        {
            printf("Error cant add to lowPriority Q\n");
        }
        break;
    case 1:
        if (List_append(mediumPriority, new_process) == -1)
        {
            printf("Error cant add to mediumPriority Q\n");
        }
        break;
    case 2:
        if (List_append(highPriority, new_process) == -1)
        {
            printf("Error cant add to highPriority Q");
        }
        break;

    default:
        printf("Invalid priority number\n");
        if (new_process != NULL)
        {
            free(new_process);
        }
        return 0;
        break;
    }

    printf("List count is %d \n", List_count(mediumPriority));

    printf("newPCB_pid is %d\n", new_process->pid);
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

    switch (runningPCopy->priority)
    {
    case (0):
        List_append(lowPriority, runningPCopy);
        break;
    case (1):
        List_append(mediumPriority, runningPCopy);
        break;
    case (2):
        List_append(highPriority, runningPCopy);
        break;

    default:
        printf("Error appending process to queue");
        return false;
        break;
    }
    printf("Process forked with PID: %d\n", runningPCopy->pid);
    return true;
};

bool kill(int pid)
{
    // check if it is a initP
    int initP_id = initP->pid;
    if (pid == initP_id)
    {
        printf("Error cannot remove initP\n");
        return false;
    }
    // Search and remove from priority queues only

    Node *findPid_node = findPCB(pid);
    PCB *PCB_p = (PCB *)(findPid_node->pItem);
    int find_pid_priority = PCB_p->pid;
    switch (find_pid_priority)
    {
    case 0:
        if (List_remove(lowPriority) == NULL)
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
        if (List_remove(highPriority) == NULL)
        {
            printf("Error removing from queue \n");
            return false;
        }
        break;

    default:
        printf("Error removing from queue \n");
        return 0;

        break;
    }

    return true;
};

void exitProcess()
{
    // check if all the ready queues are empy
    // If yes, kill the init P
    if (List_count(highPriority) == 0 && List_count(mediumPriority) == 0 && List_count(lowPriority) == 0)
    {
        free(initP);
        return;
    }
    else
    {
        if (kill(runningP->pid))
        {
            printf("Error: cannot kill the running PID %d\n", runningP->pid);
            return;
        }
        else
        {
            printf("Success exit\n");
        }
    }

    // get the next running P
    Node *next_runningP_node = List_first(highPriority);
    if (next_runningP_node == NULL)
    {
        printf("Error: cannot get the next running P\n");
        return;
    }
    PCB *next_runningP = next_runningP_node->pItem;
    runningP = next_runningP;
    total_info_helper(runningP); // print info of next running P

    return;
};

int quantum();

int send(int pid, char *msg)
{
    // add msg to list messages
    message *create_msg = allocate_message(msg);
    if (create_msg == NULL)
    {
        printf("Error create new message\n");
        return 0;
    }

    // find pid of receiverP
    Node *receiverP_node = findPCB(pid);
    PCB *receiverP = (PCB *)(receiverP_node->pItem);

    // append message to receiverP

    if (List_append(receiverP->proc_message, create_msg) == -1)
    {
        printf("Error: Failed to append message to list\n");
        free(create_msg); // Free the allocated memory
        return 0;
    }

    // block P
    runningP->state = BLOCKED;

    printf("Sucess send message to PID %d with msg: %s\n", pid, msg);
    printf("senderID: %d\n", create_msg->senderPid);

    return 1;
};

int receive()
{
    // block P

    PCB *receiveP = runningP;
    receiveP->state = BLOCKED;

    // dequeye message from list of messages, check if has message or not
    message *receive_msg = NULL;

    receive_msg = List_trim(runningP->proc_message);
    if (receive_msg == NULL)
    {
        printf("Error: cannot get the receive msg");
        return 0;
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
    return 1;
};

int reply(int pid, char *msg)
{

    printf("%s\n", msg);
    Node *sender_node = findPCB(pid);

    if (sender_node == NULL)
    {
        printf("Error: cannot get the Sender PCB\n");
        return 0;
    }
    else
    {
        ((PCB *)sender_node->pItem)->state = READY;
    }
    return 1;
};

int new_sem(int semID, int value)
{
    if (value < 0 || value > 4)
    {
        printf("Error: value is not in range");
        return 0;
    }

    sem *newSem = (sem *)malloc(sizeof(sem));
    if (newSem == NULL)
    {
        printf("Error: failure create SEM");
        return 0;
    }

    newSem->semID = semID;
    newSem->value = value;
    newSem->pList = List_create();
    if (newSem->pList == NULL)
    {
        printf("Error: failure create Process List for SEM");
        free(newSem);
        return 0;
    }

    return 1;
};

int P(int semID)
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
        if (!List_append(getSem->pList, runningP))
        {
            printf("Error: Failed to add process to semaphore's process list\n");
            return false;
        }

        // Suspend the running process
        runningP->state = BLOCKED;
    }
    return true;

}; //(semID range 0-4)

int V(int semID)
{
    if (semID < 0 || semID > 4)
    {
        printf("Error: Semaphore ID out of range\n");
        return 0;
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
            PCB *waitingProcess = (PCB *)List_remove(getSem->pList);

            waitingProcess->state = READY;
        }
    }

    return 1; // Operation succeeded
}

void proc_info(int pid)
{
    printf("here");
    Node *PCB_node = findPCB(pid);

    if (PCB_node == NULL)
    {
        printf("Error: cannot get the Sender Node\n");
    }
    else
    {
        PCB *process = (PCB *)PCB_node->pItem;
        printf("The pid of PCB is %d\n", process->pid);
        printf("The priority of PCB is %d\n", process->priority);
        printf("The state of PCB is %d\n", process->state);
    }
    return;
};

void total_info(void)
{
    // Define an array of pointers to the priority queues
    List *priorityQueues[3] = {lowPriority, mediumPriority, highPriority};

    // Loop through each priority queue
    for (int i = 0; i < 3; i++)
    {
        List *priorityQueue = priorityQueues[i];
        // Check if the priority queue is not empty
        if (List_count(priorityQueue) > 0)
        {
            Node *node_ptr = List_first(priorityQueue);

            // Loop through each node in the priority queue
            while (node_ptr != NULL)
            {
                PCB *PCB_ptr = (PCB *)node_ptr->pItem;
                total_info_helper(PCB_ptr);
                node_ptr = node_ptr->pNext;
            }
        }
    }
    return;
}

void Init()
{
    // Initialis priority queues
    highPriority = List_create();
    if (highPriority == NULL)
    {
        printf("Error: cannot create highPriority\n");
    }

    mediumPriority = List_create();
    if (mediumPriority == NULL)
    {
        printf("Error: cannot create mediumPriority\n");
    }

    lowPriority = List_create();
    if (lowPriority == NULL)
    {
        printf("Error: cannot create lowPriority\n");
    }

    // Initialize semList
    for (int i = 0; i < 5; i++)
    {
        // Allocate memory space for each semaphore
        sem *sem_allocate = (sem *)malloc(sizeof(sem));
        if (sem_allocate == NULL)
        {
            printf("Error: cannot allocate memory for sem\n");
            // Handle error, return or exit
        }

        // Assign unique semID values
        sem_allocate->semID = i;

        // Initialize the semaphore value to 0
        sem_allocate->value = 0;

        // Initialize the semaphore list to NULL
        sem_allocate->pList = NULL;

        // Assign the semaphore to the semList
        semList[i] = *sem_allocate;
    }

    // Initialize initP
    // initP = NULL; // Assuming initP is a global variable declared elsewhere
}

int main(int argc, char *argv[])
{
    Init();
//    displayMenu();
    processCommand();

    return 0;
}
