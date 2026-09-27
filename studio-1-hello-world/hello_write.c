// Kay
// 9/27/2026
// Prints hello using write()
#include <unistd.h>

int main(int argc, char* argv[]) {
    write(STDOUT_FILENO, "Hello, world!\n", 14);
    return 0;
}
