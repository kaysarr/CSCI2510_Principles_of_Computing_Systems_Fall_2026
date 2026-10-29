#include <stdio.h>
#include <string.h>
#include <unistd.h>

int main() {
    char input[256];
    int max_args = 15;
    int max_argv_size = max_args + 2;
    char* cmd;
    char* my_argv[max_argv_size];

    printf("Enter a command: ");
    if (fgets(input, sizeof(input), stdin) == NULL) {
        perror("fgets failed");
        return -1;
    }

    input[strcspn(input, "\n")] = '\0';

    cmd = strtok(input, " ");
    if (cmd == NULL) {
        printf("No command entered\n");
        return -1;
    }

    my_argv[0] = cmd;
    int i = 1;
    char* res = strtok(NULL, " ");
    while (res != NULL && i <= max_args) {
        my_argv[i] = res;
        i++;
        res = strtok(NULL, " ");
    }
    my_argv[i] = NULL;

    execvp(cmd, my_argv);
    perror("execvp failed");
    return -1;
}