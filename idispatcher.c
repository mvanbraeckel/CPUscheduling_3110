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
#include <string.h>

// =================================== STRUCTS ====================================

typedef enum pstate { NEW, RUNNING, READY, BLOCKED, TERMINATED } ProcessState;

/**
 * Simulated process control block (it will store all the necessary data)
 * --> stores: pid, prevTime, runTime, readyTime, blockTime
 */
typedef struct linked_list_node_struct {
    int pid, prevTime, runTime, readyTime, blockTime;
    ProcessState status;
    struct linked_list_node_struct *next;
} PCB;

// ============================== FUNCTION PROTOTYPES =============================

PCB* createPCB(int currTime, int pid);
void deletePCB(PCB **toDelete);
void deleteQueue(PCB **queue);
void pushBack(PCB **queue, PCB *toAdd);
void insertSorted(PCB **queue, PCB *toAdd);
PCB* popFront(PCB **queue);
PCB* popID(PCB **queue, int pid);
void printQueue(PCB **queue);

void parseInputLine(char* line, int *prevTime, int *currTime, char *event, int *resourceNum, int *pid);

void flushInput(char* input);

// ================================================================================

int main( int argc, char *argv[] ) {
    // declare variables for process queues
    PCB *runningProcess = NULL; // store the running process
    int idleTime = 0;           // track time spent idle
    PCB* queues[7];             // first is ready, other 5 are resources, then a "done" queue

    // init all queues to empty
    for(int i = 0; i < 7; i++) {
        queues[i] = NULL;
    }

    // declare variables
    char line[102];  // 100 char max
    char event = '\0';
    int currTime = 0;
    int prevTime = 0;
    int resourceNum = -1;
    int pid = 0;
    
    // continue getting input until a blank line is entered
    while(1) {
        fgets(line, 102, stdin);
        flushInput(line);
        // if input line is blank (an empty string), stop
        if(line[0] == '\0') break;

        // parse input (time, event (& maybe resource #), process ID (if it's not T))
        parseInputLine(line, &prevTime, &currTime, &event, &resourceNum, &pid);

        // based on command, update time and then execute functionality
        if(event == 'C') {          // ========================== C ==========================
            // make sure input is valid
            if(currTime < prevTime || currTime < 0) {
                fprintf(stderr, "Error: local time stamp must be a strictly-increasing, non-negative integer - input line will be ignored\n");
                continue;
            } else if(pid < 0) {
                fprintf(stderr, "Error: process ID must be a non-negative integer - input line will be ignored\n");
                continue;
            }
            
            // create new process
            PCB *newPCB = createPCB(currTime, pid);
            // if a process is not running, update idle time and make it run
            // otherwise, add it to the end of the ready queue
            if(runningProcess == NULL) {
                idleTime += currTime - prevTime;
                newPCB->status = RUNNING;
                runningProcess = newPCB;
            } else {
                newPCB->status = READY;
                pushBack(&queues[0], newPCB);
            }

        } else if(event == 'E') {   // ========================== E ==========================
            // make sure input is valid
            if(currTime < prevTime || currTime < 0) {
                fprintf(stderr, "Error: local time stamp must be a strictly-increasing, non-negative integer - input line will be ignored\n");
                continue;
            } else if(pid < 0) {
                fprintf(stderr, "Error: process ID must be a non-negative integer - input line will be ignored\n");
                continue;
            }

            // find the given process, update time, then remove and add to the done queue
            if(runningProcess != NULL && runningProcess->pid == pid) {
                // update total run time first
                runningProcess->runTime += currTime - runningProcess->prevTime;
                runningProcess->prevTime = currTime;
                // stop running (set it back to system idle process 0), put in done queue
                PCB *done = runningProcess;
                runningProcess = NULL;
                done->status = TERMINATED;
                insertSorted(&queues[6], done);

                // run the first element in the ready queue, if there is one
                PCB *toRun = NULL;
                toRun = popFront(&queues[0]);
                if(toRun != NULL) {
                    // update total ready time first
                    toRun->readyTime += currTime - toRun->prevTime;
                    toRun->prevTime = currTime;
                    // have it run
                    toRun->status = RUNNING;
                    runningProcess = toRun;
                }
            }
            // otherwise, search for it in the ready and resource queues
            else {
                fprintf(stderr, "Notice: terminating process %d from a non-running state\n", pid);
                // search ready queue
                PCB *done = NULL;
                done = popID(&queues[0], pid);

                if(done != NULL) {
                    // update total ready time first
                    done->readyTime += currTime - done->prevTime;
                    done->prevTime = currTime;
                    // put in done queue
                    done->status = TERMINATED;
                    insertSorted(&queues[6], done);
                }
                // wasn't in ready queue, so search each resource queue
                else {
                    int foundMatch = 0;
                    for(int i = 1; i < 6; i++) {
                        done = popID(&queues[i], pid);
                        if(done != NULL) {
                            // update total blocked time first
                            done->blockTime += currTime - done->prevTime;
                            done->prevTime = currTime;
                            // put in done queue
                            done->status = TERMINATED;
                            insertSorted(&queues[6], done);
                            foundMatch = 1;
                            break;
                        }
                    }
                    if(foundMatch) continue;
                    // if it reaches here, pid DNE, so display error message, ignore line
                    fprintf(stderr, "Error: process ID %d does not exist --ignoring input line\n", pid);
                }
            }

        } else if(event == 'R') {   // ========================== R ==========================
            // make sure input is valid
            if(currTime < prevTime || currTime < 0) {
                fprintf(stderr, "Error: local time stamp must be a strictly-increasing, non-negative integer - input line will be ignored\n");
                continue;
            } else if(resourceNum < 1 || resourceNum > 5) {
                fprintf(stderr, "Error: resource number must be an integer [1,5] - input line will be ignored\n");
                continue;
            } else if(pid < 0) {
                fprintf(stderr, "Error: process ID must be a non-negative integer - input line will be ignored\n");
                continue;
            }

            // find the given process, update time, then remove and add to the specified resource queue
            if(runningProcess != NULL && runningProcess->pid == pid) {
                // update total run time first
                runningProcess->runTime += currTime - runningProcess->prevTime;
                runningProcess->prevTime = currTime;
                // stop running (set back to system idle), put in specified resource queue
                PCB *toBlock = runningProcess;
                runningProcess = NULL;
                toBlock->status = BLOCKED;
                pushBack(&queues[resourceNum], toBlock);

                // run the first element in the ready queue, if there is one
                PCB *toRun = NULL;
                toRun = popFront(&queues[0]);
                if(toRun != NULL) {
                    // update total ready time first
                    toRun->readyTime += currTime - toRun->prevTime;
                    toRun->prevTime = currTime;
                    // have it run
                    toRun->status = RUNNING;
                    runningProcess = toRun;
                }
            }
            // otherwise, search for it in the ready and resource queues
            else {
                fprintf(stderr, "Notice: blocking process %d from a non-running state\n", pid);
                // search ready queue
                PCB *toBlock = NULL;
                toBlock = popID(&queues[0], pid);
                if(toBlock != NULL) {
                    // update total ready time first
                    toBlock->readyTime += currTime - toBlock->prevTime;
                    toBlock->prevTime = currTime;
                    // put in specified resource queue
                    toBlock->status = BLOCKED;
                    pushBack(&queues[resourceNum], toBlock);
                }
                // wasn't in ready queue, so search each resource queue
                else {
                    int foundMatch = 0;
                    for(int i = 1; i < 6; i++) {
                        toBlock = popID(&queues[i], pid);
                        if(toBlock != NULL) {
                            // update total blocked time first
                            toBlock->blockTime += currTime - toBlock->prevTime;
                            toBlock->prevTime = currTime;
                            // put in specified resource queue
                            toBlock->status = BLOCKED;
                            pushBack(&queues[resourceNum], toBlock);
                            foundMatch = 1;
                            break;
                        }
                    }
                    if(foundMatch) continue;
                    // if it reaches here, pid DNE, so display error message, ignore line
                    fprintf(stderr, "Error: process ID %d does not exist --ignoring input line\n", pid);
                }
            }

        } else if(event == 'I') {   // ========================== I ==========================
            // make sure input is valid
            if(currTime < prevTime || currTime < 0) {
                fprintf(stderr, "Error: local time stamp must be a strictly-increasing, non-negative integer - input line will be ignored\n");
                continue;
            } else if(resourceNum < 1 || resourceNum > 5) {
                fprintf(stderr, "Error: resource number must be an integer [1,5] - input line will be ignored\n");
                continue;
            } else if(pid < 0) {
                fprintf(stderr, "Error: process ID must be a non-negative integer - input line will be ignored\n");
                continue;
            }

            // remove specified process from specified resource queue, then add to ready queue
            PCB *fromRQ = NULL;
            fromRQ = popID(&queues[resourceNum], pid);
            if(fromRQ != NULL) {
                // update total blocked time first
                fromRQ->blockTime += currTime - fromRQ->prevTime;
                fromRQ->prevTime = currTime;
                // if a process is not running, update idle time and make it run
                if(runningProcess == NULL) {
                    idleTime += currTime - prevTime;
                    fromRQ->status = RUNNING;
                    runningProcess = fromRQ;
                }
                // otherwise, add it to the end of the ready queue
                else {
                    fromRQ->status = READY;
                    pushBack(&queues[0], fromRQ);
                }
            } else {
                fprintf(stderr, "Error: process ID %d does not exist in resource %d's queue --ignoring input line\n", pid, resourceNum);
            }

        } else if(event == 'T') {   // ========================== T ==========================
            // make sure input is valid
            if(currTime < prevTime || currTime < 0) {
                fprintf(stderr, "Error: local time stamp must be a strictly-increasing, non-negative integer - input line will be ignored\n");
                continue;
            }

            // check if a process is running first
            if(runningProcess == NULL || queues[0] == NULL) {
                continue;
            }

            // update total run time first
            runningProcess->runTime += currTime - runningProcess->prevTime;
            runningProcess->prevTime = currTime;
            // put the running process in the ready queue
            PCB *ready = runningProcess;
            runningProcess = NULL;      //set it to system idle process 0 temporarily
            ready->status = READY;
            pushBack(&queues[0], ready);

            // run the first element in the ready queue (already checked it wasn't empty earlier)
            PCB *toRun = NULL;
            toRun = popFront(&queues[0]);
            if(toRun != NULL) {
                // update total ready time first
                toRun->readyTime += currTime - toRun->prevTime;
                toRun->prevTime = currTime;
                // have it run
                toRun->status = RUNNING;
                runningProcess = toRun;
            }
            
        } else {                    // ========================= ELSE ========================
            // display error message, and ignore line
            fprintf(stderr, "Error: invalid event --ignoring input line\n");
        } // end if statement
    } // end while loop

    // delete all PCBs from ready and resource queues, then delete the running process
    // (NOTE: all of these should be empty though)
    for(int i = 0; i < 6; i++) {
        // display msg if it's not empty
        if(queues[i] != NULL) {
            fprintf(stderr, "Error: queue %d should be empty, but isn't\n", i);
        }
        deleteQueue(&queues[i]);
    }
    // display msg if it's not empty
    if(runningProcess != NULL) {
        fprintf(stderr, "Error: there shouldn't be a running process, but there is\n");
    }
    deletePCB(&runningProcess);

    // display program output (first idle time, then all completed processes' times)
    printf("0 %d\n", idleTime);
    printQueue(&queues[6]);

    // delete all finished processes after printing, before exiting program
    deleteQueue(&queues[6]);
    if(queues[6] != NULL) {
        fprintf(stderr, "Error: done queue should be empty, but isn't\n");
    }

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
    new->status = NEW;
    new->next = NULL;
    return new;
}
/**
 * Deletes (Frees) a process, sets it to NULL after freeing
 * @param PCB **toDelete -the PCB to be deleted
 */
