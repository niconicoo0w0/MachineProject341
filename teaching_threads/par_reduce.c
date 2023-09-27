/**
 * teaching_threads
 * CS 341 - Spring 2023
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"

/* You might need a struct for each task ... */
typedef struct {
    int *list;
    size_t length;
    reducer reducer_func;
    int base_case;
    int returnit;
} thread_data;


/* You should create a start routine for your threads. */

void *par_reduce_helper(void *arg) {
    thread_data *data = (thread_data*)arg;
    int returnit = data->base_case;
    for (size_t i = 0; i < data->length; i++) {
        returnit = data->reducer_func(returnit, data->list[i]);
    }
    data->returnit = returnit;
    pthread_exit(NULL);
}

int par_reduce(int *list, size_t length, reducer reducer_func, int base_case, size_t num_threads) {
    // Only one thread or empty list
    if (num_threads < 2 || length == 0) {
        return reduce(list, length, reducer_func, base_case);
    }

    size_t one = (length + num_threads - 1) / num_threads;
    size_t two = (length + one - 1) / one;

    pthread_t *thread = (pthread_t *)malloc(two * sizeof(pthread_t));
    thread_data *data = (thread_data *)malloc(two * sizeof(thread_data));

    for (size_t i = 0; i < two; i++) {
        size_t start = i * one;
        size_t end = (i + 1) * one;

        if (end > length) {
            end = length;
        }

        data[i].list = list + start;
        data[i].length = end - start;
        data[i].reducer_func = reducer_func;
        data[i].base_case = base_case;

        pthread_create(&thread[i], NULL, par_reduce_helper, (void *)&data[i]);
    }

    int returnit = base_case;
    for (size_t i = 0; i < two; i++) {
        pthread_join(thread[i], NULL);
        returnit = reducer_func(returnit, data[i].returnit);
    }

    free(thread);
    free(data);

    return returnit;
}
