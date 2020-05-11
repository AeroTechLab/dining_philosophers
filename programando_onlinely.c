//
// Created by grilo on 11/05/2020.
//
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdnoreturn.h>

#define N_PHILOSOFER 5

pthread_t tid[N_PHILOSOFER];

int forks[N_PHILOSOFER];

#define TABLE -1

int phil[N_PHILOSOFER];
int philState[N_PHILOSOFER];

pthread_mutex_t forkMtx[N_PHILOSOFER];

#define THINKING 0
#define EATING 1

int getLeftForkID(int philID)
{
    return philID;
}

int getRightForkID(int philID)
{
    return (philID+1) % N_PHILOSOFER;
}

noreturn void* philosofer(void* id)
{
    int myID = *((int *) id);
    
    int lForkID = getLeftForkID(myID);
    int rForkID = getRightForkID(myID);
    
    int *leftFork = &(forks[lForkID]);
    int *rightFork = &(forks[rForkID]);
    
    //printf("\t\t\tp%d L f%d R f%d\n", myID, getLeftForkID(myID), getRightForkID(myID));
    
    printf("p%d alive\n", myID);
    
    while(1)

    {
        philState[myID] = THINKING;
    
        printf("\t\tp%d thinking\n", myID);
        
        pthread_mutex_lock(&(forkMtx[lForkID]));
        
        *leftFork = myID;
        
//        do
//        {
//            if (*leftFork == TABLE)
//            {
//                *leftFork = myID;
//            }
//
//            //printf("p%d try L f%d [owner = %d]\n", myID, getLeftForkID(myID), *leftFork);
//        } while (*leftFork != myID);
        
        printf("p%d got L f%d\n", myID, getLeftForkID(myID));
    
    
        pthread_mutex_lock(&(forkMtx[rForkID]));
    
        *rightFork = myID;
        
//        do
//        {
//            if (*rightFork == TABLE)
//            {
//                *rightFork = myID;
//            }
//
//            //printf("p%d try L f%d [owner = %d]\n", myID, getRightForkID(myID), *rightFork);
//        } while (*rightFork != myID);
        
        printf("p%d got R f%d\n", myID, getRightForkID(myID));
        
        philState[myID] = EATING;
        
        printf("\t\tp%d eating\n", myID);
        
        usleep(2000000);
        
        *leftFork = TABLE;
        *rightFork = TABLE;
        
        pthread_mutex_unlock(&(forkMtx[lForkID]));
        pthread_mutex_unlock(&(forkMtx[rForkID]));
    }
}

int main()
{
    printf("lancando as threads\n");
    
    for(int i = 0; i < N_PHILOSOFER; ++i)
    {
        forks[i] = TABLE;
        pthread_mutex_init(&(forkMtx[i]), NULL);
    }
    
    for(int i = 0; i < N_PHILOSOFER; ++i)
    {
        phil[i] = i;
        pthread_create(&(tid[i]), NULL, philosofer, &(phil[i]));
    }
    
    for(int i = 0; i < N_PHILOSOFER; ++i)
    {
        pthread_join(tid[i], NULL);
        printf("p%d finished\n", i);
    }
    
    printf("acabaram as threads\n");
}
