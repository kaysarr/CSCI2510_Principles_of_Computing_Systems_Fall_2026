#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/wait.h>

void handle_sigint( int signum ){
	printf("Caller is ignoring SIGINT (signal %d)\n", signum);
}

int main( int argc, char* argv[] ){

	pid_t ret;

	signal( SIGINT, handle_sigint );

	printf("Forking sleeper...\n");

	ret = fork();
	if( ret == -1 ){
		perror("Could not fork");
		exit(-1);
	}

	if( ret == 0 ){
		int exec_ret;
		char* cmd = "./sleep";
		char* myargv[] = {"sleep", NULL};
		exec_ret = execvp( cmd, myargv );
		if( exec_ret == -1 ){
			perror("Error calling execvp");
			exit(-1);
		}
	}

	printf("Waiting for sleeper %d...\n", ret);
	waitpid( ret, NULL, 0 );

	printf("Parent finished waiting and returned successfully!\n");

	return 0;
}