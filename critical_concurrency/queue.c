/**
 * critical_concurrency
 * CS 341 - Spring 2023
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue *q = malloc(sizeof(queue));
    if (q == NULL) {
        return NULL;
    }

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->max_size = max_size;

    if (pthread_cond_init(&(q->cv), NULL) != 0) {
        free(q);
        return NULL;
    }

    if (pthread_mutex_init(&(q->m), NULL) != 0) {
        pthread_cond_destroy(&(q->cv));
        free(q);
        return NULL;
    }

    return q;
}

void queue_destroy(queue *this) {
    /* Your code here */
    while (this->head != NULL) {
        queue_node *tmp = this->head;
        this->head = tmp->next;
        free(tmp);
    }

    pthread_cond_destroy(&(this->cv));
    pthread_mutex_destroy(&(this->m));
    free(this);
}

void queue_push(queue *this, void *data) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));
    while (this->max_size > 0 && this->size >= this->max_size) {
        pthread_cond_wait(&(this->cv), &(this->m));
    }

    queue_node *node = malloc(sizeof(queue_node));
    node->data = data;
    node->next = NULL;

    if (this->head == NULL) {
        this->head = node;
        this->tail = node;
    } else {
        this->tail->next = node;
        this->tail = node;
    }

    ++(this->size);

    pthread_cond_signal(&(this->cv));
    pthread_mutex_unlock(&(this->m));
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));
    while (this->size == 0) {
        pthread_cond_wait(&(this->cv), &(this->m));
    }

    queue_node *node = this->head;
    void *element = node->data;

    this->head = node->next;
    if (this->head == NULL) {
        this->tail = NULL;
    }

    --(this->size);

    pthread_cond_signal(&(this->cv));
    pthread_mutex_unlock(&(this->m));

    free(node);
    return element;

}
