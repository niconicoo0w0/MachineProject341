/**
 * mini_memcheck
 * CS 341 - Spring 2023
 */
#include <stdio.h>
#include <stdlib.h>

int main() {
    // char *x = malloc(3);
    // x[0] = 'a';
    // x[1] = 'b';
    // x[2] = '\0';
    // fprintf(stderr, "%s\n", x);
    // x = realloc(x, 10);
    // // char *z = realloc(x, 2000 * sizeof(char));
    // // char *e = realloc(NULL, 10 * sizeof(char));
    // // free(z);
    // fprintf(stderr, "%s\n", x);
    // free(x);
    int *a = malloc(10 * sizeof(int));
    for (int i = 0; i < 10; ++i) {
        a[i] = i;
    }
    a = realloc(a, 20);
    for (int i = 0; i < 10; ++i) {
        if (a[i] != i) {
            fprintf(stderr, "%s\n", "failed!!");
        }
    }
    return 0;
}