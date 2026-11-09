//SLUSH - The SLU SHELL
//
//This assignment reads user input, and executes that input as a sequence of
//programs to execute.
//
//Program execution commands have the form:
//
//prog_n [args] ( ... prog_3 [args] ( prog_2 [args] ( prog_1 [args]
//
//For example, "grep slush ( sort -r ( ls -l -a" returns a reverse sorted list
//of all files in the current directory that contain the string "slush" in
//
//See the lab writeup for more details and other requirements.

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAX_LINE 256
#define MAX_ARGS 15
#define MAX_CMDS 64

volatile sig_atomic_t got_sigint = 0;

void handle_sigint(int sig) {
    got_sigint = 1;
    write(STDOUT_FILENO, "\n", 1);
}

void print_prompt() {
    char cwd[1024];
    char* home = getenv("HOME");

    if (getcwd(cwd, sizeof(cwd)) == NULL) {
        printf("slush> ");
    } else if (home != NULL && strncmp(cwd, home, strlen(home)) == 0 &&
               (cwd[strlen(home)] == '\0' || cwd[strlen(home)] == '/')) {
        char* rest = cwd + strlen(home);
        if (*rest == '/') {
            rest++;
        }
        printf("slush|%s> ", rest);
    } else {
        printf("slush|%s> ", cwd);
    }
    fflush(stdout);
}

int split_commands(char* line, char* cmds[]) {
    int count = 0;
    char* start = line;

    while (1) {
        char* paren = strchr(start, '(');
        if (paren != NULL) {
            *paren = '\0';
        }
        if (count >= MAX_CMDS) {
            return -1;
        }
        cmds[count++] = start;
        if (paren == NULL) {
            break;
        }
        start = paren + 1;
    }
    return count;
}

int tokenize(char* cmd, char* argv[]) {
    int argc = 0;
    char* tok = strtok(cmd, " \t");

    while (tok != NULL) {
        if (argc >= MAX_ARGS + 1) {
            return -1;
        }
        argv[argc++] = tok;
        tok = strtok(NULL, " \t");
    }
    argv[argc] = NULL;
    return argc;
}

void run_pipeline(char* cmd_argv[][MAX_ARGS + 2], int n) {
    pid_t pids[MAX_CMDS];
    int num_children = 0;
    int prev_read = -1;

    for (int i = n - 1; i >= 0; i--) {
        int fd[2] = {-1, -1};

        if (i > 0 && pipe(fd) == -1) {
            perror("pipe");
            break;
        }

        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            if (fd[0] != -1) { close(fd[0]); close(fd[1]); }
            break;
        }

        if (pid == 0) {
            signal(SIGINT, SIG_DFL);

            if (prev_read != -1) {
                if (dup2(prev_read, STDIN_FILENO) == -1) { perror("dup2"); exit(-1); }
                close(prev_read);
            }
            if (i > 0) {
                close(fd[0]);
                if (dup2(fd[1], STDOUT_FILENO) == -1) { perror("dup2"); exit(-1); }
                close(fd[1]);
            }

            execvp(cmd_argv[i][0], cmd_argv[i]);
            fprintf(stderr, "%s: Not found\n", cmd_argv[i][0]);
            exit(-1);
        }

        pids[num_children++] = pid;

        if (prev_read != -1) {
            close(prev_read);
        }
        if (i > 0) {
            close(fd[1]);
            prev_read = fd[0];
        } else {
            prev_read = -1;
        }
    }

    if (prev_read != -1) {
        close(prev_read);
    }

    for (int i = 0; i < num_children; i++) {
        while (waitpid(pids[i], NULL, 0) == -1) {
            if (errno != EINTR) {
                perror("waitpid");
                break;
            }
        }
    }
}

int main() {
    char line[MAX_LINE + 2];
    char* cmds[MAX_CMDS];
    char* cmd_argv[MAX_CMDS][MAX_ARGS + 2];

    struct sigaction sa;
    memset(&sa, 0, sizeof(sa));
    sa.sa_handler = handle_sigint;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = 0;
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("sigaction");
        exit(-1);
    }

    while (1) {
        got_sigint = 0;
        print_prompt();

        errno = 0;
        if (fgets(line, sizeof(line), stdin) == NULL) {
            if (got_sigint || errno == EINTR) {
                clearerr(stdin);
                continue;
            }
            printf("\n");
            break;
        }

        if (strchr(line, '\n') == NULL && !feof(stdin)) {
            int c;
            while ((c = getchar()) != '\n' && c != EOF);
            fprintf(stderr, "Input line too long (max %d characters)\n", MAX_LINE);
            continue;
        }
        line[strcspn(line, "\n")] = '\0';

        if (strspn(line, " \t") == strlen(line)) {
            continue;
        }

        int n = split_commands(line, cmds);
        if (n == -1) {
            fprintf(stderr, "Too many commands in pipeline\n");
            continue;
        }

        int valid = 1;
        for (int i = 0; i < n; i++) {
            int argc = tokenize(cmds[i], cmd_argv[i]);
            if (argc == 0) {
                fprintf(stderr, "Invalid null command\n");
                valid = 0;
                break;
            }
            if (argc == -1) {
                fprintf(stderr, "Too many arguments (max %d per command)\n", MAX_ARGS);
                valid = 0;
                break;
            }
        }
        if (!valid) {
            continue;
        }

        if (strcmp(cmd_argv[0][0], "cd") == 0) {
            if (n > 1) {
                fprintf(stderr, "cd must appear on a line by itself\n");
                continue;
            }
            char* dir = cmd_argv[0][1];
            if (dir == NULL) {
                dir = getenv("HOME");
            }
            if (dir == NULL || chdir(dir) == -1) {
                perror("cd");
            }
            continue;
        }

        run_pipeline(cmd_argv, n);
    }

    return 0;
}