#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>
#include <stdnoreturn.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include "colors.h"
#ifdef WITH_NCURSES
    #include <curses.h>
#endif
#include <signal.h>
#include <math.h>

#define N_PHILOSOFERS 9
#define N_FORKS N_PHILOSOFERS
#define EATING 1
#define THINKING 0
#define LEFT_FORK(philosoferID) philosoferID
#define RIGHT_FORK(philosoferID) ((philosoferID+1) % N_PHILOSOFERS)

#define N_TIMELOG 10000
#define MAX_WHAT_LENGTH 100

int state[N_PHILOSOFERS];
int phil[N_PHILOSOFERS];
int philStatus[N_PHILOSOFERS];
int forks[N_FORKS];

pthread_mutex_t forkMtx[N_FORKS];

//int update = 0;

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

coord forkCoords[N_FORKS+1];
philCoord philCoords[N_PHILOSOFERS];

typedef unsigned long long u64;

u64 u64useconds;
struct timeval beginingOfThinkingTimes[N_PHILOSOFERS];

static volatile int keepRunning = 1;

pthread_t thread_id[N_PHILOSOFERS];

void intHandler(int dummy) {
    keepRunning = 0;
    for (int i = 0; i < N_PHILOSOFERS; ++i)
    {
        pthread_cancel(thread_id[i]);
    }
}


char eventLog[N_PHILOSOFERS][N_TIMELOG][MAX_WHAT_LENGTH];
long int timeLog[N_PHILOSOFERS][N_TIMELOG];

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

void* philospher(void* num)
{
    int myPhilosoferID = *((int*) num);
    int myLeftForkID = LEFT_FORK(myPhilosoferID);
    int myRightForkID = RIGHT_FORK(myPhilosoferID);
    char what[MAX_WHAT_LENGTH];
    struct timeval currentTime;
    //printf("p %2d lf %2d rf %2d\n", myPhilosoferID, myLeftForkID, myRightForkID);
    
    philStatus[myPhilosoferID] = THINKING;
    
    int i=0;
    
    usleep(1000000);
    
    //usleep(1000000*(random() % 20)/10.0);
    
    gettimeofday(&(beginingOfThinkingTimes[myPhilosoferID]), NULL);
    
    int eventLogIndex = 0;
    int internalKeepRunning = 1;
    
    while (keepRunning && internalKeepRunning) {
        
        do
        {
            sprintf(what, "p%d try L f%d", myPhilosoferID, myLeftForkID);
            internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
            if(!internalKeepRunning) pthread_exit(0);
        } while( pthread_mutex_trylock(&(forkMtx[myLeftForkID])) != 0);

        forks[myLeftForkID] = myPhilosoferID;
        
        sprintf(what, "p%d got L f%d", myPhilosoferID, myLeftForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
        
//        update = myPhilosoferID;
        if(!internalKeepRunning) pthread_exit(0);

        do
        {
            sprintf(what, "p%d try R f%d", myPhilosoferID, myRightForkID);
            internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
            if(!internalKeepRunning)
            {
                pthread_mutex_unlock(&(forkMtx[myRightForkID]));
                pthread_exit(0);
            }
        } while( pthread_mutex_lock(&(forkMtx[myRightForkID])) != 0);

        forks[myRightForkID] = myPhilosoferID;
        
        sprintf(what, "p%d got R f%d", myPhilosoferID, myRightForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
//        update = myPhilosoferID;
        if(!internalKeepRunning)
        {
            pthread_mutex_unlock(&(forkMtx[myRightForkID]));
            pthread_mutex_unlock(&(forkMtx[myLeftForkID]));
            pthread_exit(0);
        }

        philStatus[myPhilosoferID] = EATING;
        
        sprintf(what, "p%d eating", myPhilosoferID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
//        update = myPhilosoferID;
        if(!internalKeepRunning)
        {
            //TODO: print imout on file
            forks[myRightForkID] = -1;
            pthread_mutex_unlock(&(forkMtx[myRightForkID]));
            forks[myLeftForkID] = -1;
            pthread_mutex_unlock(&(forkMtx[myLeftForkID]));
            pthread_exit(0);
        }
        usleep(2000000);
    
    
        philStatus[myPhilosoferID] = THINKING;
        sprintf(what, "p%d thinking", myPhilosoferID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
//        update = myPhilosoferID;
        if(!internalKeepRunning)
        {
            forks[myRightForkID] = -1;
            pthread_mutex_unlock(&(forkMtx[myRightForkID]));
            forks[myLeftForkID] = -1;
            pthread_mutex_unlock(&(forkMtx[myLeftForkID]));
            pthread_exit(0);
        }

        forks[myLeftForkID] = -1;
        pthread_mutex_unlock(&(forkMtx[myLeftForkID]));
        
        sprintf(what, "p%d drop L f%d", myPhilosoferID, myLeftForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
//        update = myPhilosoferID;
        if(!internalKeepRunning)
        {
            forks[myRightForkID] = -1;
            pthread_mutex_unlock(&(forkMtx[myRightForkID]));
            pthread_exit(0);
        }

        forks[myRightForkID] = -1;
        pthread_mutex_unlock(&(forkMtx[myRightForkID]));
        
        sprintf(what, "p%d drop R f%d", myPhilosoferID, myRightForkID);
        internalKeepRunning = !saveToLog(&eventLogIndex, what, &currentTime, myPhilosoferID);
//        update = myPhilosoferID;
        if(!internalKeepRunning) pthread_exit(0);
        
        usleep(300000);
    }
}

#ifdef WITH_NCURSES
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
        philCoords[i].status.x = 12 * i + 4;
        
        philCoords[i].forkL.y = 2;
        philCoords[i].forkL.x = philCoords[i].status.x;
    }
    
    for (i = 0; i < N_FORKS; ++i)
    {
        forkCoords[i].x = i * 12 + 1;
        forkCoords[i].y = 3;
    }
    
    forkCoords[i].x = i * 12 + 1;
    forkCoords[i].y = 3;
    
    init_pair(C_FORK_TABLE, COLOR_BLUE, COLOR_BLACK);
    init_pair(C_FORK_NOT_TABLE, COLOR_RED, COLOR_BLACK);
    init_pair(C_EATING, COLOR_GREEN, COLOR_BLACK);
    init_pair(C_THINKING, COLOR_CYAN, COLOR_BLACK);
    
    for (int i = 0; i < N_PHILOSOFERS; ++i)
    {
        mvprintw(0, philCoords[i].status.x, "  phil %2d  ", i);
    }
    
    for(i = 0; i<N_FORKS; ++i)
    {
        attron(COLOR_PAIR(C_FORK_TABLE));
        mvprintw(forkCoords[i].y, forkCoords[i].x, "f%2d", i);
        attroff(COLOR_PAIR(C_FORK_TABLE));
    }
    
    refresh();			/* Print it on to the real screen */
    
    while(keepRunning)
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
                    mvprintw(forkCoords[i].y-1, forkCoords[i].x, "___", i);
                    attroff(COLOR_PAIR(C_FORK_TABLE));
                }
                else
                {
                    attron(COLOR_PAIR(C_FORK_NOT_TABLE));
                    mvprintw(forkCoords[i].y, forkCoords[i].x, "f%2d", i);
                    attroff(COLOR_PAIR(C_FORK_NOT_TABLE));
                    mvprintw(forkCoords[i].y-1, forkCoords[i].x, "%3d", forks[i]);
                }
            }
    
            if(forks[0] == -1)
            {
                attron(COLOR_PAIR(C_FORK_TABLE));
                mvprintw(forkCoords[N_FORKS].y, forkCoords[N_FORKS].x, "f%2d", 0);
                mvprintw(forkCoords[N_FORKS].y+1, forkCoords[N_FORKS].x, "   ");
                attroff(COLOR_PAIR(C_FORK_TABLE));
            }
            else
            {
                attron(COLOR_PAIR(C_FORK_NOT_TABLE));
                mvprintw(forkCoords[N_FORKS].y, forkCoords[N_FORKS].x, "f%2d", 0);
                attroff(COLOR_PAIR(C_FORK_NOT_TABLE));
                mvprintw(forkCoords[N_FORKS].y-1, forkCoords[N_FORKS].x, "%3d", forks[0]);
            }
            
            
    
            refresh();			/* Print it on to the real screen */
            update = -1;
        }
    }
    
    getch();
    
    endwin();			/* End curses mode		  */
}
#else
void printForks()
{
    printf("\t | ");
    
    for(int i = 0; i < N_FORKS; ++i)
    {
        if(forks[i] == -1)
            printf("  %9s", "none");
        else
            printf("  %9d", forks[i]);
    }
    
    printf("\n\t | ");
    
    for(int i = 0; i < N_FORKS; ++i)
    {
        printf("    %s %2d", "   f", i);
    }

    printf("\n");
}

