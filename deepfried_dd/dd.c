/**
 * partner: ruizhou3
 * deepfried_dd
 * CS 341 - Spring 2023
 */

#include "format.h"
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <time.h>

#define DEFAULT_BLK_SIZE 512

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

static size_t full_blocks_in = 0;
static size_t partial_blocks_in = 0;
static size_t total_bytes_copied = 0;

static struct timespec start_time;
static struct timespec end_time;

void signal_handler(int sig) {
    if (sig == SIGUSR1) {
        // live status report
        clock_gettime(CLOCK_MONOTONIC, &end_time);
        double time_elapsed = (end_time.tv_nsec - start_time.tv_nsec) / 1000000000 + end_time.tv_sec - start_time.tv_sec;
        print_status_report(full_blocks_in, partial_blocks_in, full_blocks_in, partial_blocks_in, total_bytes_copied, time_elapsed);
        fflush(stdout);
        return;
    }
    return;
}

int main(int argc, char **argv) {
    signal(SIGUSR1, signal_handler);

    FILE* fin = stdin;
    FILE* fout = stdout;

    size_t skipin = 0;
    size_t skipout = 0;
    size_t num_copy = 0;
    size_t blk_size = DEFAULT_BLK_SIZE;

    int opt = 0;
    while((opt = getopt(argc, argv, "i:o:b:c:p:k:")) != -1) {
        switch(opt) {
            case 'i':
                fin = fopen(optarg, "r");
                if (fin) {
                    break;
                } else {
                    print_invalid_input(optarg);
                    exit(1);
                }
            case 'o':
                // You should create this file if does not already exist.
                fout = fopen(optarg, "w+");
                if (fout) {
                    break;
                } else {
                    print_invalid_output(optarg);
                    exit(1);
                }
            case 'b':
                if (optarg) {
                    blk_size = atoi(optarg);
                }
                if (blk_size < 0) {
                    print_invalid_input(optarg);
                    exit(1);
                }
                break;
            case 'c':
                if (optarg) {
                    num_copy = atoi(optarg);
                }
                if (num_copy < 0) {
                    print_invalid_input(optarg);
                    exit(1);
                }
                break;
            case 'p':
                if (optarg) {
                    skipin = atoi(optarg);
                }
                if (skipin < 0) {
                    print_invalid_input(optarg);
                    exit(1);
                }
                break;
            case 'k':
                if (optarg) {
                    skipout = atoi(optarg);
                }
                if (skipout < 0) {
                    print_invalid_input(optarg);
                    exit(1);
                }
                break;
            default:
                exit(1);
        }
    }

    if (fin != stdin) {
        fseek(fin, skipin * blk_size, SEEK_SET);
    }
    if (fout != stdout) {
        fseek(fout, skipout * blk_size, SEEK_SET);
    }

    while (!feof(fin)) {
        if (num_copy) {
            size_t total_block = partial_blocks_in + full_blocks_in;
            if (total_block == num_copy) {
                break;
            }
        }
        char buffer[blk_size];
        size_t num_read = fread(buffer, sizeof(char), blk_size, fin);
        if (num_read == 0) {
            break;
        }
        size_t min = MIN(num_read, blk_size);
        if (min == blk_size) {
            fflush(stdin);
            fwrite(buffer, min, sizeof(char), fout);
            full_blocks_in++;
            total_bytes_copied += min;
        } else {
            partial_blocks_in++;
            total_bytes_copied += min;
            fwrite(buffer, min, sizeof(char), fout);
        }
    }
    // print report
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    double time_elapsed = (end_time.tv_nsec - start_time.tv_nsec) / 1000000000 + end_time.tv_sec - start_time.tv_sec;

    print_status_report(full_blocks_in, partial_blocks_in, full_blocks_in, partial_blocks_in, total_bytes_copied, time_elapsed);
    return 0;
}