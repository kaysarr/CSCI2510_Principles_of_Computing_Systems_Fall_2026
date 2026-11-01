#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>

int main() {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe failed");
        return -1;
    }

    pid_t child1 = fork();
    if (child1 == -1) {
        perror("fork failed");
        return -1;
    }
    if (child1 == 0) {
        close(fd[0]);
        dup2(fd[1], STDOUT_FILENO);
        close(fd[1]);
        char* cmd = "./print";
        char* myargv[] = {"print", NULL};
        execvp(cmd, myargv);
        perror("Error exec'ing print");
        return -1;
    }

    pid_t child2 = fork();
    if (child2 == -1) {
        perror("fork failed");
        return -1;
    }
    if (child2 == 0) {
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        char* cmd = "./program2";
        char* myargv[] = {"program2", NULL};
        execvp(cmd, myargv);
        perror("Error exec'ing program2");
        return -1;
    }

    close(fd[0]);
    close(fd[1]);
    waitpid(child1, NULL, 0);
    waitpid(child2, NULL, 0);
    return 0;
}