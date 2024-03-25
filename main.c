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

bool createProcess(int priority)
{

    if ((initP != NULL && priority == 3))
    {
        printf("Error: invalid priority\n");
        return false;
    }

    // allocate memory of Process
    PCB *new_process = allocateProcess(priority);
    if (new_process == NULL)
    {
        printf("Failed to create process.\n");
        return false;
    }

    // add to ready Q
    add_to_priority(priority, new_process);
    printf("Sucess create process PID %d\n", new_process->pid);
    proc_info(new_process->pid);
    return true;
}

bool forkProcess()
{
    // check if Process is init
    if (runningP->pid == initP->pid)
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
    runningPCopy->state = READY;

    // add to readyQ
    if (add_to_priority(runningPCopy->priority, runningPCopy) == false)
    {
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

    if (runningP != NULL && pid == runningP->pid)
    {
        free(runningP);
        runningP = NULL;
        // get the next runnning P
        quantum();
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
        exit(0);
        return;
    }
    else
    {
        if (runningP != NULL)
        {
            if (!kill(runningP->pid))
            {
                return;
            }
        }
    }
};

bool cpu_scheduler()
{
    // append the running P to the low priority queue

    if (runningP != NULL && runningP->pid != initP->pid && runningP->state != BLOCKED)
    {
        PCB *resumeP = runningP;
        resumeP->priority = 2;
        resumeP->state = READY;
        if (add_to_priority(2, resumeP) == false)
        {
            return false;
        }
    }
    else if (runningP != NULL && runningP->pid == initP->pid)
    {
        initP->state = READY;
        runningP = NULL;
    }
    // find the next running P
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

    if (nextP == NULL)
    {
        printf("Error: getting next P\n");
        return false;
    }
    runningP = nextP;
    runningP->state = RUNNING;
    // check if there is any reply message in proc_message to be display
    if (runningP->proc_message != NULL)
    {
        if (runningP->proc_message->senderPid == -1)
        {
            char *reply_msg = runningP->proc_message->msg;
            printf("The reply message: %s\n", reply_msg);
            // free the mailbox
            free(runningP->proc_message);
        }
    }

    return true;
};

void quantum()
{
    bool res = cpu_scheduler();
    if (res == false)
    {
        printf("Error from cpu scheduling\n");
        return;
    }
    total_info_helper(runningP);
    return;
};

bool send(int pid, char *msg)
{
    if (runningP == NULL)
    {
        printf("Error: there is no running P\n");
        return false;
    }

    if (pid == runningP->pid)
    {
        printf("Error: cannot send message to sender\n");
        return false;
    }

    // find pid of receiverP
    PCB *receiverP = (PCB *)findPCB(pid);
    if (receiverP == NULL)
    {
        printf("Error: receiver ID is invalid\n");
        return false;
    }
    // check if the receiver has a message sent by other process that has not yet receive on them already
    // if yes then cancel send operation, else send the message
    if (receiverP->proc_message != NULL)
    {
        printf("Error: The receiver id cannot receive anymore message\n");
        return false;
    }
    // allocate message package
    if ((receiverP->proc_message = allocate_message(msg)) == NULL)
    {
        return false;
    }

    // block P
    if (runningP->pid != initP->pid)
    {
        runningP->state = BLOCKED;
        // add block P to blockQ
        if (List_append(blockQ, runningP) == LIST_FAIL)
        {
            printf("Error: Failed to append blockP to list\n");
            return false;
        }
        printf("Block Process senderID : %d\n", receiverP->proc_message->senderPid);
        // get the next running process
        printf("The next running process is:\n");
        quantum();
    }

    printf("Success send message to PID %d with msg: %s\n", pid, msg);

    return true;
};

bool receive()
{
    if (runningP == NULL)
    {
        printf("Error: there is no running Process\n");
        return false;
    }
    // dequeye message from list of messages, check if has message or not

    PCB *receiveP = runningP;

    if (receiveP->proc_message == NULL)
    {
        printf("Error: cannot get the receive msg");
        // add to blockQ
        if (receiveP->pid != initP->pid)
        {
            receiveP->state = BLOCKED;
            if (List_append(blockQ, receiveP) == LIST_FAIL)
            {
                printf("Error: Failed to append blockP to list\n");
                return false;
            }
        }
        printf("Block Process PID : %d\n", receiveP->pid);
        // get the next running process
        printf("The next running process is:\n");
        quantum();
        return false;
    }
    // print the receive message

    char *receive_msg = receiveP->proc_message->msg;
    printf("The receive message: %s\n", receive_msg);

    // get sender id
    int senderID = receiveP->proc_message->senderPid;
    // reply to sender
    char *reply_msg = "Reply message\n";
    if (reply(senderID, reply_msg) == 0)
    {
        printf("Error: cannot reply to sender with pid %d \n", senderID);
        return 0;
    }
    // empty the message box (proc_msg)
    free(receiveP->proc_message);

    return true;
};

bool reply(int pid, char *msg)
{
    if (msg == NULL)
    {
        printf("Error: message reply is NULL\n");
        return false;
    }
    void *ptr_pid = &pid;
    PCB *PCB_sender = (PCB *)List_search(blockQ, pComparator, ptr_pid);
    if (PCB_sender == NULL)
    {
        printf("Error: invalid PID or the Process PID has not been blocked by Send\n");
        return false;
    }

    // append message to the reply Process
    if ((PCB_sender->proc_message = allocate_message(msg)) == NULL)
    {
        return false;
    }
    strcpy(PCB_sender->proc_message->msg, msg);
    PCB_sender->proc_message->senderPid = -1;

    // unblock the sender
    PCB_sender->state = READY;
    List_remove(blockQ);

    // add the unblock P back to ready Q
    if (add_to_priority(PCB_sender->priority, PCB_sender) == false)
    {
        return false;
    };
    printf("Unblock sender ID %d \n", PCB_sender->pid);

    return true;
};

bool new_sem(int semID)
{

    if (semList[semID].pList != NULL)
    {
        printf("Error: Semaphore %d has already been created\n", semID);
        return false;
    }

    sem *semItem = &semList[semID];
    semItem->value = 1;
    semItem->pList = List_create();
    if (semItem->pList == NULL)
    {
        printf("Error: Failed to create process list for Semaphore %d\n", semID);
        return false;
    }

    return true;
}

bool P(int semID)
{
    // check if semID value is valid
    if (semID < 0 || semID > 4)
    {
        printf("Error: Semaphore ID is out of range\n");
        return false;
    }

    // check if the sem has been created yet

    sem *getSem = &semList[semID];

    if (getSem->pList == NULL)
    {
        printf("Error: the Semaphore has not been created\n");
        return false;
    }
    // check if there is running process (so we can perfore P on it)
    if (runningP == NULL)
    {
        printf("Error: There is no current running Process\n");
        return false;
    }
    // check if the runningP is initP, if yes, we can not block the runningP
    if (runningP == initP)
    {
        printf("Error: Cannot blocked init Process\n");
        return false;
    }
    // P operation perform

    getSem->value--;
    // If semaphore value is negative, block the process
    printf("Semaphore value is %d\n", getSem->value);
    if (getSem->value < 0)
    {
        // Add runningP to the semaphore's process list
        PCB *blockP = runningP;
        if (List_append(getSem->pList, blockP) == LIST_FAIL)
        {
            printf("Error: Failed to add process to semaphore's process list\n");
            return false;
        }
        printf("Success: Process PID %d blocked on semaphore %d\n", blockP->pid, semID);
        blockP->state = BLOCKED;
        // get the next running process
        printf("The next running process is:\n");
        quantum();
    }

    return true;
}

bool V(int semID)
{
    if (semID < 0 || semID > 4)
    {
        printf("Error: Semaphore ID out of range\n");
        return false;
    }
    sem *getSem = &semList[semID];
    if (getSem->pList == NULL)
    {
        printf("Error: the Semaphore has not been created\n");
        return false;
    }

    getSem->value++;
    printf("Value of sem %d\n", getSem->value);
    if (getSem->value <= 0)
    {
        // Check if there are processes waiting on the semaphore
        if (List_count(getSem->pList) > 0)
        {
            // Get the first process waiting on the semaphore
            List_first(getSem->pList);
            PCB *waitingProcess = (PCB *)List_remove(getSem->pList);
            // unblock the process, add back to readyQ
            waitingProcess->state = READY;
            if (add_to_priority(waitingProcess->priority, waitingProcess) == false)
            {
                return false;
            };

            // print info of the wakeup P
            printf("Wake up Process PID %d\n", waitingProcess->pid);
        }
    }

    return true; // Operation succeeded
}

void proc_info(int pid)
{
    PCB *process = (PCB *)findPCB(pid);

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

    List *queues[9] = {highPriority, mediumPriority, lowPriority, blockQ, semList[0].pList, semList[1].pList, semList[2].pList, semList[3].pList, semList[4].pList};

    // Loop through each priority queue
    for (int i = 0; i < 9; i++)
    {

        List *queue = queues[i];
        // Check if the priority queue is not empty
        if (List_count(queue) > 0)
        {
            printf("%s\n", getQueueName(i));
            Node *node_ptr = queue->pFirstNode;

            // Loop through each node in the priority queue
            while (node_ptr != NULL)
            {
                PCB *PCB_ptr = (PCB *)node_ptr->pItem;
                total_info_helper(PCB_ptr);
                node_ptr = node_ptr->pNext;
            }
            printf("\n");
        }
    }
    if (runningP != NULL || runningP == initP)
    {
        printf("The running process: \n");
        total_info_helper(runningP);
    }

    return;
}

void Init()
{
    // Initialis  queues: 3 ready queues, 1 block queue,
    lowPriority = List_create();
    if (lowPriority == NULL)
    {
        printf("Error: cannot create lowPriority\n");
        return;
    }

    mediumPriority = List_create();
    if (mediumPriority == NULL)
    {
        printf("Error: cannot create mediumPriority\n");
        return;
    }

    highPriority = List_create();
    if (highPriority == NULL)
    {
        printf("Error: cannot create highP\n");
        return;
    }

    blockQ = List_create();
    if (blockQ == NULL)
    {
        printf("Error: cannot create highP\n");
        return;
    }

    // Initialize List for the semaphore
    semList = (sem *)malloc(sizeof(sem) * 5);
    if (semList == NULL)
    {
        printf("Error: Failed to allocate memory for semList\n");
        return;
    }

    for (int i = 0; i < 5; i++)
    {
        semList[i].pList = NULL;
    }

    // Initialize initP
    initP = allocateProcess(3);
    runningP = initP;
    runningP->state = RUNNING;
}
void menu()
{
    // print the menu for user
    printf("\nSimulation Menu:\n");
    printf("Enter command and parameter: \n");
    printf("C <priority>: Create a process\n");
    printf("F: Fork a process\n");
    printf("K <pid>: Kill a process\n");
    printf("E: Exit the simulation\n");
    printf("Q: Quantum the simulation\n");
    printf("S <id> <message>: Send message to Process pid\n");
    printf("R: Receive message\n");
    printf("Y <id> <message>: Reply message to Process pid\n");
    printf("N <id>: Create new SEM\n");
    printf("P <id>: Operate sem P operation\n");
    printf("V <id>: Operate sem V operation\n");
    printf("I: Print process info\n");
    printf("T: Print all info\n");
}
// Function to process the user command
void processCommand()
{
    while (1)
    {
        // Get user input (command and parameters)
        printf("\nEnter your command: ");
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

        scanf(" %c", &command);

        // Process user input
        switch (toupper(command))
        {
            //// Process cmd from the terminal
        case 'C':
        {
            scanf("%d", &priority);
            int p = priority;
            if (p >= 0 && p <= 3)
            {
                createProcess(priority);
                break;
            }
            else
            {
                printf("Error: invalid priority\n");
                break;
            }
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
            if (id < 0 || id > 4)
            {
                printf("Error: Semaphore ID is out of range\n");
                return;
            }
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
        case 'I':
            scanf("%d", &id);
            proc_info(id);
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

int main(int argc, char *argv[])
{
    Init();
    menu();
    while (1)
    {
        processCommand();
    }

    return 0;
}
