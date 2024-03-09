#include "help.h"


int findPID()
{
    static int availablePID = 0;
    return availablePID++;
};



// search the PCB base on pid and make current item the found item; return -1 if not found; return the priority 
Node* findPCB(int pid)
{
    Node *PCBSender = NULL; // Initialize to NULL to handle cases where PCB is not found

    void *ptr_pid = &pid; // Pointer to the process ID

    // Find sender ID across different priority levels
    if ((PCBSender = List_search(highPriority, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = List_search(mediumPriority, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else if ((PCBSender = List_search(lowPriority, pComparator, ptr_pid)) != NULL)
    {
        return PCBSender;
    }
    else
    {
        fprintf(stderr, "Error: Sender with PID %d does not exist\n", pid);
        return NULL; // Return NULL if PCB is not found
    }
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

void printState(PCB* process){
    printf("State of PID %d is %d:", process->pid, process->state);
}

void getRunningP(){
    printf("PID running is %d\n", runningP->pid);
}