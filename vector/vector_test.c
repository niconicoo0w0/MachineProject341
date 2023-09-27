/**
 * vector
 * CS 341 - Spring 2023
 */
#include "vector.h"
int main() {
    // Write your test cases here
    vector *v = vector_create(string_copy_constructor, string_destructor, string_default_constructor);
    vector_resize(v, 20);
    vector_resize(v, 13);
    vector_insert(v, 0, "elem7");
    vector_resize(v, 13);
    vector_insert(v, 0, "elem7");
    vector_resize(v, 1);
    vector_insert(v, 0, "28");
    vector_resize(v, 0);
    vector_insert(v, 3, "67");
    vector_resize(v, 13);
    vector_insert(v, 2, "21");
    vector_resize(v, 13);
    vector_insert(v, vector_size(v), "23");
    vector_resize(v, 17);
    vector_destroy(v);
    return 0;
}
