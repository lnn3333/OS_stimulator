#include "main.h"
#include <unistd.h>

// Define the variables
List *lowPriority = NULL;
List *mediumPriority = NULL;
List *highPriority = NULL;
List *blockQ = NULL;

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
    add_to_priority(priority,new_process);
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
        List_append(highPriority, runningPCopy);
        break;
    case (1):
        List_append(mediumPriority, runningPCopy);
        break;
    case (2):
        List_append(lowPriority, runningPCopy);
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
    if (PCB_p->state == BLOCKED)
    {
        if (List_remove(blockQ) == NULL)
        {
            printf("Error removing from queue \n");
            return false;
        }
    }
    else
    {
        int find_pid_priority = PCB_p->pid;
        switch (find_pid_priority)
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
            printf("Error removing from queue \n");
            return 0;

            break;
        }
    }
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
    bool res = cpu_scheduler();
    if (res == false)
    {
        printf("Error from cpu scheduling\n");
        return;
    }
    total_info_helper(runningP); // print info of next running P

    return;
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

int send(int pid, char *msg)
{
    // allocate message package
    message *create_msg = allocate_message(msg);
    if (create_msg == NULL)
    {
        printf("Error create new message\n");
        return 0;
    }

    // find pid of receiverP
    Node *receiverP_node = findPCB(pid);
    if (receiverP_node == NULL){
        printf("Error: receiver ID is invalid\n");
        return 0;
    }
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
    PCB *blockP = runningP;

    // add block P to blockQ
    if (List_append(blockQ, blockP) == -1)
    {
        printf("Error: Failed to append blockP to list\n");
        return 0;
    }

    bool res = cpu_scheduler();
    if (res == false)
    {
        printf("Error from cpu scheduling\n");
        return 0;
    }
    printf("Sucess send message to PID %d with msg: %s\n", pid, msg);
    printf("senderID: %d\n", create_msg->senderPid);

    return 1;
};

int receive()
{
    // block P

    PCB *receiveP = runningP;
    receiveP->state = BLOCKED;

    // getRunningP();

    // dequeye message from list of messages, check if has message or not
    message *receive_msg = List_trim(runningP->proc_message);

    if (receive_msg == NULL)
    {
        printf("Error: cannot get the receive msg");
        // add to blockQ
        PCB *blockP = runningP;
        if (List_append(blockQ, blockP) == -1)
        {
            printf("Error: Failed to append blockP to list\n");
            return 0;
        }

        // get the next runningP
        bool res = cpu_scheduler();
        if (res == false)
        {
            printf("Error from cpu scheduling\n");
        }
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

    if (sender_node->pItem == NULL)
    {
        printf("Error: cannot get the Sender PCB\n");
        return 0;
    }
    else
    {
        PCB *PCB_sender = (PCB *)sender_node->pItem;
        PCB_sender->state = READY;
        int P_priority = PCB_sender->priority;

        if (add_to_priority(P_priority, PCB_sender) == false)
        {
            return 0;
        };
    }
    return 1;
};

int new_sem(int semID)
{
    if (semID < 0 || semID > 4)
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
    newSem->value = 1;
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

        PCB *blockP = runningP;
        // schedule the next runningP
        int res = quantum();
        if (res == 0)
        {
            return false;
        }

        blockP->state = BLOCKED;

        if (!List_append(getSem->pList, blockP))
        {
            printf("Error: Failed to add process to semaphore's process list\n");
            return false;
        }
    }
    printf("Sucess Block the process on sem ID %d\n", semID);
    return true;

}; //(semID range 0-4)

int V(int semID)
{
    PCB *waitingProcess;
    int waitingP_priority = -1;
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
            List_first(getSem->pList);
            waitingProcess = (PCB *)List_remove(getSem->pList);
            waitingP_priority = waitingProcess->priority;
            waitingProcess->state = READY;
        }
    }

    if ( add_to_priority(waitingP_priority, waitingProcess) == false)
    {
        return 0;
    };

    return 1; // Operation succeeded
}

void proc_info(int pid)
{
    Node *PCB_node = findPCB(pid);

    if (PCB_node == NULL)
    {
        printf("Error: Cannot find PCB with PID %d\n", pid);
        return; // Exit early if PCB not found
    }

    PCB *process = (PCB *)PCB_node->pItem;
    if (process == NULL)
    {
        printf("Error: PCB pointer is NULL for PID %d\n", pid);
        return; // Return early to avoid dereferencing NULL pointer
    }
    printf("The pid of PCB is %d\n", process->pid);
    printf("The priority of PCB is %d\n", process->priority);
    printf("The state of PCB is %d\n", process->state); // Consider changing format specifier if necessary
}

void total_info(void)
{
    // Define an array of pointers to the priority queues
    List *priorityQueues[3] = {highPriority, mediumPriority, lowPriority};

    // Loop through each priority queue
    for (int i = 0; i < 3; i++)
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
    initP = allocateProcess(3);
    runningP = initP;

    // initP = NULL; // Assuming initP is a global variable declared elsewhere
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

    if(runningP != initP){
        PCB *resumeP = runningP;
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
    runningP = nextP;
    nextP->state = RUNNING;
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
