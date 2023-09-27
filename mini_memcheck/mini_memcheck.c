/**
 * mini_memcheck
 * CS 341 - Spring 2023
 */

#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

size_t total_memory_requested = 0;
size_t total_memory_freed = 0;
size_t invalid_addresses = 0;
meta_data *head = NULL;

void *mini_malloc(size_t request_size, const char *filename, void *instruction) {
    if (request_size == 0) { 
        return NULL;
    }
    meta_data *tmp = (meta_data*)malloc(sizeof(meta_data) + request_size);
    if (!tmp) {
        return NULL;
    }
    tmp->request_size = request_size;
    tmp->filename = filename;
    tmp->instruction = instruction;
    tmp->next = head;
    head = tmp;
    total_memory_requested += request_size;
    return (void*)(tmp + 1);
}

void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    if (num_elements == 0 || element_size == 0) return NULL;
    size_t request_size = num_elements * element_size;
    meta_data *tmp = (meta_data*)malloc(sizeof(meta_data) + request_size);
    if (!tmp) {
        return NULL;
    }
    memset((void*)(tmp + 1), 0, request_size);
    tmp->request_size = request_size;
    tmp->filename = filename;
    tmp->instruction = instruction;
    tmp->next = head;
    head = tmp;
    total_memory_requested += request_size;
    return (void*)(tmp + 1);
}

void *mini_realloc(void *ptr, size_t request_size,
                   const char *filename, void *instruction) {
    if (!ptr) { return mini_malloc(request_size, filename, instruction); }

    if (request_size == 0) {
        mini_free(ptr);
        return NULL;
    }
    meta_data *old = head;
    meta_data *prev = NULL;

    while (old) {
        void *mem = (void*)(old + 1);
        if (mem == ptr) {
            break;
        }
        prev = old;
        old = old->next;
    }
    if (!old) {
        invalid_addresses++;
        return NULL;
    }
    meta_data* new = (meta_data*)realloc(old, sizeof(meta_data) + request_size);
    if (!new) { return NULL; }


    //connect prev with new
    if (!prev) {
        head = new;
    } else {
        prev->next = new;
    }
    if (new->request_size < request_size) {
        total_memory_requested += request_size - new->request_size;
    }
    if (new->request_size > request_size) {
        total_memory_freed += new->request_size - request_size;
    }

    new->request_size = request_size;
    new->filename = filename;
    new->instruction = instruction;
    
    return (void*)(new + 1);
}


void mini_free(void *ptr) {
    if (!ptr) return;
    meta_data *tmp = (meta_data*)ptr - 1;
    meta_data *prev = NULL;
    meta_data *current = head;
    while (current) {
        if (current == tmp) {
            if (prev) {
                prev->next = current->next;
            } else {
                head = current->next;
            }
            total_memory_freed += current->request_size;
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
    invalid_addresses++;
}
