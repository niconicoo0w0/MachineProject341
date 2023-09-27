/**
 * password_cracker
 * CS 341 - Spring 2023
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "./includes/queue.h"
#include "crypt.h"
#include <stdio.h>
#include <string.h>
#include <unistd.h>

typedef struct thread_info {
    char username[9];
    char hash[14];
    char known[9];
} thread_info;


static queue* q = NULL;
static int total_num = 0;
static int recovered = 0;
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

int find_word(char* password_hash, char* known_password, char* to_compare) {
    if (!password_hash || !known_password) {
        return -1;
    }
    if (!strcmp(to_compare, password_hash)) {
        pthread_mutex_lock(&lock);
        recovered++;
        pthread_mutex_unlock(&lock);
        return 0;
    }
    return 1;
}

void* cracker_helper(void* thread_id) {
    char* curr_task = NULL;

    struct crypt_data cdata;
    cdata.initialized = 0;

    while (1) {
        curr_task = queue_pull(q);
        //EOF
        if (!curr_task) {
            break;
        }
        // 1 failed, 0 success
        int result = 1;
        // start
        struct thread_info tinfo;
        sscanf(curr_task, "%s %s %s", tinfo.username, tinfo.hash, tinfo.known);
        v1_print_thread_start((size_t)thread_id, tinfo.username);
        int num_letter = getPrefixLength(tinfo.known);

        long pos = 0;
        setStringPosition(tinfo.known + num_letter, pos);
        int hash_count = 0;
        double start_time = getThreadCPUTime();
        // found
        while (1) {
            hash_count++;
            result = find_word(tinfo.hash, tinfo.known, crypt_r(tinfo.known, "xx", &cdata));
            if (result == -1) {
                return NULL;
            }
            if (result == 0) {
                break;
            }
            int flag = incrementString(tinfo.known + num_letter);
            if (!flag) {
                break;
            }
        }
        double end_time = getThreadCPUTime();
        v1_print_thread_result((size_t)thread_id, tinfo.username, tinfo.known, hash_count, end_time - start_time, result);
        // curr_task = queue_pull(q);
        if (curr_task) {
            free(curr_task);
            curr_task = NULL;
        }
    }
    // if (curr_task) {
    //     free(curr_task);
    //     curr_task = NULL;
    // }
    // free(user_name);
    // free(password_hash);
    // free(known_password);
    return NULL;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    q = queue_create(-1);
    pthread_t lst[thread_count];
    
    size_t length = 0;
    char* line = NULL;
    
    while (getline(&line, &length, stdin) != -1) { 
        queue_push(q, strdup(line));
        total_num++;
    }
    for (size_t i = 0; i < thread_count; i++) {
        queue_push(q, NULL);
    }
    for (size_t j = 0; j < thread_count; j++) {
        pthread_create(lst + j, NULL, cracker_helper, (void*)j + 1);
    }
    for (size_t k = 0; k < thread_count; k++) {
        pthread_join(*(lst + k), NULL);
    }
    int failed = total_num - recovered;
    v1_print_summary(recovered, failed);

    //destroy
    if (line) {
        free(line);
        line = NULL;
    }
    queue_destroy(q);
    pthread_mutex_destroy(&lock);
    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}

