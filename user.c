/*-------------------------------------------------------
File: user.c

Name:   Bradley Rose
Student Number: 8746930

Description: This program is designed to test the severs
             program using pipes.
             Please follow the instructions in the comments

Note that all functionality (INCLUDING STEPS 8-10) was done within user.c as I was unable to call router.c successfully using execlp(...)
--------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/fcntl.h>
#include <signal.h>
#include <string.h>

// Utilities

// Time Information Structure
typedef struct TIME_INFO
{
    int hour;
    int minute;
    int second;
}TimeInfo;

// This function would close the pipe
void closePipe(int *pipe)
{
    close(pipe[0]);
    close(pipe[1]);
}

// This function will get current time and store it in TimeInfo struct
TimeInfo getTimeInfo()
{
    time_t now;
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
    
    TimeInfo timeInfo = {tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec};
    return timeInfo;
}

// This function is used for print the time information storing in the TimeInfo
void printTimeInfo(TimeInfo timeInfo)
{
    printf("%d@%d:%d:%d\n", getpid(), timeInfo.hour, timeInfo.minute, timeInfo.second);
}

// This function is to print the current time
void printTime()
{
    TimeInfo timeInfo = getTimeInfo();
    printTimeInfo(timeInfo);
}

// read function in block mode, same usage as read(int fd, const void *buf, size_t count)
int blockRead(int fd, const void *buf, size_t count)
{
    int readNum = 0;
    for(;readNum < count;)
    {
        char data = 0;
        int retVal = read(fd, &data, sizeof(data));
        if(retVal > 0)
        {
            readNum += sizeof(data);
            *(char *)buf = data;
            (char *)buf++;
        }
    }
    return readNum;
}

//Thread function
void * threadFunction(void * args){
    printTime();
    pthread_exit(NULL);
}

// define your parameters here
int queryTimesMax = 2;
int queryInterval = 2;
int serverPrintTimesMax = 2;
int serverPrintInterval = 2;

#define QueryCommand 't' // Used for Querying Time
#define QuitCommand 'q'
#define Read_End 0
#define Write_End 1

/*---------------------------------------------------------------
Function: main

Description: Complete the functions listed in the comments.
             Each function is 10 points.

Assignment: Complete this function to ensure proper calls to 
            the other functions.
---------------------------------------------------------------*/
int main(int argc, char *argv[])
{
    // You may liek to use the utility functions provided above //
    pthread_t thread;
    char buffer1[2], buffer2[12];
    char *QuerryT = "t";
    char *QuerryQ = "q";
    char timeString[9];
    char *program = "router";

    // Create pipes
	int userToRouter[2], routerToUser[2], routerToServer[2], serverToRouter[2];

    if(pipe(userToRouter) == -1){ /*handle error*/
        fprintf(stderr, "userToRouter pipe failed");
        exit(-1);
    }
    if(pipe(routerToUser) == -1){ /*handle error*/
        fprintf(stderr, "routerToUser pipe failed");
        exit(-1);
    }
    if(pipe(routerToServer) == -1){ /*handle error*/
        fprintf(stderr, "routerToUser pipe failed");
        exit(-1);
    }
    if(pipe(serverToRouter) == -1){ /*handle error*/
        fprintf(stderr, "routerToUser pipe failed");
        exit(-1);
    }
    
    // Create router process
	pid_t pid_router = fork();
    if(pid_router<0){ /*handle error*/
        printf("Create router process fork failed \n");
        exit(-1);
    }else if(pid_router == 0) { /*This is the router child process*/
        printf("Router PID:   %d\n", getpid());

        // 8. Router connects with User and Server via pipes
        close(userToRouter[Write_End]);
        close(routerToServer[Read_End]);
        close(serverToRouter[Write_End]);
        close(routerToUser[Read_End]);
        //execlp(&program, &program, userToRouter[Read_End], routerToUser[Write_End], routerToServer[Write_End], serverToRouter[Read_End]);
        
        // 9. Router receives any data from User and forwards the data to the Server
        //reading from user
        int j;
        //Querry time twice
        for(j=0; j<2; j++){
            wait();
            blockRead(userToRouter[Read_End], buffer1, sizeof(buffer1));
            
            //writing to the server process
            write(routerToServer[Write_End], buffer1, 2);
            sleep(2);
            printf("%d@User has sent 1 byte to Server\n", getpid());

            // 10. Router receives any data from Server and forwards the data to the User
            //reading from server process
            wait();
            blockRead(serverToRouter[Read_End], buffer2, sizeof(buffer2));

            //writing to user process
            printf("%d@Server has sent 12 bytes to User\n", getpid());
            write(routerToUser[Write_End], buffer2, 12); //router sending time info to user
            sleep(2);
        }
        //receiving QuerryQ, server will not send anything back so don't read
        wait();
        blockRead(userToRouter[Read_End], buffer1, sizeof(buffer1));
        
        //writing to the server process
        write(routerToServer[Write_End], buffer1, 2);
        sleep(2);
        printf("%d@User has sent 1 byte to Server\n", getpid());
        wait(0);
        exit(0);

    }else{
        //Create Server process
        pid_t pid_server;
        pid_server = fork();
        if(pid_server<0){ /*handle error*/
            printf("Create server process fork failed \n");
            kill(pid_router);
            exit(-1);
        }else if(pid_server == 0) { /*This is the server child process*/
            printf("Server PID:   %d\n", getpid());
            // 1. Server process should create a thread to display current time for at least 2 times
            pthread_create(&thread,  NULL, &threadFunction, NULL);
            pthread_join(thread, NULL); /*Main program waits for other thread to execute and finish before continuing*/

            close(routerToServer[Write_End]);
            close(serverToRouter[Read_End]);
            
            //Reading from router
            // int k;
        // for(k=0; k<2; k++){
            //for loop then doesn't allow user to send to server?????
            wait();
            blockRead(routerToServer[Read_End], buffer1, sizeof(buffer1));
            if(strcmp(buffer1, QuerryT) == 0){
                //received QuerryT from server, respond with time
                write(serverToRouter[Write_End], getTimeInfo, 12); //server sending time info to router
            }else{
                //QuerryQ received
                printf("Server Quits\n");
                exit(0);
            }
        //}
        }else{
            // Create User process
            printf("User PID:   %d \n", getpid());    
            close(userToRouter[Read_End]);
            close(routerToUser[Write_End]);
            // 4. User queries time for at least 2 times by sending 't' command to Router

            int i;
            for(i=0; i<2; i++){
                write(userToRouter[Write_End], QuerryT, 2); //user sending the 't' command to router
                //reading from router
                wait();
                blockRead(routerToUser[Read_End], buffer2, sizeof(buffer2));
                printTime(buffer2); // user printing the time info received from user
            }
            // 5. Then User sends 'q' command to Router
            write(userToRouter[Write_End], QuerryQ, 2); //user sending the 't' command to router
            
            // 6. User waits for the Server ending itself
            int *serverEnded;
            waitpid(pid_server, &serverEnded, 0);
            
            //7. User kills the Router process
            if(&serverEnded == 0){
                kill(pid_router, SIGTERM);
                printf("Router has been Killed");
                closePipe(userToRouter);
                closePipe(routerToUser);
                closePipe(routerToServer);
                closePipe(serverToRouter);
                exit(0);
            }
            exit(0); 
        }

    }
    return 0;
}