void deletePCB(PCB **toDelete) {
    // make sure it exists first
    if(*toDelete == NULL) {
        return;
    }
    free(*toDelete);
    *toDelete = NULL;
}
/**
 * Deletes (Frees) a queue of processes
 * @param PCB **queue -the the queue to be deleted
 */
void deleteQueue(PCB **queue) {
    // make sure it exists first
    if((*queue) == NULL) {
        return;
    }
    // free all nodes in list, then set tail and head to NULL
    while((*queue) != NULL) {
        PCB *temp = (*queue);
        (*queue) = (*queue)->next;
        deletePCB(&temp);
    }
    (*queue) = NULL;   // reset queue ptr (just in case)
}

/**
 * Adds a process to the back of a queue
 * @param PCB **queue -the queue being added to
 * @param PCB *toAdd -the process being added
 */
void pushBack(PCB **queue, PCB *toAdd) {
    // make sure process given is valid
    if(toAdd == NULL) {
        return;
    }
    // if it's empty, set as first node in list
    if((*queue) == NULL) { 
        (*queue) = toAdd;
        return;
    }
    // traverse to end of queue and add the node there (queue pointer remains the same)
    PCB *q = (*queue);
    while(q->next != NULL) {
        q = q->next;
    }
    q->next = toAdd;
}

