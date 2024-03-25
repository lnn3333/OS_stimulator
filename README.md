I. The Structure of the stimulator (in file structure.h)
1. Constants: Two constants are defined:

SEMAPHORE_NUMBER: Indicates the number of semaphores.
MAX_MSG_LENGTH: Maximum length of a message.
Process State Enumeration: An enumeration named processState is defined to represent the state of a process. Possible states are READY, RUNNING, BLOCKED, and DEADLOCK.

2. Structure
Message Structure: A structure named message is defined to represent a message. It contains the sender's process ID and the message content.

Process Control Block (PCB) Structure: A structure named PCB is defined to represent the Process Control Block. It contains process ID, priority, state, and a pointer to a message.

Semaphore Structure: is represented by a structure named sem. It contains semaphore ID, value, and a pointer to a list.

3. Lists Declarations are external, meaning they are defined elsewhere.
- 3 list for different priorities (lowPriority, mediumPriority, highPriority)
- 1 block queue (blockQ) store the process block by command send(), receive()
- An array store 5 semaphore (semList)
    
4. Process Declarations: 
Pointers to the currently running process (runningP) and the initial process (initP) are declared as external.

II. Helper functions (in file help.h)

unction Declarations:

int findPID();
Finds the next available process ID.

PCB* findPCB(int pid);
Searches for a PCB based on a given process ID and returns a pointer to the PCB if found, otherwise returns NULL.

void total_info_helper(PCB *pcb);
Displays detailed information about a given PCB pointer.

message* allocate_message(char *msg);
Allocates memory for a message and sets its content base on parameter msg.

PCB* allocateProcess(int priority);
Allocates memory for a new process with the specified priority base on parameter.


bool add_to_priority(int priority, PCB* item);
Adds a process to the appropriate priority queue based on its priority level.

bool remove_from_queue(int pid);
Removes a process from the queue based on its process ID.

const char *getStateName(enum processState);
Returns a string representation of a given process state.

const char *getPriorityName(int priority);
Returns a string representation of a given priority level.

const char *getQueueName(int n);
Returns a string representation of a given queue.

III. Functions Implement for assignment (in main.h)
1. Global Variables:

Declarations of global variables such as lists for different process priorities (lowPriority, mediumPriority, highPriority), a block queue (blockQ), pointers to the currently running process (runningP) and the initial process (initP), and an array of semaphores (semList).

2. Function Declarations:
i. Create Process (createProcess):
Creates a new process with the specified priority and adds it to the ready queue.

Arguments:
int priority: Priority level of the process (0 = high, 1 = normal, 2 = low).

Returns:
bool: Indicates whether the creation was successful.

ii. Fork Process (forkProcess):

Copies the currently running process and adds the copy to the ready queue.
Then print the info of the copy process
Note: Attempting to fork the initial process will fail.

Returns:
bool: Indicates whether the forking was successful.

iii. Kill Process (kill):

Terminates a process and removes it from the system.
If suceed kill(), get the next runningP (by calling quantum())
Note: attempt to kill the initP will fail

Arguments:
int pid: Process ID of the process to be killed.

Returns:
bool: Indicates whether the termination was successful.

iv. Exit Process (exitProcess):

Terminates the currently running process.
Note: allow to terminate initP. If initP is terminated, exit the stimualation.
Reports process scheduling information, including which process now gets control of the CPU.

v. Quantum Function:
Helper Function:
CPU Scheduler (cpu_scheduler):
Implements the CPU scheduling algorithm using Round Robin.
Moves the currently running process to the lowest priority queue (lowPriority).
Finds the next process to run.(the first item in the highest non-empty priority queue)

Returns:
bool: Indicates whether the scheduling was successful.

Quantum (quantum):
Represents a time quantum of the running process expiring.
Triggers the CPU scheduler to determine the next process to run.
Reports the action taken, such as process scheduling information.

vi. send(int pid, char *msg)
Send a message from the currently running process to another process identified by its PID (pid).
It performs the following steps:

Input Validation: Checks if there is a currently running process and if the specified PID is not the same as the PID of the running process and PID exist.

Receiver Validation: Find the receiver process identified by the provided PID. If the receiver's message box is not empty, the message box is now full, waiting for (receive() command), the sending operation is canceled.

Message Allocation: Allocates memory for the message to be sent and attaches it to the receiver process's message box.

Blocking the Sender: If the sender process is not the initial process, it is marked as blocked, and it is added to the block queue (blockQ).

Get the next process is scheduled to run.

Success Notification: Prints a success message indicating that the message has been sent successfully.

vii. receive()
This function is responsible for receiving a message by the currently running process. 

It performs the following steps:

Running Process Validation: Checks if there is a currently running process.

Message Reception: If the running process's message box is empty, indicating there are no messages in the message box, the process is blocked, added to the block queue (blockQ), and the next process is scheduled to run.

Message Display: If there is a message in the process's message box, it displays the received message.

Reply to Sender: Prepares and sends a reply message to the sender of the received message.

Message Box Cleanup: Empties the message box of the running process by freeing the allocated memory.

viii. reply(int pid, char *msg)
This function is responsible for replying to a previously received message. 

It performs the following steps:

Input Validation: Checks if the reply message is not NULL and searches for the PCB of the sender process in the block queue (blockQ) based on the provided PID.
Note: cannot reply to process that is not on the block queue

Message Allocation: Allocates memory for the reply message and attaches it to the sender process.

Unblocking the Sender: Sets the sender process to a ready state and removes it from the block queue.

Success Notification: Prints a success message indicating that the reply message has been sent successfully.
 

ix. new_sem(int semID)
This function is responsible for creating a new semaphore with the specified semaphore ID.

It performs the following steps:

Semaphor ID Validation: Checks if a semaphore with the provided ID has already been created.(ID range from 0 - 4)

Semaphore Initialization: Initializes the semaphore's value to 1 and creates a process list for the semaphore.

Success Notification: Prints a success message indicating that the semaphore has been successfully created.

x. P(int semID)
This function performs the "P" operation on the semaphore identified by semID.

It performs the following steps:

Semahore ID validation

PCB validation: check if the runnningP is initP, cannot blocked.

Operating process:
It decrements the semaphore's value and blocks the process (add to the semaphore block list) if the semaphore value becomes negative.
Get the next runningP (by calling quantum())

xi. V(int semID)
This function performs the "V" operation on the semaphore identified by semID.

It performs the following steps:

Semahore ID validation

Operating process:
It increments the semaphore's value and unblocks a waiting process if the semaphore value is non-negative.
Add the wake up process to the ready queue base on its priority

xii. proc_info(int pid)
This function displays information about the process identified by pid, including its priority, state, and other relevant details.

xiii. total_info()
This function provides comprehensive information about all processes and semaphores in the system. It displays details such as priority queues, blocked processes, and the currently running process. (using helper function: total_info_helper )



3. Initialization Function:

void Init(): Initializes the lists and semaphores required for the simulation and sets up the initial process.

4. User Interface Functions:
void menu(): Prints a menu for user interaction, providing options for various simulation commands.
void processCommand(): Processes user commands, executing the corresponding functions based on user input.