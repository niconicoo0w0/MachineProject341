/**
 * password_cracker
 * CS 341 - Spring 2023
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include "crypt.h"
#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>

typedef struct thread_info {
    char username[9];
    char hash[14];
    char known[9];
    char result[9];
    size_t status;
    long range;
} thread_info;

static size_t done = 0;
static int total_num = 0;
static int num_thread = 0;

static thread_info* tinfo = NULL;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_barrier_t bar;

void* crack_password(void* thread_id) {
    int result = 0;
    
    struct crypt_data cdata;
    cdata.initialized = 0;
    char* known_password = NULL;

    while (1) {
        pthread_barrier_wait(&bar);
        if (done) {
            break;
        }
        long start_pos = 0;
        int num = 0;
        result = 0;
        known_password = strdup(tinfo->known);

        int len = getPrefixLength(known_password);

        getSubrange(strlen(known_password) - len, num_thread, (size_t)thread_id, &start_pos, &tinfo->range); 
        setStringPosition(known_password + len, start_pos);

        v2_print_thread_start((size_t)thread_id, tinfo->username, start_pos, known_password);
        
        for(size_t i = 0; i < (size_t)tinfo->range; i++) {
            char* current_hash = crypt_r(known_password, "xx", &cdata);
            num++;
            if (!strcmp(current_hash, tinfo->hash)) {
                pthread_mutex_lock(&lock);
                strcpy(tinfo->result, known_password);
                tinfo->status = 1;
                v2_print_thread_result((size_t)thread_id, num, result);
                total_num += num; 
                pthread_mutex_unlock(&lock);
                break;
            }
            pthread_mutex_lock(&lock);
            if (tinfo->status) {
                result = 1;
                v2_print_thread_result((size_t)thread_id, num, result);
                total_num += num;
                pthread_mutex_unlock(&lock);
                break;
            }
            pthread_mutex_unlock(&lock);
            incrementString(known_password + len);
        } 
        if (!tinfo->status) {
            result = 2;
            pthread_mutex_lock(&lock);
            v2_print_thread_result((size_t)thread_id, num, result);
            total_num += num;
            pthread_mutex_unlock(&lock);
        }
        pthread_barrier_wait(&bar); 
    }
    free(known_password);
    return NULL;
}

int start(size_t thread_count) {

    double start_time = 0;
    double start_cpu = 0;

    pthread_t lst[thread_count];
    pthread_barrier_init(&bar, NULL, thread_count + 1);

    for(size_t i = 0; i < thread_count; i++) {
        num_thread++;
        pthread_create(lst + i, NULL, crack_password, (void*) i + 1);
    }
    tinfo = calloc(sizeof(thread_info), 1);

    size_t length = 0;
    char* line = NULL;
    while (getline(&line, &length, stdin) != -1) {
        sscanf(line, "%s %s %s", tinfo->username, tinfo->hash, tinfo->known);  
        v2_print_start_user(tinfo->username);

        start_time = getTime();
        start_cpu = getCPUTime();

        pthread_barrier_wait(&bar);
        pthread_barrier_wait(&bar);

        v2_print_summary(tinfo->username, tinfo->result, total_num, getTime() - start_time, getCPUTime() - start_cpu, !tinfo->status);
        tinfo->status = total_num = 0;
    }

    done = 1;
    pthread_barrier_wait(&bar);
    for (size_t j = 0; j < thread_count; j++) {
        pthread_join(*(lst + j), NULL);
    }

    if (line) { free(line); }
    if (tinfo) { free(tinfo); }
    pthread_mutex_destroy(&lock);
    pthread_barrier_destroy(&bar);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}