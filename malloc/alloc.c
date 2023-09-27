/**
 * malloc
 * CS 341 - Spring 2023
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define min(a,b) ((a) < (b) ? (a) : (b))

typedef struct _meta_data {
    void *ptr;
    size_t size;
    char free;
    struct _meta_data *next;
    struct _meta_data *prev;
} meta_data;

static meta_data *head = NULL;
static int total_free_blocks = 0;

void *calloc(size_t num, size_t size) {
    void* ptr = malloc(num * size);
    if (!ptr) {
        return NULL;
    }
    memset(ptr, 0, num * size);
    return ptr;
}

meta_data *set_newblk(meta_data* ptr, size_t size) {
    meta_data *returnit = (void*)ptr + sizeof(meta_data) + size;
    returnit->free = '1';
    returnit->next = ptr;
    returnit->prev = ptr->prev;
    returnit->ptr = (void*)returnit + sizeof(meta_data);
    returnit->size = ptr->size - size - sizeof(meta_data);
    return returnit;
}

meta_data *help_merge(meta_data* first, meta_data* second, meta_data* third, meta_data* current, size_t size) {
    meta_data *chosen = NULL;
    if (current->free == '1' && size < first->size + current->size + sizeof(meta_data)) {
        current->size += first->size + sizeof(meta_data);
        if (!second) {
            head = current;
        } else {
            second->next = current; 
            second->prev = current->prev; 
        }
        total_free_blocks--;
        total_free_blocks--;
        chosen = current;
        return chosen;
    }
    if (second && second->free == '1' && current->free == '1' && first->size + second->size + current->size + 2 * sizeof(meta_data) >= size) {
        current->size += first->size + second->size + 2 * sizeof(meta_data);
        if (!third) {
            head = current;
        } else {
            third->next = current; 
            third->prev = current->prev; 
        }
        total_free_blocks--;
        total_free_blocks--;
        total_free_blocks--;
        chosen = current;
        return chosen;
    }
    return NULL;
}

meta_data *find_fit(size_t size) {
    if (total_free_blocks == 0) { return NULL; }
    if (!head) { return NULL; }
    if (size == 0) { return NULL; }

    meta_data *current = head;
    meta_data *chosen = NULL;
    meta_data *first = NULL;
    meta_data *second = NULL;
    meta_data *third = NULL;

    while (current) {
        if (current->free == '1' && current->size >= size) {
            if (current->size > size + sizeof(meta_data)) {
                meta_data *new_block = set_newblk(current, size);
                if (current == head){
                    head = new_block;
                } else {
                    meta_data *temp = (meta_data *)(((void*)current) + sizeof(meta_data) + current->size);
                    temp->next = new_block;
                    new_block->prev = temp;
                }
                current->size = size;
                total_free_blocks++;
                chosen = current;
                return chosen;
            } else {
                total_free_blocks--;
                chosen = current;
                return chosen;
            }
        }
        if (first && first->free == '1') {
            chosen = help_merge(first, second, third, current, size);
        }
        //found!!
        if (chosen) { 
            return chosen; 
        } else {
            third = second;
            second = first;
            first = current;
            current = current->next;
        }
    }
    return NULL;
}

void* split_helper(size_t size, meta_data *current) {
    meta_data* new_block = current->ptr + size;
    new_block->free = '1';
    new_block->size = current->size - size - sizeof(meta_data);
    new_block->ptr = (void*)(new_block+1);

    new_block->next = current->next;
    new_block->prev = current;

    current->free = '0';
    current->size = size;

    current->next = new_block;
    if (new_block->next) {
        new_block->next->prev = new_block;
    }

    total_free_blocks--;
    return current->ptr;
}

void *malloc(size_t size) {
    if (size == 0) {
        return NULL;
    }
    meta_data *chosen = find_fit(size);
    if (chosen) {
        chosen->free = '0';
        if ((chosen->size-size) > sizeof(meta_data) && (chosen->size-size) > 2 * size) {
            return split_helper(size, chosen);
        }
        return chosen->ptr;
    }
    chosen = sbrk(sizeof(meta_data));
    chosen->ptr = sbrk(0);
    if (sbrk(size) == (void*)-1) {
        return NULL;
    }
    chosen->size = size;
    chosen->free = '0';

    chosen->next = head;
    chosen->prev = NULL;
    head = chosen;
    return chosen->ptr;
}

void free(void *ptr) {
    if (!head) {
        return;
    }
    if (!ptr) {
        return;
    }
    if (ptr >= sbrk(0)) {
        return;
    }
    meta_data *temp = ((meta_data*)ptr) - 1;
    if (temp->free == '1') {
        return;
    }
    temp->free = '1';
    // Merge
    if (temp->next && temp->next->free == 1) {
        temp->size += temp->next->size + sizeof(meta_data);
        temp->next = temp->next->next;
        total_free_blocks++;
        return;
    }
    total_free_blocks++;
    return;
}

void *realloc(void *ptr, size_t size) {
    if (!ptr) {
        return malloc(size);
    }
    if (size == 0) {
        free(ptr);
        return NULL;
    }
    meta_data *tmp = ((meta_data*)ptr) - 1;
    if (size < tmp->size) {
        return ptr;
    }
    void* returnit = malloc(size);
    if (!returnit) {
        return NULL;
    }
    memcpy(returnit, ptr, min(size, tmp->size));
    free(ptr);
    return returnit;
}