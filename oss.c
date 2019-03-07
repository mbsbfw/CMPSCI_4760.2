#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/time.h>
#include <signal.h>
#include <errno.h>
#include <stdbool.h>

#define SHMKEY 314159
#define BUFF_SZ sizeof(10)
 
void addSeconds(long*, int);
void cleanShareMem(); 
static void myhandler(int s);
static int setupinterrupt(void);
static int setupitimer(void);

int main(int argc, char **argv){

	if(setupinterrupt () == -1){

	perror("Failed to set up handler for SIGPROF");
	return 1;
	}//end if

	if (setupitimer() == -1){

		perror("Failed to set up the ITIMER_PROF interval timer");
		return 1;
	}

	int option;
	int status;
	int t = 0;
	int timeInc;
	int forksRunning = 0;
	int totalForks = 0;
	int maxTotalForks = 4;
	int numOfForks = 2;

	char * input = malloc(32 * sizeof(char) + 1);
	char * output = malloc(32 * sizeof(char) + 1);

	strcpy(input, "input.txt");
	strcpy(output, "output.txt");

	while((option = getopt(argc, argv, "hn:s:i:o:")) != -1){

		switch(option){

			case 'h' :
				printf("\t\t---Help Menu---\n");
				printf("\t-h Display Help Menu\n");
				printf("\t-n x indicate the maximum total of child processes\n");
				printf("\t-s x indicate how many children exist at same time\n");
				printf("\t-i Specify input file name\n");
				printf("\t-o Specify output file name\n");
				exit(1);
			
			case 'n' :
				maxTotalForks = atoi(optarg);
				if(maxTotalForks > 20)
					maxTotalForks = 19;
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
	long * myClock = (long *)(paddr);

	myClock[0] = 0;
	myClock[1] = 0;
	
	//read input file
	char readLine[1000];
	
	//read line character by character
	fgetc(fPointer);
	fgets(readLine, 1000, fPointer);
	
	//strtok() usage "tutorialspoint"
	char s[100] = " ";
	char* token;
	char* tokens[3];

	//get first token
	token = strtok(readLine, s);
	
	//walk through other tokens
	while(token != NULL){	  
		tokens[t] = token;
		token = strtok(NULL, s);
		t++;
	}//end token while

	long childStartSec = strtol(tokens[0], NULL, 10);
	long childStartNano = strtol(tokens[1], NULL, 10);
	long childStop = strtol(tokens[2], NULL, 10);

	FILE* fp;

	int i = 0;
	pid_t user_pid;

	//infinite loop
	while(true){

		addSeconds(myClock, timeInc);
	
		//create outputfile if it doesnt exist
		//if exist apend to it 
		fp = fopen(output, "a+");

		char arg1[100];
		snprintf(arg1, 100, "%d", childStop);
	
		if(myClock[0] >= childStartSec && 
				  myClock[1] >= childStartNano){
			if(totalForks < maxTotalForks && forksRunning < numOfForks){
				
				user_pid = 0;		
				user_pid = fork();

				fprintf(fp, "PID: %d ", user_pid);
				fprintf(fp, "timeLaunched: [%d] [%d] ", myClock[0], myClock[1]);
				fprintf(fp, "duration: %d \n", childStop);
					
				if(user_pid == 0){
					execlp("./user", "user", arg1, NULL);
					printf("exec worked? or NOT\n");
				}//end if

				else{
					fclose(fp);
					totalForks++;
					forksRunning++;		

					//fgetc(fPointer);							
					fgets(readLine, 100, fPointer);
	
					//get first token
					token = strtok(readLine, s);
					tokens[0] = token;	
					//walk through other tokens
					while(token != NULL){
						tokens[i] = token;	  
						token = strtok(NULL, s);
						i++;
					}//end token while
					i = 0;
									
					childStartSec = strtol(tokens[0], NULL, 10);
					childStartNano = strtol(tokens[1], NULL, 10);
					childStop = strtol(tokens[2], NULL, 10);
					/*printf("%d\n", childStartSec);	
					printf("%d\n", childStartNano);	
					printf("%d\n", childStop);*/
				}//end else
			}//end if	
		}//end if

		if((user_pid = waitpid(0, &status, WNOHANG)) > 0){
			if(WIFEXITED(status)){
				fp = fopen(output, "a+");
				forksRunning--;
				fprintf(fp, "PID:%d Terminated at:[%d][%d] \n", user_pid, myClock[0], myClock[1]);
				fclose(fp);
			}
		}
	}//end while
	
	cleanShareMem();		
	return 0;	
}//end main

void cleanShareMem(){
	
	int shmid = shmget (SHMKEY, BUFF_SZ, 0775 | IPC_CREAT);

	shmctl(shmid,IPC_RMID,NULL);	
}//end cleanShare

void addSeconds(long* myClock, int timeInc){
	//long leftOvers;
	//long remainder;
	int p;
	for(p = 0; p < timeInc; p++){
		myClock[1] = myClock[1] + 1;
		if(myClock[1] > 999999999){
			myClock[0] = myClock[0] + 1;
			myClock[1] = 0; 
		}
	}

	//printf("myClock: [%d][%d]", myClock[0], myClock[1]);
}

static void myhandler(int s){

	char aster = '*';
	int errsave;
	errsave = errno;
	write(STDERR_FILENO, &aster, 1);
	printf("Terminate after 2 REAL life seconds\n");
	//printf("clock:[%d][%d]", myClock[0], myClock[1]);
	exit(1);
	errno = errsave;
}//end myhandler

static int setupinterrupt(void){

	struct sigaction act;
	act.sa_handler = myhandler;
	act.sa_flags = 0;
	return(sigemptyset(&act.sa_mask) || sigaction(SIGPROF, &act, NULL));
}//end setupinterrupt

static int setupitimer(void){

	struct itimerval value;
	value.it_interval.tv_sec  = 2;
	value.it_interval.tv_usec  = 0;
	value.it_value  = value.it_interval;	
	return (setitimer(ITIMER_PROF, &value, NULL));
}//end setupitimer