/**
 * Adds a process to a queue, sorted by process ID (ascending order)
 * @param PCB **queue -the queue being added to
 * @param PCB *toAdd -the process being added
 */
void insertSorted(PCB **queue, PCB *toAdd) {
    // make sure process given is valid
    if(toAdd == NULL) {
        return;
    }
    // if it's empty, set as first node in list
    if((*queue) == NULL) { 
        (*queue) = toAdd;
        return;
    }

    // check if it's the first (or only) element
    if(toAdd->pid < (*queue)->pid) {
        toAdd->next = (*queue);
        (*queue) = toAdd;
        return;
    } 
    // otherwise, traverse to end of queue and add the node there (queue pointer remains the same)
    PCB *q = (*queue);
    PCB *prev = q;
    while(q->next != NULL) {
        if(toAdd->pid < q->pid) {
            // add it since it's less than the next node
            prev->next = toAdd;
            toAdd->next = q;
            return;
        }
        prev = q;
        q = q->next;
    }
    // check if it's less than the last element
    if(toAdd->pid < q->pid) {
        prev->next = toAdd;
        toAdd->next = q;
        return;
    }
    // otherwise, it's the highest pid and goes on the end
    q->next = toAdd;
}

/**
 * Pops and gets the front-most element of a queue
 * @param PCB **queue -the queue being accessed
 * @return the front-most element of the queue (not attached to the queue anymore),
 *          or NULL if not found
 */
