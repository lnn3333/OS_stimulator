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

    if( (initP!=NULL && priority ==  3))
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
    printf("Sucess create process PID %d\n",new_process->pid);
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
        return true;
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

    if(!(pid>=0 && pid <=2))
    {
        printf("Error: Invalid receiver PID\n");
    }

    // allocate message package
    message *create_msg = allocate_message(msg);
    if (create_msg == NULL)
    {
        printf("Error create new message\n");
        return false;
    }

    // find pid of receiverP
    PCB *receiverP = (PCB *)findPCB(pid);
    if (receiverP == NULL)
    {
        printf("Error: receiver ID is invalid\n");
        return false;
    }
    // add message to receiverP
    if (receiverP->proc_message != NULL)
    {
        printf("Error: The receiver id cannot receive anymore message\n");
        return false;   
    }

    if(!(receiverP->proc_message = create_msg)){
        printf("Error: error sending\n");
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
        runningP = NULL;
    }

    printf("Sucess send message to PID %d with msg: %s\n", pid, msg);
    printf("senderID: %d\n", create_msg->senderPid);

    return true;
};

bool receive()
{
    if(runningP == NULL){
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
        runningP = NULL;
        return false;
    }

    // get sender id
    int senderID = receiveP->proc_message->senderPid;
    // reply to sender
    char *reply_msg = "Reply message\n";
    int resultReply = reply(senderID, reply_msg);
    if (resultReply == 0)
    {
        printf("Error: cannot reply to sender with pid %d \n", senderID);
        return 0;
    }
    // unblock the sender
    PCB *PCB_sender = (PCB *)findPCB(senderID);
    if (PCB_sender!=NULL && PCB_sender->state == BLOCKED)
    {
        PCB_sender->state = READY;
        List_remove(blockQ);
        if (add_to_priority(PCB_sender->priority, PCB_sender) == false)
        {
            return false;

        };
        printf("Unblock sender ID %d \n", PCB_sender->pid);
    }

    return true;
};

bool reply(int pid, char *msg)
{
    printf("%s\n", msg);
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
    if (semID < 0 || semID > 4)
    {
        printf("Error: Semaphore ID is out of range\n");
        return false;
    }

    sem *getSem = &semList[semID];

    if (getSem->pList == NULL)
    {
        printf("Error: the Semaphore has not been created\n");
        return false;
    }

    if (runningP == NULL)
    {
        printf("Error: There is no current running Process\n");
        return false;
    }
    if (runningP == initP)
    {
        printf("Error: Cannot blocked init Process\n");
        return false;
    }

    getSem->value--;
    printf("Value of semaphore %d is now %d\n", semID, getSem->value);

    // If semaphore value is negative, block the process
    if (getSem->value < 0)
    {
        // Block the process by adding it to the semaphore's process list
        PCB *blockP = runningP;
        if (List_append(getSem->pList, blockP) == LIST_FAIL)
        {
            printf("Error: Failed to add process to semaphore's process list\n");
            return false;
        }
        printf("Number of processes blocked on semaphore %d: %d\n", semID, List_count(getSem->pList));
        blockP->state = BLOCKED;
    }
    printf("Success: Process blocked on semaphore %d\n", semID);
    return true;
}

bool V(int semID)
{
    PCB *waitingProcess;

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
    printf("value of sem is %d\n", getSem->value);
    printf("value list of sem is %d\n", List_count(getSem->pList));
    if (getSem->value <= 0)
    {
        // Check if there are processes waiting on the semaphore
        if (List_count(getSem->pList) > 0)
        {
            // Get the first process waiting on the semaphore
            List_first(getSem->pList);
            waitingProcess = (PCB *)List_remove(getSem->pList);
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
        printf("\n");
        }
       
    }
    if (runningP != NULL || runningP == initP)
    {
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
    semList = (sem *)malloc(sizeof(sem) * 5);
    if (semList == NULL)
    {
        printf("Error: Failed to allocate memory for semList\n");
        // Handle error appropriately, e.g., return from the function or exit program
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

        printf("Enter command and parameter: \nC <priority>, F, K <pid>, E, Q, S<id><message>,R, Y, N<id>, P<id>, V<id>, I<id>, T: ");
        scanf(" %c", &command);
        // Process user input
        switch (toupper(command))
        {
        case 'C':
        {
            scanf("%d", &priority);
            if(!(priority>=0 && priority <=3)){
                printf("Error: invalid priority\n");
                return  ;
            }
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
            if (id < 0 || id > 4)
            {
                printf("Error: Semaphore ID is out of range\n");
                return ;
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

bool cpu_scheduler()
{
   //append the running P to the low priority queue

    if (runningP != initP && runningP != NULL && runningP->state != BLOCKED)
    {
        PCB *resumeP = runningP;
        resumeP->priority = 2;
        if (List_append(lowPriority, resumeP) == LIST_FAIL)
        {
            printf("Error adding process to low priority \n");
            return false;
        }
    }
    else if (runningP !=NULL && runningP->pid == initP->pid){
        initP->state = READY;
        runningP = NULL;
    }



//find the next running P
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
