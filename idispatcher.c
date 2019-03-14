/**
 * Mitchell Van Braeckel (mvanbrae@uoguelph.ca) 1002297
 * 11/03/2019
 * CIS*3110: Operating Systems A3 - CPU Scheduling: Simple Dispatcher
 * --> Simulates a simple dispacter of an OS using simulated events to do CPU scheduling
 * 
 *  (1) The sequence of events will be given in the standard input (one event per line).
 *  --> Empty line will signify the end of the input.
 *  --> Format of one line is as follows:
 *         <time> <event> {<process id>} ; where:
 *             - <time> - is an integer number denoting local time in milliseconds measured
 *                        from 0 (time will be strictly increasing from line to line)
 *             - <event> - is one of the following:
 *                 – C - create 
 *                 – E - exit 
 *                 – R N - request resource number N 
 *                 – I N - interrupt from resource number N (request accomplished)
 *                 – T - timer interrupt
 *             - <process id> - is a nonnegative integer, unique for each process
 *                              (system idle process has <process id> = 0);
 * 
 *  (2) Your Dispatcher will have to keep track of events and changes in the state of the
 *      processes, taking into account the following additional conditions:
 *      1. There are 5 different kinds of resources in the system and requests can be
 *          serviced out of order of arrival
 *      2. Time sharing - the process which is in running state is to be preempted as the
 *          result of the timer interrupt if there are other ready processes in the system
 *      3. Running process can also exit or get blocked because of request for a resource
 *      4. If there are no ready user processes, then process number 0
 *          (system idle process) is running
 *      5. If process 0 is running and new process is created, or as the result of an
 *          event one of the blocked processes becomes ready (unblocked), this process
 *          will get CPU immediately
 * 
 *  (3) When all lines of the input are processed, Dispatcher will print the following
 *      cumulative information about all processes admitted to the system during simulation:
 * 
 *      <process id> <total time Running> <total time Ready> <total time Blocked>
 * 
 *  --> For the system idle process, print only <total time Running> and assume that
 *      process 0 was created at time 0.
 *  --> The output is one line per process in the increasing order of process IDs.
 * 
 * You can assume that the sequence of events given in the input is consistent and no input errors are present.
 */

// =================================== INCLUDES ===================================
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// =================================== STRUCTS ====================================

typedef enum pstate { RUNNING, READY, BLOCKED } ProcessState;

/**
 * Simulated process control block (it will store all the necessary data)
 * --> stores: pid, prevTime, runTime, readyTime, blockTime
 */
typedef struct linked_list_node_struct {
    int pid, prevTime, runTime, readyTime, blockTime;
    struct linked_list_node_struct *next;
} PCB;

// ============================== FUNCTION PROTOTYPES =============================

PCB* createPCB(int currTime, int pid);
void deletePCB(PCB *toDelete);
void deleteQueue(PCB *queue);
void pushBack(PCB *queue, PCB *toAdd);
void insertSorted(PCB *queue, PCB *toAdd);
PCB* popFront(PCB *queue);
PCB* popID(PCB *queue, int pid);

void parseInputLine(char* line, int *prevTime, int *currTime, char *event, bool *riEvent, int *resourceNum, int *pid);

void flushInput(char* input);

// ================================================================================

//* create a PCB linked list node struct (it will store all the necessary data)
//*      --> stores: prevTime, runTime, readyTime, blockTime, pid
//***^ create helpers to push, pop (front and based on pid), and insert_sorted (for final output)

//* need a var to store running PCB
//* need a var to track total running time of the default system-idle process (process/pid=0)

//* need a list pointer for the ready queue (head and tail pointers)
//* need a list pointer for each of the 5 resource queues (head and tail pointers)

// declare variables for process queues
PCB *runningProcess = NULL; // store the running process
int idleTime = 0;           // track time spent idle
PCB* queues[6];             // first is ready, other 5 are resources

