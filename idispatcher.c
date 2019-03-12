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

// ============================== FUNCTION PROTOTYPES =============================

void flushInput(char* input);

// ================================================================================

int main( int argc, char *argv[] ) {
    // declare variables
    char line[32];  // 32 char max
    char *token;    // for parsing input lines
    int currTime = 0;
    int prevTime = 0;
    char event = '\0';
    int resourceNum = -1;
    int pid = 0;
    bool riEvent = false;

    // continue getting input until a blank line is entered
    while(1) {
        fgets(line, 32, stdin);
        flushInput(line);
        printf("first char = '%c'\n", line[0]);
        // if input line is blank (empty string), stop
        if(line[0] == '\0') {
            break;
        }

        // parse input (time, event (& maybe resource #), process ID)
        prevTime = currTime; // keep track of
        token = strtok(line, " ");
        currTime = atoi(token);
        token = strtok(NULL, " ");
        event = token[0];
        // check if the event is 'R' or 'I', in which case it is followed by an extra argument
        if(event == 'R' || event == 'I') {
            riEvent = true;
            token = strtok(NULL, " ");
            resourceNum = atoi(token);
        } else {
            resourceNum = -1;
        }
        // check if the event is 'T', in which case there is no process ID
        if(event != 'T') {
            token = strtok(NULL, " ");
            pid = atoi(token);
        } else {
            pid = -1;
        }
        
        // print testing output
        if(event == 'T') {
            printf("line = '%s'\t--> time = %5d | time diff = %5d | event=%c\n", 
                line, currTime, currTime-prevTime, event);
        } else {
            printf("line = '%s'\t--> time = %5d | time diff = %5d | event=%c", 
                line, currTime, currTime-prevTime, event);
            // print the resource number if necessary
            if(event == 'R' || event == 'I') {
                printf(" %d", resourceNum);
            } else {
                printf("  ");
            }
            printf(" | pid = %d\n", pid);
        }
        
    }

    return 0;
}

// ================================================================================

/**
 * Flushes all leftover data in the stream
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