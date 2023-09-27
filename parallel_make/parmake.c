/**
 * parallel_make
 * CS 341 - Spring 2023
 */

#include "format.h"
#include "parmake.h"
#include "parser.h"

#include "vector.h"
#include "dictionary.h"
#include "set.h"
#include "graph.h"

#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>

typedef enum {
    TARGET_ERROR = -1,
    TARGET_CYCLE = 0,
    TARGET_READY = 1,
    TARGET_SKIPPED = 2,
    TARGET_DONE = 3,
} target_state;

static set* set_ = NULL;
static graph *dependency_graph = NULL;
static vector *rules = NULL;

pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cv = PTHREAD_COND_INITIALIZER;

// Checks if the graph contains a cycle
int is_Cycle(void* target) {
    if (!set_) {
        set_ = shallow_set_create();
    }
    if (set_contains(set_, target)) {
        set_destroy(set_);
        set_ = NULL;
        return 1;
    }
    set_add(set_, target);
    vector* vec = graph_neighbors(dependency_graph, target);
    for (size_t i = 0; i < vector_size(vec); i++) {
        if (is_Cycle(vector_get(vec, i))) {
            vector_destroy(vec);
            return 1;
        }
    }
    vector_destroy(vec);
    set_destroy(set_);
    set_ = NULL;
    return 0;
}

void help_get_rules(vector *goals, dictionary *counter) {
    for (size_t i = 0; i < vector_size(goals); i++) {
        void *goal = vector_get(goals, i);
        vector *neighbors = graph_neighbors(dependency_graph, goal);
        help_get_rules(neighbors, counter);
        if (*((int*)dictionary_get(counter, goal)) == 0) {
            vector_push_back(rules, goal);
            int one = 1;
            dictionary_set(counter, goal, &one);
        }
        if (neighbors) {
            vector_destroy(neighbors);
        }
    }
}

target_state get_status(vector* neighbors, void* goal) {
    if (access(goal, 0) != TARGET_ERROR) {
        for (size_t i = 0; i < vector_size(neighbors); ++i) {
            struct stat status;
            if (stat(goal, &status) == TARGET_ERROR) {
                vector_destroy(neighbors);
                return TARGET_ERROR;
            }
        }
        vector_destroy(neighbors);
        return TARGET_SKIPPED;
    }
    for (size_t i = 0; i < vector_size(neighbors); ++i) {
        rule_t *temp = graph_get_vertex_value(dependency_graph, vector_get(neighbors, i));
        if (temp->state == TARGET_DONE) {
            continue;
        } else {
            vector_destroy(neighbors);
            return temp->state;
        }
    }
    vector_destroy(neighbors);
    return TARGET_READY;
}


void *process_rules(void *data) {
    while (1) {
        pthread_mutex_lock(&lock);
        if (!rules || !vector_size(rules)) {
            pthread_mutex_unlock(&lock);
            break;
        }
        for (size_t i = 0; i < vector_size(rules); i++) {
            void *goal = vector_get(rules, i);
            rule_t *rule = graph_get_vertex_value(dependency_graph, goal);
            vector *neighbors = graph_neighbors(dependency_graph, goal);
            if (rule->state == TARGET_DONE) {
                vector_erase(rules, i);
                pthread_mutex_unlock(&lock);
                break;
            }
            target_state status = get_status(neighbors, goal);
            if (status == TARGET_ERROR) {
                vector_erase(rules, i);
                pthread_mutex_unlock(&lock);
                pthread_cond_broadcast(&cv);
                break;
            }
            if (status == TARGET_READY) {
                vector_erase(rules, i);
                pthread_mutex_unlock(&lock);
                vector *cmd = rule->commands;
                int flag = 0;
                for (size_t i = 0; i < vector_size(cmd); i++) {
                    if (system(vector_get(cmd, i))) {
                        flag = 1;
                        break;
                    }
                }
                if (flag) {
                    rule->state = TARGET_ERROR;
                } else {
                    rule->state = TARGET_DONE;
                }
                pthread_cond_broadcast(&cv);
                break;
            }
            if (status == TARGET_SKIPPED) {
                vector_erase(rules, i);
                pthread_mutex_unlock(&lock);
                rule->state = TARGET_READY;
                pthread_cond_broadcast(&cv);
                break;
            }
            // finished
            if (i + 1 == vector_size(rules)) {
                pthread_cond_wait(&cv, &lock);
                pthread_mutex_unlock(&lock);
                break;
            }
        }
    }
    return data;
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    rules = shallow_vector_create();
    pthread_t threads[num_threads]; 

    dependency_graph = parser_parse_makefile(makefile, targets);
    vector *neighbors= graph_neighbors(dependency_graph, "");
    for (size_t idx = 0; idx < vector_size(neighbors); idx++) {
        void *curr = vector_get(neighbors, idx);
        if (is_Cycle(curr)) {
            print_cycle_failure((char*) curr);
            rule_t *curr_rule = graph_get_vertex_value(dependency_graph, curr);
            curr_rule->state = TARGET_CYCLE;
            // graph_destroy(dependency_graph);
            // vector_destroy(neighbors);
            vector_erase(neighbors, idx);
        }
    }
    if (!vector_size(neighbors)) {
        vector_destroy(rules);
        vector_destroy(neighbors);
        if (dependency_graph) { graph_destroy(dependency_graph); }
        return 0;
    }
    dictionary *check = string_to_int_dictionary_create();
    vector *vertices = graph_vertices(dependency_graph);
    for (size_t i = 0; i < vector_size(vertices); ++i) {
        int value = 0;
        dictionary_set(check, vector_get(vertices, i), &value);
    }
    help_get_rules(neighbors, check);
    vector_destroy(vertices);
    if (check) {
        dictionary_destroy(check);
    }
    for (size_t j = 0; j < num_threads; j++) {
        pthread_create(&threads[j], NULL, process_rules, NULL);
    }
    for (size_t k = 0; k < num_threads; k++) {
        pthread_join(threads[k], NULL);
    }
    // destroy 
    vector_destroy(rules);
    vector_destroy(neighbors);
    if (dependency_graph) { graph_destroy(dependency_graph); }
    return 0;
}