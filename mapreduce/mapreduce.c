/**
 * mapreduce
 * CS 341 - Spring 2023
 * partner: ruizhou3
 */

#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>

int main(int argc, char **argv) {
    if (argc != 6) {
        return 1;
    }
    if (!argv) {
        return 1;
    }
    size_t count = strtoul(argv[5], NULL, 10);
    pid_t split_pid[count];
    pid_t map_pid[count];
    int* file[count];
    for (size_t i = 0; i < count; i++) {
        file[i] = (int*)calloc(2, sizeof(int));
        pipe(file[i]);
    }
    int reducer[2];
    pipe(reducer);
    int flag1 = O_CREAT | O_TRUNC | O_WRONLY;
    int flag2 = S_IRUSR | S_IWUSR;
    int open_file = open(argv[2], flag1, flag2);
    for (size_t i = 0; i < count; i++) {
        split_pid[i] = fork();
        if (split_pid[i] == 0) {
            close(file[i][0]);
            char temp[32];
            sprintf(temp, "%zu", i);
            dup2(file[i][1], 1);
            execl("./splitter", "./splitter", argv[1], argv[5], temp, NULL);
            exit(1);
        }
    }
    for (size_t i = 0; i < count; i++) {
        close(file[i][1]);
        map_pid[i] = fork();
        if (map_pid[i] == -1) {
            exit(1);
        }
        if (map_pid[i] == 0) {
            close(reducer[0]);
            dup2(file[i][0], 0);
            dup2(reducer[1], 1);
            execl(argv[3], argv[3], NULL);
            exit(1);
        }
    }
    close(reducer[1]);
    pid_t child = fork();
    if (child == 0) {
        dup2(reducer[0], 0);
        dup2(open_file, 1);
        execl(argv[4], argv[4], NULL);
        exit(1);
    }
    close(open_file);
    close(reducer[0]);
    int tmp;
    for (size_t i = 0; i < count; i++) {
        tmp = 0;
        waitpid(split_pid[i], &tmp, 0);
    } 
    for (size_t i = 0; i < count; i++) {
        close(file[i][0]);
        tmp = 0;
        waitpid(map_pid[i], &tmp, 0);
    }
    tmp = 0;
    waitpid(child, &tmp, 0);
    if (tmp) {
        print_nonzero_exit_status(argv[4], tmp);
    }
    print_num_lines(argv[2]);
    for (size_t i = 0; i < count; i++) {
        free(file[i]);
    }
    return 0;
}
