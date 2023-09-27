/**
 * vector
 * CS 341 - Spring 2023
 */
#include "sstring.h"
#include "vector.h"

#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif

#include <assert.h>
#include <string.h>

struct sstring {
    // Anything you want
    char *c;
};

sstring *cstr_to_sstring(const char *input) {
    // your code goes here
    assert(input);
    struct sstring *returnit = malloc(sizeof(sstring)); //size of char *vec
    returnit->c = malloc(sizeof(char) * (strlen(input) + 1));
    strcpy(returnit->c, input);
    return returnit;
}

char *sstring_to_cstr(sstring *input) {
    // your code goes here
    assert(input);
    char *returnit = strdup(input->c);
    return returnit;
}

int sstring_append(sstring *this, sstring *addition) {
    // your code goes here
    int len = strlen(this->c) + strlen(addition->c);
    this->c = realloc(this->c, len + 1);
    strcat(this->c, addition->c);       //strcat for concatenate
    return strlen(this->c);
}

vector *sstring_split(sstring *this, char delimiter) {
    // your code goes here
    vector *returnit = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    int num = 0;
    int len = strlen(this->c);
    for (int i = 0; i <= len; i++) {
        if (this->c[i] == delimiter || i == len) {
            char *segment = (char *)malloc(i - num + 1);
            strncpy(segment, this->c + num, i - num);
            segment[i - num] = '\0';
            vector_push_back(returnit, segment);
            num = i + 1;
        }
    }
    return returnit;
}

int sstring_substitute(sstring *this, size_t offset, char *target, char *substitution) {
    assert(substitution);
    assert(this);
    assert(target);

    char *position = strstr(this->c + offset, target);

    if (!position) {
        return -1;          // Check if target was found in this->c
    }

    size_t target_len = strlen(target);
    size_t substitution_len = strlen(substitution);
    size_t len = strlen(this->c) + substitution_len - target_len;
    this->c = realloc(this->c, sizeof(char)*(len + 1));
    memmove(position + substitution_len, position + target_len, strlen(position + target_len) + 1);
    memcpy(position, substitution, substitution_len);
    
    return 0;
}

char *sstring_slice(sstring *this, int start, int end) {
    // your code goes here
    assert(this);
    int length = end - start;
    char* returnit = malloc(length + 1);
    strncpy(returnit, this->c + start, length);
    returnit[length] = '\0';
    return returnit;
}

void sstring_destroy(sstring *this) {
    // your code goes here
    free(this->c);
    free(this);
}