int main( int argc, char *argv[] ) {
    // init all queues to empty
    for(int i = 0; i < 6; i++) {
        queues[i] = NULL;
    }

    // declare variables
    char line[32];  // 32 char max
    char event = '\0';
    bool riEvent = false;
    int currTime = 0;
    int prevTime = 0;
    int resourceNum = -1;
    int pid = 0;
    
    // continue getting input until a blank line is entered
    while(1) {
        fgets(line, 32, stdin);
        flushInput(line);
        // if input line is blank (an empty string), stop
        if(line[0] == '\0') {
            break;
        }

        // parse input (time, event (& maybe resource #), process ID (if it's not T))
        parseInputLine(line, &prevTime, &currTime, &event, &riEvent, &resourceNum, &pid);
        
        // print testing output
        /*if(event == 'T') {
            printf("line = '%s'\t--> time = %5d | time diff = %5d | event = %c\n", 
                line, currTime, currTime-prevTime, event);
        } else {
            printf("line = '%s'\t--> time = %5d | time diff = %5d | event = %c", 
                line, currTime, currTime-prevTime, event);
            // print the resource number if necessary
            if(event == 'R' || event == 'I') {
                printf(" %d", resourceNum);
            } else {
                printf("  ");
            }
            printf(" | pid = %d\n", pid);
        }*/

        PCB *tester = createPCB(currTime, pid);
        //printf("PCB id = %2d | prevTime = %5d | runTime = %5d | readyTime = %5d | blockTime = %5d\n",
        //        tester->pid, tester->prevTime, tester->runTime, tester->readyTime, tester->blockTime);
        //deletePCB(tester);

        pushBack(queues[0], tester);
        if(queues[0] == NULL) {
            printf("\tis NULL\n");
        }
    }
    PCB *temp = queues[0];
    while(temp->next != NULL) {
        printf("PCB id = %2d | prevTime = %5d | runTime = %5d | readyTime = %5d | blockTime = %5d\n",
                temp->pid, temp->prevTime, temp->runTime, temp->readyTime, temp->blockTime);
        temp = temp->next;
    }
    deleteQueue(queues[0]);

    return 0;
}

// ============================ LINKED LIST FUNCTIONS =============================

/**
 * Creates and initializes the process control block
 * @param int currTime -the current time when it's created
 * @param int pid -its process ID
 * @return an allocated and initialized PCB
 */
PCB* createPCB(int currTime, int pid) {
    // create PCB
    PCB *new = NULL;
    new = malloc(sizeof(PCB));
    // init values
    new->runTime = new->readyTime = new->blockTime = 0;
    new->prevTime = currTime;
    new->pid = pid;
    new->next = NULL;
    return new;
}
/**
 * Deletes (Frees) a process, sets it to NULL after freeing
 * @param PCB *toDelete -the PCB to be deleted
 */
void deletePCB(PCB *toDelete) {
    // make sure it exists first
    if(toDelete == NULL) {
        return;
    }
    free(toDelete);
    toDelete = NULL;
}
/**
 * Deletes (Frees) a queue of processes
 * @param PCB *queue -the the queue to be deleted
 */
void deleteQueue(PCB *queue) {
    // make sure it exists first
    if(queue == NULL) {
        return;
    }
    // free all nodes in list, then set tail and head to NULL
    while(queue != NULL) {
        PCB *temp = queue;
        queue = queue->next;
        deletePCB(temp);
    }
    queue = NULL;   // reset queue ptr (just in case)
}

/**
 * Adds a process to the back of a queue
 * @param PCB *queue -the queue being added to
 * @param PCB *toAdd -the process being added
 */
void pushBack(PCB *queue, PCB *toAdd) {
    // if it's empty, set as first node in list
    if(queue == NULL) { 
        queue = toAdd;
        return;
    }
    // traverse to end of queue and add the node there (queue pointer remains the same)
    PCB *q = queue;
    while(q->next != NULL) {
        q = q->next;
    }
    q->next = toAdd;
}

/**
 * Adds a process to a queue, sorted by process ID (ascending order)
 * @param PCB *queue -the queue being added to
 * @param PCB *toAdd -the process being added
 */
