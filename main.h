#ifndef _MAIN_H_
#define _MAIN_H_

#include "help.h"

// Functions
// Create| C: create a P, add to ready Q
// Report Success or Failure, pid of created P
bool createProcess(int priority); //(0 = high, 1 = norm, 2 = low)

// Fork| F: copy current running P, add to ready Q
// Report Success or Failure, pid of new P
//  attemp to Fork the init P should fail
bool forkProcess();

// Kill| K: kill P, remove P from the system
// Report Success or Failure
bool kill(int pid);

// Exit| E: kill current running P
//  Reports P scheduling info, which P now get control of CPU
void exitProcess();

//helper function
bool cpu_scheduler();
// Quantum| Q: time quantum of running P expire
// Report action taken (eg. process scheduling information)
void quantum();

// Send| S: send a msg to another P - block until reply
// Reports Success of Failure, scheduling info, once received the reply: the //msg and source of msg
bool send(int pid, char *msg);

// Receive| R: receive a msg - block until one msg arrive
// Reports scheduling info, once received the reply: the msg and source of msg
bool receive();

// Reply| Y: unblock sender, deliver the reply
// Report success or failure
bool reply(int pid, char *msg); // null terminate string, MAX lenght of msg = 40 char

// New sem| N: init name,value of SEM (0-4) (can do only once, error if attempt again)
// Report: action taken, Success or Failure
bool new_sem(int semID);

// sem P |P : execute the semaphore P
// Report action taken (blocked or not), success or failure.
bool P(int semID); //(semID range 0-4)

// sem V |V: execute the semaphore V
// Report action taken (whether P is readied), success or failure.
bool V(int semID); //(semID range 0-4)

// Procinfo |I: dump complete state info of P (incl all status)
// Report see action
void proc_info(int pid);

// Totalinfo |T: display all process queues and their content
// Report see action
void total_info(void);

void processCommand();

// init Proccess
void Init();

// CPU scheduler using round robin for preemptive
bool cpu_scheduler();


#endif // _MAIN_H_