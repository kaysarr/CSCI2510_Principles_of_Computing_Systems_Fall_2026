//This program brute-forces a given password hash by trying all possible
//passwords of a given length.
//
//Usage:
//crack <threads> <keysize> <target>
//
//Where <threads> is the number of threads to use, <keysize> is the maximum
//password length to search, and <target> is the target password hash.
//
//For example:
//
//./crack 1 5 na3C5487Wz4zw
//
//Should return the password 'apple'

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <crypt.h>
#include <pthread.h>

#define ALPHA 26

char salt[3];
char target[14];
int  num_threads;
volatile int found = 0;

typedef struct {
    long start;
    long end;
    int  length;
} thread_arg;

void index_to_password(long index, int length, char *out) {
    for (int i = length - 1; i >= 0; i--) {
        out[i] = 'a' + (index % ALPHA);
        index /= ALPHA;
    }
    out[length] = '\0';
}

void *worker(void *arg) {
    thread_arg *t = (thread_arg *)arg;

    struct crypt_data data;
    data.initialized = 0;

    char guess[9];

    for (long i = t->start; i < t->end; i++) {
        if (found) return NULL;

        index_to_password(i, t->length, guess);
        char *hash = crypt_r(guess, salt, &data);

        if (strcmp(hash, target) == 0) {
            found = 1;
            printf("%s\n", guess);
            exit(0);
        }
    }
    return NULL;
}

void crack_length(int length) {
    long total = 1;
    for (int i = 0; i < length; i++) total *= ALPHA;

    pthread_t threads[num_threads];
    thread_arg args[num_threads];
    long chunk = total / num_threads;

    for (int i = 0; i < num_threads; i++) {
        args[i].start  = i * chunk;
        args[i].end    = (i == num_threads - 1) ? total : (i + 1) * chunk;
        args[i].length = length;
        pthread_create(&threads[i], NULL, worker, &args[i]);
    }
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
}

int main( int argc, char* argv[] ){

    if (argc != 4) {
        fprintf(stderr, "Usage: %s <threads> <keysize> <target>\n", argv[0]);
        return 1;
    }

    num_threads = atoi(argv[1]);
    int keysize = atoi(argv[2]);
    strncpy(target, argv[3], sizeof(target) - 1);

    salt[0] = target[0];
    salt[1] = target[1];
    salt[2] = '\0';

    for (int len = 1; len <= keysize; len++) {
        crack_length(len);
    }

    return 0;
}