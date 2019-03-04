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
#include <stdbool.h>

#define SHMKEY 314159
#define BUFF_SZ sizeof(2) 


int main(int argc, char **argv){

	int option;
	long timeInc;
	int totalForks = 1;
	int numOfForks = 1;

	char * input = malloc(32 * sizeof(char) + 1);
	char * output = malloc(32 * sizeof(char) + 1);

	strcpy(input, "input.txt");
	strcpy(output, "output.txt");

	while((option = getopt(argc, argv, "hn:s:i:o:")) != -1){

		switch(option){

			case 'h' :
				printf("\t\t---Help Menu---\n");
				printf("\t-h Display Help Menu\n");
				printf("\t-n x indicate the maximum to tal of child processes\n");
				printf("\t-s x indicate how many children exist at same time\n");
				printf("\t-i Specify input file name\n");
				printf("\t-o Specify output file name\n");
				exit(1);
			
			case 'n' :
				totalForks = atoi(optarg);
				break;

			case 's' :
				numOfForks = atoi(optarg);
				break;

			case 'i' :
				strcpy(input, optarg);
				break;

			case 'o' :
				strcpy(output, optarg);
				break;

			case '?' :
				printf("ERROR: Improper arguments");
				break;

		}//end switch
	}//end while

	//open input file
	FILE * fPointer = fopen(input, "r");

	//if input file failed to open
	if(fPointer == NULL){
		perror("ERROR: Input file failed to open");
	}
	
	//read input file
	char readLine[100];

	//get time Increment
	fscanf(fPointer, "%i", &timeInc);

	int shmid = shmget (SHMKEY, BUFF_SZ, 0775 | IPC_CREAT);

	if (shmid == -1){

		perror("Parent - ERROR in shmget");
		exit(1);
	}

	/*attach the given shared memory segment, at a free position
	 that will be allocated by the system*/
	char * paddr = (char *)(shmat(shmid, 0, 0 ));
	int * myClock = (int *)(paddr);
	
	int second = 0;
	int nano = 0;

	myClock[0] = second;
	myClock[1] = nano;

	
	myClock[1] += timeInc;
	
	char arg1[10];
	snprintf(arg1, 10, "%d", myClock[1]);

	//printf("OSS: %d  %d\n", myClock[0], myClock[1]);
	
	int i;
	for(i = 0; i < totalForks; i++){
		pid_t user_pid = 0;
		user_pid = fork();				  
		if(user_pid == 0)
			execl("./user", "user", arg1, NULL);
			printf("exec worked? or NOT\n");
	}//end for
		
	wait(NULL);
	printf("OSS: %d  %d\n", myClock[0], myClock[1]);

	//printf( "Finished executing the parent process\n"
	//" - the child won't get here--you will only see this once\n" );

	int leftOvers;
	int remainder;

	if(myClock[1] > 999999999){
		leftOvers = myClock[1]/1000000000;
		myClock[0] = myClock[0] + leftOvers;
		remainder = myClock[1] % 1000000000;
		myClock[1] = remainder;
		printf("leftOvers: %d\n", leftOvers);
		printf("remainder: %d\n", remainder);
	}
			
	return 0;	
}//end main



