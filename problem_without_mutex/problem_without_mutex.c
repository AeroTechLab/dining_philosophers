#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdnoreturn.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "colors.h"
#include <curses.h>
#include <math.h>

#define N_PHILOSOFERS 12
#define N_FORKS N_PHILOSOFERS
#define EATING 1
#define THINKING 0
#define LEFT_FORK(philosoferID) ((philosoferID + N_PHILOSOFERS-1) % N_PHILOSOFERS)
#define RIGHT_FORK(philosoferID) ((philosoferID) % N_PHILOSOFERS)

#define N_TIMELOG 100

int state[N_PHILOSOFERS];
int phil[N_PHILOSOFERS];
int philStatus[N_PHILOSOFERS];
int forks[N_FORKS];

pthread_mutex_t mtx_timeLog;

float timeStamp[N_TIMELOG];
char what[N_TIMELOG][30];
int id[N_TIMELOG];
int update = 0;
int nPhilEating = 0;
int nPhilThinking = 0;


int imout = 0;

typedef struct coord_
{
    int x;
    int y;
} coord;

typedef struct philCoord_
{
    coord status;
    coord forkL;
    coord forkR;
} philCoord;

coord forkCoords[N_FORKS];
philCoord philCoords[N_PHILOSOFERS];

typedef unsigned long long u64;

u64 u64useconds;
struct timeval beginingOfThinkingTimes[N_PHILOSOFERS];

//void markTime(char* what)
//{
//    pthread_mutex_lock(&mtx_timeLog);
//        gettimeofday(&tv,NULL);
//        u64useconds = (1000000*tv.tv_sec) + tv.tv_usec;
//    pthread_mutex_unlock(&mtx_timeLog);
//}
//
//void printForks()
//{
//    printf("\t | ");
//    for(int i = 0; i < N_FORKS; ++i)
//    {
//        printf("  %4s [%2d]", "fork", i);
//    }
//
//    printf("\n\t | ");
//
//    for(int i = 0; i < N_FORKS; ++i)
//    {
//        if(forks[i] == -1)
//            printf("  %8s", "none");
//        else
//            printf("  %8d", forks[i]);
//    }
//
//    printf("\n");
//}
//
//void printPhilosofers()
//{
//    printf("\t | ");
//    for(int i = 0; i < N_PHILOSOFERS; ++i)
//    {
//        printf("  %4s [%2d]", "phil", i);
//    }
//
//    printf("\n\t | ");
//
//    for(int i = 0; i < N_PHILOSOFERS; ++i)
//    {
//        printf("  %s", philStatus[i] == THINKING ? BLUE("thinking") : RED("  eating"));
//    }
//
//    printf("\n");
//}

// take up chopsticks
int takeFork(int philosoferID, int forkID)
{
    int youGotTheFork = 0;
    
    if (forks[forkID] == -1 || forks[forkID] == philosoferID)
    {
        forks[forkID] = philosoferID;
        youGotTheFork = 1;
    }
    
    return youGotTheFork;
}

int putFork(int forkID)
{
    forks[forkID] = -1;
    return 0;
}

void* philospher(void* num)
{
    int gotLeftFork = 0;
    int gotRightFork = 0;
    
    int myPhilosoferID = *((int*)num);
    int myLeftForkID = LEFT_FORK(myPhilosoferID);
    int myRightForkID = RIGHT_FORK(myPhilosoferID);
    
    struct timeval currentTime;
    
    philStatus[myPhilosoferID] = THINKING;
    nPhilThinking++;
    
    long int triesCount = 0;
    
    int i=0;
    
    sleep(1);
    
    gettimeofday(&(beginingOfThinkingTimes[myPhilosoferID]), NULL);
    
    while (1) {
        
        do
        {

            gotLeftFork = takeFork(myPhilosoferID, myLeftForkID);
            
            gotRightFork = takeFork(myPhilosoferID, myRightForkID);
            
            gettimeofday(&currentTime, NULL);
            
//            if(1000000*(currentTime.tv_sec - beginingOfThinkingTimes[myPhilosoferID].tv_sec) - beginingOfThssssinkingTimes[myPhilosoferID].tv_usec > 10000000)
//            {
//                imout++;
//                pthread_exit(0);
//            }
            
        } while (!(gotLeftFork && gotRightFork));
        
        philStatus[myPhilosoferID] = EATING;
        nPhilEating++;
        nPhilThinking--;
        
        update = myPhilosoferID;
        
        sleep(2);
    
        gotLeftFork = putFork(RIGHT_FORK(myLeftForkID));
        
        
        gotRightFork = putFork(LEFT_FORK(myRightForkID));
        
        gettimeofday(&(beginingOfThinkingTimes[myPhilosoferID]), NULL);
        philStatus[myPhilosoferID] = THINKING;
        nPhilThinking++;
        nPhilEating--;
        update = myPhilosoferID;
    }
}