void printPhilosofers()
{
    printf("\t |         ");
    for(int i = 0; i < N_PHILOSOFERS; ++i)
    {
        printf("  %4s [%2d]", "phil", i);
    }

    printf("\n\t |         ");

    for(int i = 0; i < N_PHILOSOFERS; ++i)
    {
        printf("  %s", philStatus[i] == THINKING ? BLUE("thinking ") : RED(" eating  "));
    }

    printf("\n");
}

//void* printUpdate(void* unused)
//{
//    int prints = 0;
//    while(keepRunning)
//    {
//        if (update != -1)
//        {
//            printf("%d\tp%2d\n", prints, update);
//            printPhilosofers();
//            printForks();
//            printf("\n");
//            update = -1;
//            prints++;
//        }
//    }
//}
#endif

int main()
{
    int i;
    
    signal(SIGINT, intHandler);
    
    srand(time(NULL));
    
    pthread_t thread_updater;
    
    for (i = 0; i < N_PHILOSOFERS; i++) {
        forks[i] = -1;
        phil[i] = i;
        pthread_mutex_init(&(forkMtx[i]),NULL);
    }
    
    //pthread_create(&thread_updater, NULL, printUpdate, NULL);
    
    
    //update = 0;
    
    usleep(1000000);
    
    printf("launching threads\n");
    
    //for (i = N_PHILOSOFERS - 1; i >= 0; --i) {
    for (i = 0; i < N_PHILOSOFERS; ++i)
    {
        // create philosopher processes
        pthread_create(&thread_id[i], NULL,
                philospher, &phil[i]);
    }
    
    printf("threads launched!\n");
    
    for (i = 0; i < N_PHILOSOFERS; ++i)
    {
        pthread_join(thread_id[i], NULL);
    
        //mvprintw(8, 0, "");
        //printf("philosofer %2d stopped\n", i);
    
    }
    
    printf("all thread joined.\n");
    
    //pthread_join(thread_updater, NULL);
    
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
