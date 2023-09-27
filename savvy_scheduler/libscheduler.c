/**
 * partner: ruizhou3
 * savvy_scheduler
 * CS 341 - Spring 2023
 */
#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    int id;

    /* TODO: Add any other information and bookkeeping you need into this
     * struct. */
     double the_time_arrive;
     double time_remain;
     double run_time;
     double require_time;
     double time_start;
     int priority;
} job_info;

static int total_number_of_process;
static double the_time_wait;
static double the_time_response;
static double the_time_turn;


void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here

    total_number_of_process = 0;
    the_time_wait = 0;
    the_time_response = 0;
    the_time_turn = 0;
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int compare_helper(const void *a, const void *b, double c, double d, int if_break_tie) {
    if (if_break_tie) {
        if (c == d) {
            return break_tie(a, b);
        } 
    }
    if (MIN(d - c, 0) != 0) {
        return 1;
    }
    return -1;
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_info_a = ((job *)a)->metadata;
    job_info *job_info_b = ((job *)b)->metadata;
    return compare_helper(a, b, job_info_a->the_time_arrive, job_info_b->the_time_arrive, 0);
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_info_a = ((job *)a)->metadata;
    job_info *job_info_b = ((job *)b)->metadata;
    return compare_helper(a, b, job_info_a->priority, job_info_b->priority, 1);
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_info_a = ((job *)a)->metadata;
    job_info *job_info_b = ((job *)b)->metadata;
    return compare_helper(a, b, job_info_a->time_remain, job_info_b->time_remain, 1);
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_info_a = ((job*)a)->metadata;
    job_info *job_info_b = ((job*)b)->metadata;
    return compare_helper(a, b, job_info_a->run_time, job_info_b->run_time, 1);
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info *job_info_a = ((job*)a)->metadata;
    job_info *job_info_b = ((job*)b)->metadata;
    return compare_helper(a, b, job_info_a->require_time, job_info_b->require_time, 1);
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    job_info *info = calloc(1, sizeof(job_info));
    info->id = job_number;
    info->time_start = -1;
    info->time_remain = sched_data->running_time + info->require_time - info->require_time;
    info->run_time = -1;
    info->require_time = sched_data->running_time;
    info->priority = sched_data->priority;
    info->the_time_arrive = time;
    newjob->metadata = info;
    priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    if (!job_evicted) {
        return priqueue_peek(&pqueue);
    }

    job_info* info = job_evicted->metadata;
    info->run_time = time;
    info->time_remain -= 1;
    if (MAX(info->time_start, 0) == 0) {
        info->time_start = time -1;
    }
    if (pqueue_scheme == PPRI) {
        job* curr = priqueue_poll(&pqueue);
        priqueue_offer(&pqueue, curr);
        return priqueue_peek(&pqueue);
    } else if (pqueue_scheme == PSRTF) {
        job* curr = priqueue_poll(&pqueue);
        priqueue_offer(&pqueue, curr);
        return priqueue_peek(&pqueue);
    } else if (pqueue_scheme == RR) {
        job* curr = priqueue_poll(&pqueue);
        priqueue_offer(&pqueue, curr);
        return priqueue_peek(&pqueue);
    }
    return job_evicted;
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
    total_number_of_process += 1;
    job_info* for_job_in_this = job_done->metadata;
    the_time_wait = the_time_wait + time - for_job_in_this->the_time_arrive - for_job_in_this->require_time + for_job_in_this->require_time - for_job_in_this->require_time;
    the_time_response = the_time_response + for_job_in_this->time_start - for_job_in_this->the_time_arrive + for_job_in_this->the_time_arrive - for_job_in_this->the_time_arrive;
    the_time_turn = the_time_turn + time - for_job_in_this->the_time_arrive - time + time;
    free(for_job_in_this);
    priqueue_poll(&pqueue);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    // return 9001;
    return the_time_wait/total_number_of_process;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    // return 9001;
    return the_time_turn/total_number_of_process;
}

double scheduler_average_response_time() {
    // TODO: Implement me!
    // return 9001;
    return the_time_response/total_number_of_process;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}