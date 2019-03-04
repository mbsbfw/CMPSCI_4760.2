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

int main(int argc, char **argv){

	int addTimeInc;
	addTimeInc = atoi(argv[1]);

	int shmid = shmget ( SHMKEY, BUFF_SZ, 0775);

	if (shmid == -1){

		perror("Child - ERROR in shmget");
		exit(1);
	}

	/*attach the given shared memory segment, at a free position
	 that will be allocated by the system*/
	int * myClock = (int *)(shmat(shmid, 0, 0 ));
	
	int i;
	for(i = 0; i < addTimeInc; i++){
		myClock[1] = myClock[1] + .1;

		if(myClock[1] == 999999999){
			myClock[0] == myClock[0] + 1;
			myClock[1] = 0;
		}
	}

	
	printf("CHILD_PID: %i - Time Increment: %d\n", getpid(), addTimeInc);
		
	return 0;	
}//end main



