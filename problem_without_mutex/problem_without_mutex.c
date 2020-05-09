#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#include <pthread.h>
#include <sys/time.h>
#include <signal.h>

#define EATING 1
#define THINKING 0
#define TABLE -1

#ifndef N_PHILOSOFERS
    #define N_PHILOSOFERS 5
#endif
#define N_FORKS N_PHILOSOFERS

#define LEFT_FORK(philosoferID) philosoferID
#define RIGHT_FORK(philosoferID) ((philosoferID+1) % N_PHILOSOFERS)

#define N_TIMELOG 100
#define MAX_WHAT_LENGTH 100

int state[N_PHILOSOFERS];
int phil[N_PHILOSOFERS];
int philStatus[N_PHILOSOFERS];
int forks[N_FORKS];

pthread_mutex_t forkMtx[N_FORKS];

static volatile int keepRunning = 1;

pthread_t thread_id[N_PHILOSOFERS];

char eventLog[N_PHILOSOFERS][N_TIMELOG][MAX_WHAT_LENGTH];
long int timeLog[N_PHILOSOFERS][N_TIMELOG];

void intHandler(int dummy) {
    keepRunning = 0;
    for (int i = 0; i < N_PHILOSOFERS; ++i)
    {
        pthread_cancel(thread_id[i]);
    }
}

int saveToLog(int *eventLogIndex, char* what, struct timeval* currentTime, int myID)
{
    int stop = 0;
    
    if (*eventLogIndex >= N_TIMELOG)
    {
        stop = 1;
    }
    else
    {
        gettimeofday(currentTime, NULL);
        timeLog[myID][*eventLogIndex] = 1000000*currentTime->tv_sec+currentTime->tv_usec;
        sprintf(eventLog[myID][*eventLogIndex], "%10ld %s", timeLog[myID][*eventLogIndex], what);
        *eventLogIndex = *eventLogIndex + 1;
        
        stop = 0;
    }
    
    return stop;
}

void* philosopher(void* philosopherID)
{
    int myPhilosoferID = *((int*) philosopherID);
    int myLeftForkID = LEFT_FORK(myPhilosoferID);
    int myRightForkID = RIGHT_FORK(myPhilosoferID);
    
    char what[MAX_WHAT_LENGTH];
    struct timeval currentTime;
    
    philStatus[myPhilosoferID] = THINKING;
    
    usleep(1000000);
    
    int eventLogIndex = 0;
    int internalKeepRunning = 1;
    
    while (keepRunning && internalKeepRunning) {
    
        //TRY LEFT FORK
        sprintf(what, "p%d try L f%d", myPhilosoferID, myLeftForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        if(!internalKeepRunning) pthread_exit(0);
        
        while(forks[myLeftForkID] != TABLE ){}
        
        //GOT LEFT FORK
        forks[myLeftForkID] = myPhilosoferID;
        
        sprintf(what, "p%d got L f%d", myPhilosoferID, myLeftForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        
        if(!internalKeepRunning)
        {
            forks[myLeftForkID] = TABLE;
            pthread_exit(0);
        }
        
        //TRY RIGHT FORK
        sprintf(what, "p%d try R f%d", myPhilosoferID, myRightForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        if(!internalKeepRunning)
        {
            forks[myLeftForkID] = TABLE;
            pthread_exit(0);
        }
        while( forks[myRightForkID] != TABLE){}
        
        //GOT RIGHT FORK
        forks[myRightForkID] = myPhilosoferID;
        
        sprintf(what, "p%d got R f%d", myPhilosoferID, myRightForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        
        if(!internalKeepRunning)
        {
            forks[myRightForkID] = TABLE;
            forks[myLeftForkID] = TABLE;
            pthread_exit(0);
        }
        
        //STARTS EATING
        philStatus[myPhilosoferID] = EATING;
        
        sprintf(what, "p%d eating", myPhilosoferID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        if(!internalKeepRunning)
        {
            //TODO: print imout on file
            forks[myRightForkID] = TABLE;
            forks[myLeftForkID] = TABLE;
            pthread_exit(0);
        }
        
        //EATS FOR 2 SECONDS
        usleep(2000000);
        
        //STOPING EATING, STARTS THINKING
        philStatus[myPhilosoferID] = THINKING;
        sprintf(what, "p%d thinking", myPhilosoferID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        
        if(!internalKeepRunning)
        {
            forks[myRightForkID] = TABLE;
            forks[myLeftForkID] = TABLE;
            pthread_exit(0);
        }
        
        //DROPPING LEFT FORK
        forks[myLeftForkID] = TABLE;
        
        sprintf(what, "p%d drop L f%d", myPhilosoferID, myLeftForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        
        if(!internalKeepRunning)
        {
            forks[myRightForkID] = TABLE;
            pthread_exit(0);
        }
        
        //DROPPING RIGHT FORK
        forks[myRightForkID] = TABLE;
        
        sprintf(what, "p%d drop R f%d", myPhilosoferID, myRightForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        
        if(!internalKeepRunning) pthread_exit(0);
        
        //THINKING FOR 3 SECONDS
        usleep(300000);
    }
}

int main()
{
    int i;
    
    signal(SIGINT, intHandler);
    
    srand(time(NULL));
    
    for (i = 0; i < N_PHILOSOFERS; i++) {
        forks[i] = TABLE;
        phil[i] = i;
        pthread_mutex_init(&(forkMtx[i]),NULL);
    }
    
    usleep(1000000);
    
    printf("launching threads\n");
    
    for (i = 0; i < N_PHILOSOFERS; ++i)
    {
        // create philosopher processes
        pthread_create(&thread_id[i], NULL,
                       philosopher, &phil[i]);
    }
    
    printf("threads launched!\n");
    
    for (i = 0; i < N_PHILOSOFERS; ++i)
    {
        pthread_join(thread_id[i], NULL);
        
    }
    
    printf("all thread joined.\n");
    
    char fileName[20];
    
    FILE *f;
    
    for (int p = 0; p < N_PHILOSOFERS; ++p)
    {
        sprintf(fileName, "logPhil%d.txt", p);
        
        printf("creating file %s\n", fileName);
        
        f = fopen(fileName, "wb");
        
        for (i = 0; i < N_TIMELOG; ++i)
        {
            fputs(eventLog[p][i],f);
            fwrite("\n", sizeof(char), 1, f);
        }
        
        fclose(f);
        printf("file %s created\n", fileName);
        
    }
}