void insertSorted(PCB *queue, PCB *toAdd) {
    // if it's empty, set as first node in list
    if(queue == NULL) { 
        queue = toAdd;
        return;
    }
    // traverse to end of queue and add the node there (queue pointer remains the same)
    PCB *q = queue;
    PCB *prev = q;
    while(q->next != NULL) {
        if(toAdd->pid <= q->pid) {
            // if it's first (or only) element, need to reset front of the queue
            if(queue == q) {
                toAdd->next = q;
                queue = toAdd;
            } else {
                prev->next = toAdd;
                toAdd->next = q;
            }
        }
        prev = q;
        q = q->next;
    }
    // otherwise, it's the highest pid and goes on the end
    q->next = toAdd;
}

/**
 * Pops and gets the front-most element of a queue
 * @param PCB *queue -the queue being accessed
 * @return the front-most element of the queue (not attached to the queue anymore),
 *          or NULL if not found
 */
PCB* popFront(PCB *queue) {
    // if it's empty, return NULL to indicate so
    if(queue == NULL) {
        return NULL;
    }
    // pop first node and reset the front of the queue
    PCB *toReturn = queue;
    queue = queue->next;
    toReturn->next = NULL;
    return toReturn;
}
/**
 * Pops and gets the process designated by process ID from a queue
 * @param PCB *queue -the queue being accessed
 * @param int pid -the process ID to be removed from the queue
 * @return the process designated by the given ID (not attached to the queue anymore),
 *          or NULL if it's empty or not found
 */
PCB* popID(PCB *queue, int pid) {
    // if it's empty, return NULL to indicate so
    if(queue == NULL) {
        return NULL;
    }
    // loop through and search for the process
    PCB *q = queue;
    PCB *prev = q;
    while(q->next != NULL) {
        if(q->pid == pid) {
            // if it's the first (or only) element, need to reset front of the queue
            if(q == queue) {
                return popFront(queue);
            } else {
                prev->next = q->next;
                q->next = NULL;
                return q;
            }
        }
        prev = q;
        q = q->next;
    }
    return NULL;    // pid wasn't found
}

// =============================== HELPER FUNCTIONS ===============================

/**
 * Helper that parses an input line (assuming there are no errors) storing in appropriate variables
 * @param char* line -the input line being parsed
 * @param int *prevTime -will hold the most recent time
 * @param int *currTime -will hold the parsed time
 * @param char *event -will hold the parsed event
 * @param bool *riEvent -will be true if the event is 'R' or 'I', otherwise false
 * @param int *resourceNum -will hold the resource number if event is 'R' or 'I', otherwise -1
 * @param int *pid -will hold the process ID (-1 if event is 'T')
 */
void parseInputLine(char* line, int *prevTime, int *currTime, char *event, bool *riEvent, int *resourceNum, int *pid) {
    // declare var for parsing input lines
    char *token;
    
    // parse time
    *prevTime = *currTime; // keep track of this to calculate difference
    token = strtok(line, " ");
    *currTime = atoi(token);

    // parse event
    token = strtok(NULL, " ");
    *event = token[0];

    // parse resource number - only if the event is 'R' or 'I'
    *riEvent = false; //reset
    if(*event == 'R' || *event == 'I') {
        *riEvent = true;
        token = strtok(NULL, " ");
        *resourceNum = atoi(token);
    } else {
        *resourceNum = -1;
    }

    // parse process ID - check if the event is 'T', in which case there isn't one
    if(*event != 'T') {
        token = strtok(NULL, " ");
        *pid = atoi(token);
    } else {
        *pid = -1;
    }
}

// ================================== MY HELPERS ==================================

/**
 * Flushes all leftover data in the stdin stream
 * @param char* input -the string that was just read from stdin
 */
void flushInput(char* input) {
    // if the '\n' is NOT found in the word itself, flush the stream (null-terminate the input regardless)
    if(strchr(input, '\n') == NULL) {
        while ((getchar()) != '\n');
        input[strlen(input)] = '\0';
    } else {
        input[strlen(input)-1] = '\0';
    }
}
