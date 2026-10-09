#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>

#define bufferSize 200

char buffer[bufferSize];

int main(int argc, char *argv[]) {
    int fd = open(argv[1], O_RDONLY);

    if (fd == -1) {
        perror("Error opening file");
        return -1;
    }

    while (1) {
        ssize_t bytesRead = read(fd, buffer, bufferSize);
        if (bytesRead == 0) {
            break;
        }
        write(STDOUT_FILENO, buffer, bytesRead);
    }

    close(fd);
    return 0;
}