PCB* popFront(PCB **queue) {
    // if it's empty, return NULL to indicate so
    if((*queue) == NULL) {
        return NULL;
    }
    // pop first node and reset the front of the queue
    PCB *toReturn = (*queue);
    (*queue) = (*queue)->next;
    toReturn->next = NULL;
    return toReturn;
}
/**
 * Pops and gets the process designated by process ID from a queue
 * @param PCB **queue -the queue being accessed
 * @param int pid -the process ID to be removed from the queue
 * @return the process designated by the given ID (not attached to the queue anymore),
 *          or NULL if it's empty or not found
 */
PCB* popID(PCB **queue, int pid) {
    // if it's empty, return NULL to indicate so
    if((*queue) == NULL) {
        return NULL;
    }

    // check if it's the first (or only) element
    if((*queue)->pid == pid) {
        return popFront(queue);
    }
    // otherwise, loop through and search for the process (queue pointer remains the same)
    PCB *q = (*queue);
    PCB *prev = q;
    while(q != NULL) {
        if(q->pid == pid) {
            // it's a match, so pop it
            prev->next = q->next;
            q->next = NULL;
            return q;
        }
        prev = q;
        q = q->next;
    }
    return NULL;    // pid wasn't found
}

/**
 * Prints the info of each process in the queue
 *  --> Format: <process id> <total time Running> <total time Ready> <total time Blocked>
 * @param PCB **queue -the queue to be printed
 */
void printQueue(PCB **queue) {
    // make sure it's not empty
    if((*queue) == NULL) {
        return;
    }
    // loop through and print each process' info
    PCB *curr = (*queue);
    while(curr != NULL) {
        printf("%d %d %d %d\n", curr->pid, curr->runTime, curr->readyTime, curr->blockTime);
        curr = curr->next;
    }
}

// =============================== HELPER FUNCTIONS ===============================

/**
 * Helper that parses an input line (assuming there are no errors) storing in appropriate variables
 * @param char* line -the input line being parsed
 * @param int *prevTime -will hold the most recent time
 * @param int *currTime -will hold the parsed time
 * @param char *event -will hold the parsed event
 * @param int *resourceNum -will hold the resource number if event is 'R' or 'I', otherwise -1
 * @param int *pid -will hold the process ID (-1 if event is 'T')
 */
void parseInputLine(char* line, int *prevTime, int *currTime, char *event, int *resourceNum, int *pid) {
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
    if(*event == 'R' || *event == 'I') {
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