#define C_FORK_TABLE      1
#define C_FORK_NOT_TABLE  2
#define C_EATING          3
#define C_THINKING        4

void* printUpdate(void* unused)
{
    initscr();			/* Start curses mode 		*/
    raw();				/* Line buffering disabled	*/
    struct timeval currentTime;
    
    if (has_colors() == FALSE) {
        endwin();
        printf("Your terminal does not support color\n");
        exit(1);
    }
    
    start_color();
    
    int i = 0;
    
    for (i = 0; i < N_PHILOSOFERS; ++i)
    {
        philCoords[i].status.y = 1;
        philCoords[i].status.x = 10 * i + 2;
        
        philCoords[i].forkL.y = 2;
        philCoords[i].forkL.x = philCoords[i].status.x;
        
        philCoords[i].forkL.y = 3;
        philCoords[i].forkL.x = philCoords[i].status.x;
        
    }
    
    for (i = 0; i < N_FORKS; ++i)
    {
        forkCoords[i].x = i * 5 + 12;
        forkCoords[i].y = 4;
    }
    
    init_pair(C_FORK_TABLE, COLOR_BLUE, COLOR_BLACK);
    init_pair(C_FORK_NOT_TABLE, COLOR_RED, COLOR_BLACK);
    init_pair(C_EATING, COLOR_GREEN, COLOR_BLACK);
    init_pair(C_THINKING, COLOR_CYAN, COLOR_BLACK);
    
    for (int i = 0; i < N_PHILOSOFERS; ++i)
    {
        mvprintw(0, philCoords[i].status.x, "  phil %2d  ", i);
    }
    
    mvprintw(4,0,"on table: ");
    for(i = 0; i<N_FORKS; ++i)
    {
        attron(COLOR_PAIR(C_FORK_TABLE));
        mvprintw(forkCoords[i].y, forkCoords[i].x, "f%2d", i);
        attroff(COLOR_PAIR(C_FORK_TABLE));
    }
    
    refresh();			/* Print it on to the real screen */
    
    while(1)
    {
        if(update != -1)
        {
//            printf("%d\n", update);
//            printPhilosofers();
//            printForks();
//            printf("\n");

            for(i = 0; i < N_PHILOSOFERS; ++i)
            {
                if (philStatus[i] == THINKING)
                {
                    attron(COLOR_PAIR(C_THINKING));
                    mvprintw(philCoords[i].status.y, philCoords[i].status.x, "thinking");
                    attroff(COLOR_PAIR(C_THINKING));
                }
                else
                {
                    attron(COLOR_PAIR(C_EATING));
                    mvprintw(philCoords[i].status.y, philCoords[i].status.x, "eating  ");
                    attroff(COLOR_PAIR(C_EATING));
                }
                
                gettimeofday(&currentTime, NULL);
                
                if(forks[i] == -1)
                {
                    attron(COLOR_PAIR(C_FORK_TABLE));
                    mvprintw(forkCoords[i].y, forkCoords[i].x, "f%2d", i);
                    mvprintw(forkCoords[i].y+1, forkCoords[i].x, "  ", i);
                    attroff(COLOR_PAIR(C_FORK_TABLE));
                }
                else
                {
                    attron(COLOR_PAIR(C_FORK_NOT_TABLE));
                    mvprintw(forkCoords[i].y, forkCoords[i].x, "f%2d", i);
                    attroff(COLOR_PAIR(C_FORK_NOT_TABLE));
                    mvprintw(forkCoords[i].y+1, forkCoords[i].x, "p%2d", forks[i]);
                }
            }
    
            refresh();			/* Print it on to the real screen */
            update = -1;
        }
        if(imout == N_PHILOSOFERS)
        {
            break;
        }
    }
    
    getch();
    
    endwin();			/* End curses mode		  */
}

int main()
{
    int i;
    pthread_mutex_init(&mtx_timeLog, NULL);
    
    pthread_t thread_id[N_PHILOSOFERS];
    pthread_t thread_updater;
    
    for (i = 0; i < N_PHILOSOFERS; i++) {
        forks[i] = -1;
        phil[i] = i;
    }
    
    pthread_create(&thread_updater, NULL,
                   printUpdate, NULL);
    
    //for (i = N_PHILOSOFERS - 1; i >= 0; --i) {
    for (i = 0; i < N_PHILOSOFERS; ++i)
    {
        // create philosopher processes
        pthread_create(&thread_id[i], NULL,
                philospher, &phil[i]);
    }
    
    update = 0;
    
    for (i = 0; i < N_PHILOSOFERS; i++)
    {
        pthread_join(thread_id[i], NULL);
    
        //mvprintw(8, 0, "");
        //printf("philosofer %2d stopped\n", i);
    
    }
    
    pthread_join(thread_updater, NULL);
    
    pthread_mutex_destroy(&mtx_timeLog);
    
    printf("starvation!\n");
}
