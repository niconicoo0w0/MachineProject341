/**
 * deadlock_demolition
 * CS 341 - Spring 2023
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>
#include <stdlib.h>
#include "queue.h"

pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;
static graph* g = NULL;
set* set_ = NULL;
// static int lock_count = 0;

//Helper functions
int is_Cycle();
int Can_Create_Cycle();
 
struct drm_t {
    pthread_mutex_t m;
};
 
drm_t* drm_init() {
    // Create a new DRM graph if one does not already exist
    pthread_mutex_lock(&m);
    if (!g) {
        g = shallow_graph_create();
    }
    drm_t* drm = malloc(sizeof(drm_t));
    if (!drm) {
        return NULL;
    }
    pthread_mutex_init(&(drm->m), NULL);
    graph_add_vertex(g, drm);
    pthread_mutex_unlock(&m);
    return drm;
}
 
int drm_post(drm_t* drm, pthread_t* thread_id) {
    pthread_mutex_lock(&m);
    if (!graph_contains_vertex(g, drm)) {
        pthread_mutex_unlock(&m);
        return 0;
    }
    if (!graph_contains_vertex(g, thread_id)) {
        pthread_mutex_unlock(&m);
        return 0;
    }
    if (!graph_adjacent(g, drm, thread_id)) {
        pthread_mutex_unlock(&m);
        return 0;
    }
    graph_remove_edge(g, drm, thread_id);
    pthread_mutex_unlock(&(drm->m));
    pthread_mutex_unlock(&m);
    return 1;
}
 
// Wait for a DRM
int drm_wait(drm_t *drm, pthread_t *thread_id) {
    int returnit = 0;
    pthread_mutex_lock(&m);
    if (!graph_contains_vertex(g, thread_id)) { graph_add_vertex(g, thread_id); }

    if (graph_adjacent(g, drm, thread_id)) {
        pthread_mutex_unlock(&m);
        return returnit;
    }
    graph_add_edge(g, thread_id, drm);
    if (is_Cycle(thread_id) != 1) {
        pthread_mutex_unlock(&m);
        pthread_mutex_lock(&(drm->m));
        pthread_mutex_lock(&m);
        graph_remove_edge(g, thread_id, drm);
        graph_add_edge(g, drm, thread_id);
        pthread_mutex_unlock(&m);
        returnit = 1;
        return returnit;
    }
    graph_remove_edge(g, thread_id, drm);
    pthread_mutex_unlock(&m);
    return returnit;
}
 
void drm_destroy(drm_t *drm) {
    if (!drm) { return; }
    pthread_mutex_destroy(&(drm->m));
    if (graph_contains_vertex(g, drm)) { graph_remove_vertex(g, drm); }
    free(drm);
    // if (g && graph_edge_count(g) == 0) { graph_destroy(g); }
}

 
// Checks if the graph contains a cycle
int is_Cycle(void* thread_id) {
    if (!set_) {
        set_ = shallow_set_create();
    }
    if (set_contains(set_, thread_id)) {
        set_destroy(set_);
        set_ = NULL;
        return 1;
    }
    set_add(set_, thread_id);
    vector* vec = graph_neighbors(g, thread_id);
    for (size_t i = 0; i < vector_size(vec); i++) {
        if (is_Cycle(vector_get(vec, i)) == 1) {
            return 1;
        }
    }
    set_destroy(set_);
    set_ = NULL;
    return 0;
}


// // Checks if adding a new node to the graph creates a cycle
// int Can_Create_Cycle(int node, vector* visited, vector* recStack) {
//     const int VISITED = 1;
//     const int NOT_VISITED = 0;
//     const int IN_RECURSION_STACK = 1;
//     const int NOT_IN_RECURSION_STACK = 0;
//     if (*((int*) vector_at(visited, node)) == NOT_VISITED) {
//         vector_set(visited, node, &VISITED);
//         vector_set(recStack, node, &IN_RECURSION_STACK);
//         vector* neighbors = graph_neighbors(g, *(vector_at(visited, node)));
//         for (int i = 0; i < (int) vector_size(neighbors); ++i) {
//             if (!(*((int*) vector_at(visited, i))) && Can_Create_Cycle(i, visited, recStack)) {
//                 return 1;
//             } else if (*((int*) vector_at(recStack, i))) {
//                 return 1;
//             }
//         }
//     }
//     vector_set(recStack, node, &NOT_IN_RECURSION_STACK);
//     return 0;
// }

