#include <stdio.h>
#include <ctype.h>
#include "list.h"
#include <stdbool.h>
#include <unistd.h>
// Define constants for semaphore IDs
#define SEMAPHORE_NUMBER 5
#define MAX_MSG_LENGTH 40

// Define state (0 : ready, 1 : running, 2 : blocked, 3 : deadlock)
enum processState
{
    READY,
    RUNNING,
    BLOCKED,
    DEADLOCK
};

// Process Block Control data structure
typedef struct PCB_s PCB;
struct PCB_s
{
    int pid;
    int priority;
    processState state;
    List* messages;
};

// Semaphore data structure

typedef struct sem_S sem;
struct sem_S
{
    int semID;
    int value;
    PCB *pList;
};

// Define a structure to represent a message
typedef struct message_S message;
struct message_S {
    int pid;
    char msg[MAX_MSG_LENGTH + 1]; // +1 for null terminator
};

// Ready Q based on priority (high 2, medium 1, low 0)
List *highPriority;
List *mediumPriority;
List *lowPriority;

// Queue wait for send msg
List *jobQ;

// Queue wait for receive msg
List *readyQ;

PCB *runningP;
PCB *initP;
sem semList[5];
List *messages;
// Functions
// Create| C: create a P, add to ready Q
// Report Success or Failure, pid of created P
int createProcess(int priority); //(0 = high, 1 = norm, 2 = low)

// Fork| F: copy current running P, add to ready Q
// Report Success or Failure, pid of new P
//  attemp to Fork the init P should fail
bool forkProcess();

// Kill| K: kill P, remove P from the system
// Report Success or Failure
bool kill(int pid);

// Exit| E: kill current running P
//  Reports P scheduling info, which P now get control of CPU
void exit();

// Quantum| Q: time quantum of running P expire
// Report action taken (eg. process scheduling information)
int quantum();

// Send| S: send a msg to another P - block until reply
// Reports Success of Failure, scheduling info, once received the reply: the //msg and source of msg
int send(int pid, char *msg);

// Receive| R: receive a msg - block until one msg arrive
// Reports scheduling info, once received the reply: the msg and source of msg
int receive();

// Reply| Y: unblock sender, deliver the reply
// Report success or failure
int reply(int pid, char *msg); // null terminate string, MAX lenght of msg = 40 char

// New sem| N: init name,value of SEM (0-4) (can do only once, error if attempt again)
// Report: action taken, Success or Failure
int new_sem(int semID, int value);

// sem P |P : execute the semaphore P
// Report action taken (blocked or not), success or failure.
int P(int semID); //(semID range 0-4)

// sem V |V: execute the semaphore V
// Report action taken (whether P is readied), success or failure.
int V(int semID); //(semID range 0-4)

// Procinfo |I: dump complete state info of P (incl all status)
// Report see action
void proc_info(int pid);

// Totalinfo |T: display all process queues and their content
// Report see action
void total_info(void);
