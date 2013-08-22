//
//  main.c
//  MonteCarloPI
//
//  Created by Alcides Fonseca on 9/9/11.
//  Modified by Bruno Cabral on 10/09/2011
//  Copyright 2011 SO 2011/2012 - DEI - FTCUC All rights reserved.
//  Modified by Alexandre Rui Santos Fonseca Pinto && Carlos Miguel Rosa Avim

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <signal.h>

#define RESOLUTION 100000000
#define PROCESS 4

pid_t pids[PROCESS];

static void handler_pause(int);
static void handler_sigusr1(int);
static void handler_c(int);
void setupRand();
float randint();
int isInsideCircle(float x,float y);
double generate();


int main (int argc, const char * argv[])
{
	int sum=0,j;
	int fd[PROCESS][2];
    double c, c_aux = 0, pi;
    long i;
	fd_set readset;
    
	signal(SIGQUIT,SIG_IGN);
	
	// Create processes and pipes
	for(i=0;i<PROCESS;i++) {			

		if(pipe(fd[i])==-1) {
			perror("Failed to create pipe");
			return 1;
    	}
   		if((pids[i]=fork())==-1) {					//save the id of children
   			perror("Failed to fork");
   			return 1;
		}
   		if(pids[i]==0) {
			signal(SIGINT,handler_pause);
			signal(SIGUSR1,handler_sigusr1);
   			c=generate(RESOLUTION/PROCESS);
   			
   			if(write(fd[i][1], &c, sizeof(double))<0)
				perror("Failed to write to pipe");
			
			exit(0);
   		}
   	}

	signal(SIGINT, handler_c);
	
	FD_ZERO(&readset);									//setup the set of ready fd's
	
		
	
	while(sum<PROCESS)									//while exists some fd's to read'
	{
		close(fd[sum][1]);	
		
		FD_SET(fd[sum][0], &readset);			
		
		if ( select(fd[PROCESS-1][0]+1, &readset, NULL, NULL, NULL) > 0 ) {		//when someone is ready
			for(j=0;j<PROCESS;j++)
			{
				if (FD_ISSET(fd[j][0], &readset))								//which one is ready
				{
					if(read(fd[j][0], &c_aux, sizeof(double)) > 0)				//read it
					{
						c+=c_aux;
						sum++;												//count, one more was read
						break;
					}
				}
			}
		}																	//wait for your childrens
	}

    wait(NULL);

    // calculate the number of PI based on the number of points
    // that fall in the circle
    pi = 4 * c/RESOLUTION;

    // show the result
    printf("PI is: %f\n", pi);

    return 0;
}

// signal handler SIGINT
static void handler_pause(int signum) {
	pause();
}

// signal handler SIGUSR1
static void handler_sigusr1(int signum){
	signal(SIGUSR1,handler_sigusr1);
}

// signal handler
static void handler_c(int signum) {

	char option[2];
	int i;
	sigset_t sigset;

	if( signal(SIGINT, handler_c) == SIG_ERR) {
			perror("Error handling SIGINT");
			exit(1);
	}

	sigemptyset(&sigset);
	sigaddset(&sigset, SIGINT);

	if (sigprocmask(SIG_BLOCK, &sigset, NULL) == -1) {
		perror("Error in sigprocmask");
		exit(1);
	}

	printf("^C pressed. Do you want to abort? [y/n] ");
	scanf("%1s", option);

	if (option[0] == 'y'){
		if(sigprocmask(SIG_UNBLOCK,&sigset,NULL) == -1) {
			perror("Error in sigprocmask");
			exit(1);
		}
		for(i=0;i<PROCESS;i++)
			kill(pids[i],SIGKILL);

		exit(0);
	}

	for(i=0;i<PROCESS;i++)
		kill(pids[i],SIGUSR1);

	if(sigprocmask(SIG_UNBLOCK,&sigset,NULL) == -1) {
		perror("Error in sigprocmask");
		exit(1);
	}
}

//generate the exact number (RESOLUTION) of random coordinates 
// and count how many are inside the circle

double generate(int n) {

	// seed the random number generator

	double c = 0;
    long i;

    setupRand();
		
	for (i=0; i<n; i++) {
        	float x = randint();
        	float y = randint();
        	c += isInsideCircle(x, y);
    	}
    return c;
}

// check if a coordinate is below or above the circle line
int isInsideCircle(float x,float y) {
    return (sqrt( pow(x,2) + pow(y,2)) <= 1) ? 1 : 0;
}

// seed the random numbers generator
void setupRand() {
    time_t seconds;
    time(&seconds);
    srand(((unsigned int) seconds) + getpid());
}

// set a limit for the random number generator
float randint() {
    return rand() / (float) RAND_MAX;
}
