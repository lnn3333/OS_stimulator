#include <stdio.h>
#include <ctype.h>
#include "list.h"
#include <stdbool.h>
#include <unistd.h>

//Define state (0 : ready, 1 : running, 2 : blocked, 3 : deadlock) 
enum processState {
    READY,
    RUNNING,
    BLOCKED,
    DEADLOCK
};

//Define priority level (high 2, medium 1, low 0)
enum processPriority{
    LOW,
    MEDIUM,
    HIGH
};

//Process Block Control data structure
typedef struct PCB_s PCB;
struct PCB_s
{
    int pid;
	processPriority priority; 
	processState state; 
};

//Semaphore data structure

typedef struct sem_S sem;
struct sem_S 
{
    int value;
    PCB *pList;
};

//Ready Q based on priority (high 2, medium 1, low 0)
List_s *highPriority;
List_s *mediumPriority;
List_s *lowPriority;

// Queue wait for send msg
List_s *jobQ;

// Queue wait for receive msg
List_s *readyQ;

//Functions
//Create| C: create a P, add to ready Q 
//Report Success or Failure, pid of created P
int create(int priority); //(0 = high, 1 = norm, 2 = low)

//Fork| F: copy current running P, add to ready Q
//Report Success or Failure, pid of new P
// attemp to Fork the init P should fail
int fork();

//Kill| K: kill P, remove P from the system
//Report Success or Failure
int kill(int pid);

//Exit| E: kill current running P
// Reports P scheduling info, which P now get control of CPU
void exit();

//Quantum| Q: time quantum of running P expire
//Report action taken (eg. process scheduling information)
int quantum();

//Send| S: send a msg to another P - block until reply
//Reports Success of Failure, scheduling info, once received the reply: the //msg and source of msg
int send(int pid,  char *msg);


//Receive| R: receive a msg - block until one msg arrive
//Reports scheduling info, once received the reply: the msg and source of msg
int receive();


//Reply| Y: unblock sender, deliver the reply
//Report success or failure
int reply (int pid, char *msg); //null terminate string, MAX lenght of msg = 40 char

//New sem| N: init name,value of SEM (0-4) (can do only once, error if attempt again) 
//Report: action taken, Success or Failure
int new_sem(int semID, int value);

//sem P |P : execute the semaphore P
//Report action taken (blocked or not), success or failure.
int P(int semID); //(semID range 0-4)

//sem V |V: execute the semaphore V
//Report action taken (whether P is readied), success or failure.
int V(int semID); //(semID range 0-4)

//Procinfo |I: dump complete state info of P (incl all status)
//Report see action
void proc_info(int pid);

//Totalinfo |T: display all process queues and their content
//Report see action
void total_info(void);


