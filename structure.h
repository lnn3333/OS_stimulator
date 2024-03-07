#ifndef STRUCTURE_H
#define STRUCTURE_H
#include "list.h"
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
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
    enum processState state;
    List *proc_message;
};

// Semaphore data structure

typedef struct sem_S sem;
struct sem_S
{
    int semID;
    int value;
    List *pList;
};

// Define a structure to represent a message
typedef struct message_S message;
struct message_S
{
    int senderPid;
    char msg[MAX_MSG_LENGTH + 1]; // +1 for null terminator
};

// Ready Q based on priority (high 2, medium 1, low 0)
extern List *lowPriority;
extern List *mediumPriority;
extern List *highPriority;
extern PCB *runningP;
extern PCB *initP;
extern sem semList[5];
#endif // STRUCTURES_H
