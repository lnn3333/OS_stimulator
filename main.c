#include "main.h"
#include <stdlib.h>
#include <string.h>

// Define helper function to find an available PID
int findPID()
{
    static int availablePID = 0;
    return availablePID++;
};

// comparator function
bool pComparator(void *pItem, void *pComparisonArg)
{
    const Node *Node_pItem = (Node *)pItem;
    PCB *PCB_pItem = Node_pItem->pItem;
    int PID_pItem = PCB_pItem->pid;
    int *intptr_pComparisonArg = pComparisonArg;
    int int_pComparisonArg = *intptr_pComparisonArg;

    return (PID_pItem == int_pComparisonArg);
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

// search the PCB base on pid and return the pointer to the PCB; return NULL if not found
PCB *findPCB(int pid)
{
    PCB *PCBSender;
    void *ptr_pid = &pid;
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
        return 0;
    }

    return PCBSender;
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
void processCommand(char* command, int param1, int param2)
{
    switch (command[0]) // accessing the first character of the command string
    {
    case 'C':
        printf("Creating a new process...\n");
        // Add code to create a process here
        if (createProcess(param1))
        {
            printf("Error: cannot create Process\n");
        }
        break;
    case 'F':
        printf("Forking a process...\n");
        // Add code to fork a process here
        if (forkProcess())
        {
            printf("Error: cannot fork Process\n");
        }
        break;
    case 'K':
        printf("Killing a process...\n");
        // Add code to kill a process here
        if (kill(param1))
        {
            printf("Error: cannot kill Process PID %d\n", param1);
        }
        break;
    case 'E':
        // No action needed here, handled in main loop
        printf("Exit the stimulation\n");
        exitProcess();
        break;
    default:
        printf("Invalid command! Please enter a valid command.\n");
        break;
    }
}

int createProcess(int priority){
    PCB *newPCB = (PCB*) malloc(sizeof(PCB));
    if (newPCB == NULL){
        printf("Error: cannot create newPCB\n");
        free(newPCB);
        return 0;
    }
    
    newPCB->pid = findPID();
    newPCB->priority = priority;
    newPCB->state = READY;
    newPCB->proc_message = List_create();
    if (newPCB->proc_message == NULL){
        printf("Error: cannot create list for PCB\n");
        free(newPCB);
        return 0;
    }
    
    printf("Sucess create PCB \n");
    proc_info(newPCB->pid);
    return newPCB->pid;

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
    memcpy(runningPCopy, runningPCopy, sizeof(PCB));
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
    // Search and remove from priority queues only
    void *ptr_pid = &pid;
    if (List_search(lowPriority, pComparator, ptr_pid) != NULL)
    {
        if (searchAndKill(lowPriority, pid))
        {
            printf("Error: cannot kill PID %d\n", pid);
        };
    }
    else if (List_search(mediumPriority, pComparator, ptr_pid) != NULL)
    {
        if (searchAndKill(mediumPriority, pid ))
        {
            printf("Error: cannot kill PID %d\n", pid);
        };
    }
    else if (List_search(lowPriority, pComparator, ptr_pid) != NULL)
    {
        if (searchAndKill(lowPriority, pid))
        {
            printf("Error: cannot kill PID %d\n", pid);
        };
    }
    else if (List_search(waitForReceiveQueue, pComparator, ptr_pid) != NULL)
    {
        if (searchAndKill(waitForReceiveQueue, pid))
        {
            printf("Error: cannot kill PID %d\n", pid);
        };
    }
    else if (List_search(waitForReplyQueue, pComparator, ptr_pid) != NULL)
    {
        if (searchAndKill(waitForReplyQueue, pid))
        {
            printf("Error: cannot kill PID %d\n", pid);
        };
    }
    else
    {
        printf("Error: Process with PID %d does not exist\n", pid);
        return false;
    }

    printf("Sucess kill PID %d", pid);

    return true;
};

void exitProcess()
{
    // check if all the ready queues are empy
    // If yes, kill the init P
    if (List_count(highPriority) == 0 && List_count(mediumPriority) == 0 && List_count(lowPriority) == 0)
    {
        // kill the init P
        // return ;
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
        return ;
    }
    PCB *next_runningP = next_runningP_node->pItem;
    runningP = next_runningP;
    total_info_helper(runningP); // print info of next running P

    return ;
};

int quantum();

int send(int pid, char *msg)
{
    // add msg to list messages
    message *new_msg = (message *)malloc(sizeof(message));
    if (new_msg == NULL)
    {
        printf("Error: Memory allocation failed for message\n");
        return false;
    }

    // populate new_msg
    new_msg->senderPid = runningP->pid;
    strncpy(new_msg->msg, msg, MAX_MSG_LENGTH);
    new_msg->msg[MAX_MSG_LENGTH] = '\0';

    // find pid of receiverP
    Node *nodeReceiver;
    PCB *PCBReceiver;
    if (List_search(lowPriority, pComparator, &pid) != NULL)
    {
        nodeReceiver = List_curr(lowPriority);
    }
    else if (List_search(mediumPriority, pComparator, &pid) != NULL)
    {
        nodeReceiver = List_curr(mediumPriority);
    }
    else if (List_search(highPriority, pComparator, &pid) != NULL)
    {
        nodeReceiver = List_curr(highPriority);
    }
    else
    {
        printf("Error: Process with PID %d does not exist\n", pid);
        return false;
    }

    // append message to receiverP
    PCBReceiver = nodeReceiver->pItem;
    if (List_append(PCBReceiver->proc_message, new_msg) == -1)
    {
        printf("Error: Failed to append message to list\n");
        free(new_msg); // Free the allocated memory
        return 0;
    }

    // remove sender from ready Q
    int sender_priority = runningP->priority;
    Node *senderNode;
    switch (sender_priority)
    {
    case 0:
        senderNode = List_remove(lowPriority);
        if (senderNode == NULL)
        {
            printf("Error: removing senderNode from ready Queue");
            return 0;
        }
        break;
    case 1:
        senderNode = List_remove(mediumPriority);
        if (senderNode == NULL)
        {
            printf("Error: removing senderNode from ready Queue");
            return 0;
        }
        break;
    case 2:
        senderNode = List_remove(highPriority);
        if (senderNode == NULL)
        {
            printf("Error: removing senderNode from ready Queue");
            return 0;
        }
        break;

    default:
        printf("Error: inappropriate sender_priority");
        return 0;
        break;
    }
    // append to the waitForReplyQueue
    List_append(waitForReplyQueue, senderNode);
    // block P
    runningP->state = BLOCKED;

    printf("Sucess send message to PID %d with msg: %s\n",pid,new_msg->msg );
    printf("senderID: %d\n",new_msg->senderPid);

    return 1;
};

int receive()
{
    // block P
    runningP->state = BLOCKED;
    // remove from ready Queue
    int receiver_priority = runningP->priority;
    Node *receiveNode;
    switch (receiver_priority)
    {
    case 0:
        receiveNode = List_remove(lowPriority);
        if (receiveNode == NULL)
        {
            printf("Error: removing receiveNode from ready Queue");
            return 0;
        }
        break;
    case 1:
        receiveNode = List_remove(mediumPriority);
        if (receiveNode == NULL)
        {
            printf("Error: removing receiveNode from ready Queue");
            return 0;
        }
        break;
    case 2:
        receiveNode = List_remove(highPriority);
        if (receiveNode == NULL)
        {
            printf("Error: removing receiveNode from ready Queue");
            return 0;
        }
        break;

    default:
        printf("Error: inappropriate sender_priority");
        return 0;
        break;
    }
    // add to waitForReceiveQueue
    List_append(waitForReceiveQueue, receiveNode);

    // dequeye message from list of messages, check if has message or not
    message *receive_msg;
    PCB *receivePCB = receiveNode->pItem;

    receive_msg = List_trim(receivePCB->proc_message);
    if (receive_msg == NULL)
    {
        printf("Error: cannot get the receive msg");
        return 0;
    }
    else
    {
        // remove from waitForReceiveQueue
        receiveNode = List_remove(waitForReceiveQueue);
        receivePCB = receiveNode->pItem;
        // append to the appropriate queue
        int receive_priority = receivePCB->priority;
        switch (receive_priority)
        {
        case 0:
            if (List_append(lowPriority, receiveNode))
            {
                printf("Error: cannot append to readyQ\n");
                return 0;
            };
            break;
        case 1:
            if (List_append(mediumPriority, receiveNode))
            {
                printf("Error: cannot append to readyQ\n");
                return 0;
            };
            break;
        case 2:
            if (List_append(highPriority, receiveNode))
            {
                printf("Error: cannot append to readyQ\n");
                return 0;
            };
            break;

        default:
            printf("Error: inappropriate receive_priority");
            return 0;
            break;
        }


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
    receivePCB->state = READY;
    return 1;
};

int reply(int pid, char *msg)
{

    printf("%s\n", msg);
    PCB *getPCBSender = NULL;
    getPCBSender = findPCB(pid);

    if (getPCBSender == NULL)
    {
        printf("Error: cannot get the Sender PCB\n");
        return 0;
    }
    else
    {
        getPCBSender->state = READY;
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

    PCB *getPCBSender = findPCB(pid);
printf("here");
    if (getPCBSender == NULL)
    {
        printf("Error: cannot get the Sender PCB\n");
    }
    else
    {
        printf("The pid of PCB is %d\n", getPCBSender->pid);
        printf("The priority of PCB is %d\n", getPCBSender->priority);
        printf("The state of PCB is %d\n", getPCBSender->state);
    }
    return ;
};

void total_info(void)
{
    // loop through all queues
    // call procinfo

    Node *node_ptr = List_first(lowPriority);
    PCB *PCB_ptr = node_ptr->pItem;

    while (node_ptr->pNext != NULL)
    {
        PCB_ptr = node_ptr->pItem;
        total_info_helper(PCB_ptr);
        node_ptr = node_ptr->pNext;
    };

    node_ptr = List_first(mediumPriority);

    while (node_ptr->pNext != NULL)
    {
        PCB_ptr = node_ptr->pItem;
        total_info_helper(PCB_ptr);
        node_ptr = node_ptr->pNext;
    };

    node_ptr = List_first(highPriority);

    while (node_ptr->pNext != NULL)
    {
        PCB_ptr = node_ptr->pItem;
        total_info_helper(PCB_ptr);
        node_ptr = node_ptr->pNext;
    };

    node_ptr = List_first(waitForReceiveQueue);

    while (node_ptr->pNext != NULL)
    {
        PCB_ptr = node_ptr->pItem;
        total_info_helper(PCB_ptr);
        node_ptr = node_ptr->pNext;
    };

    node_ptr = List_first(waitForReplyQueue);
    while (node_ptr->pNext != NULL)
    {
        PCB_ptr = node_ptr->pItem;
        total_info_helper(PCB_ptr);
        node_ptr = node_ptr->pNext;
    };
}

void Init()
{

    // initialis priority queues
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

    // Queue wait for receive msg
    waitForReceiveQueue = List_create();
    if (waitForReceiveQueue == NULL)
    {
        printf("Error: cannot create waitForReceiveQueue\n");
    };
    // Queue wait for reply msg
    waitForReplyQueue = List_create();
    if (waitForReplyQueue == NULL)
    {
        printf("Error: cannot create waitForReplyQueue\n");
    };

    // init semList
    for (int i = 0; i < 5; i++)
    {
        // allocate memory space for each semaphore
        sem *sem_allocate = (sem *)malloc(sizeof(sem));
        if (sem_allocate == NULL)
        {
            printf("Error: cannot allocate memory for sem\n");
        };

        sem_allocate->value = 0;
        sem_allocate->semID = -1;
        sem_allocate->pList = NULL;
    };

    // initP = (PCB*) malloc(sizeof(PCB));
    // if (initP == NULL){
    //     printf("Error: cannot allocate memory for initP\n");
    // };
};

int main(int argc, char *argv[]) {
    if (argc < 2) {
        printf("Insufficient arguments provided.\n");
        return 1; // indicate error
    }

    displayMenu();

    char *command = argv[1]; // Corrected to start from argv[1]
    int p2 = (argc > 2) ? atoi(argv[2]) : 0;
    int p3 = (argc > 3) ? atoi(argv[3]) : 0;

    printf("Arguments received are: %s, %d, %d\n", command, p2, p3);
    processCommand(command, p2, p3); // Passing command, not argv[0]

    return 0; // indicate success
}

