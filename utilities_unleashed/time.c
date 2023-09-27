/**
 * utilities_unleashed
 * CS 341 - Spring 2023
 */

#include"format.h"
#include<unistd.h>
#include<time.h>

#include<string.h>
#include<stdlib.h>

#include<sys/wait.h>
#include<sys/types.h>


int main(int argc, char *argv[]) { 
	if (argc < 2){
		print_time_usage();
	}
    struct timespec start;
	clock_gettime(CLOCK_MONOTONIC, &start);

	char **temp = malloc(sizeof(char **) * argc);
	for(int i = 1; i < argc; i++){
		temp[i-1] = malloc(1 + strlen(argv[i]));
		strcpy(temp[i-1],argv[i]);
	}
	free(temp[argc-1]);
	temp[argc-1] = NULL;
	
	pid_t pid = fork();
	if (pid == -1) {
		print_fork_failed();
	} else if(pid == 0) {
        execvp(argv[1], argv+1);
		print_exec_failed();
	} else {
		int status;
	    waitpid(pid, &status, 0);
		if (status != 0){
			return 1;
		}
        struct timespec end;
		clock_gettime(CLOCK_MONOTONIC, &end);
		display_results(argv, (end.tv_nsec - start.tv_nsec)/1e9 + (end.tv_sec - start.tv_sec));
	}
	return 0;
}