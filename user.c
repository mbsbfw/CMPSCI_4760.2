#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <signal.h>

#define SHMKEY 314159
#define BUFF_SZ sizeof(2) 

void addSeconds(long* myClock);

int main(int argc, char **argv){

	long duration;
	duration = atol(argv[1]);

	int shmid = shmget ( SHMKEY, BUFF_SZ, 0775);

	if (shmid == -1){

		perror("Child - ERROR in shmget");
		exit(1);
	}

	/*attach the given shared memory segment, at a free position
	 that will be allocated by the system*/
	long * myClock = (long *)(shmat(shmid, 0, 0 ));
		
	int i;

	//read clock and add duration to it
	for(i = 0; i < duration; i++){
		myClock[1] = myClock[1] + 1;

		addSeconds(myClock);
	}

	fprintf(stdout, "PID: %d - clock: [%d][%d] - Child TERMINATING\n", getpid(), myClock[0], myClock[1]);
	
	shmdt(myClock);	
	exit(EXIT_SUCCESS);	
}//end main

void addSeconds(long * myClock){
	long leftOvers;
	long remainder;

	if(myClock[1] > 999999999){
		leftOvers = myClock[1]/1000000000;
		myClock[0] = myClock[0] + leftOvers;
		remainder = myClock[1] % 1000000000;
		myClock[1] = remainder;
	}
}


