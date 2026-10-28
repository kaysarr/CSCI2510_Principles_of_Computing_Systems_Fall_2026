#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    char* child_argv[] = {"ls", "-l", NULL};
    char* child_prog = child_argv[0];

    pid_t child_PID = fork();

    if (child_PID == -1) {
        perror("fork failed");
        return -1;
    }

    if (child_PID == 0) {
        execvp(child_prog, child_argv);
        perror("execvp failed");
        return -1;
    } else {
        waitpid(child_PID, NULL, 0);
        printf("Parent: my PID is %d\n", getpid());
    }
    return 0;
}