/**
 * vector
 * CS 341 - Spring 2023
 */
#include "vector.h"
#include <assert.h>
#include <stdio.h>

/**
 * 'INITIAL_CAPACITY' the initial size of the dynamically.
 */
const size_t INITIAL_CAPACITY = 8;
/**
 * 'GROWTH_FACTOR' is how much the vector will grow by in automatic reallocation
 * (2 means double).
 */
const size_t GROWTH_FACTOR = 2;

struct vector {
    /* The function callback for the user to define the way they want to copy
     * elements */
    copy_constructor_type copy_constructor;

    /* The function callback for the user to define the way they want to destroy
     * elements */
    destructor_type destructor;

    /* The function callback for the user to define the way they a default
     * element to be constructed */
    default_constructor_type default_constructor;

    /* Void pointer to the beginning of an array of void pointers to arbitrary
     * data. */
    void **array;

    /**
     * The number of elements in the vector.
     * This is the number of actual objects held in the vector,
     * which is not necessarily equal to its capacity.
     */
    size_t size;

    /**
     * The size of the storage space currently allocated for the vector,
     * expressed in terms of elements.
     */
    size_t capacity;
};

/**
 * IMPLEMENTATION DETAILS
 *
 * The following is documented only in the .c file of vector,
 * since it is implementation specfic and does not concern the user:
 *
 * This vector is defined by the struct above.
 * The struct is complete as is and does not need any modifications.
 *
 * The only conditions of automatic reallocation is that
 * they should happen logarithmically compared to the growth of the size of the
 * vector inorder to achieve amortized constant time complexity for appending to
 * the vector.
 *
 * For our implementation automatic reallocation happens when -and only when-
 * adding to the vector makes its new  size surpass its current vector capacity
 * OR when the user calls on vector_reserve().
 * When this happens the new capacity will be whatever power of the
 * 'GROWTH_FACTOR' greater than or equal to the target capacity.
 * In the case when the new size exceeds the current capacity the target
 * capacity is the new size.
 * In the case when the user calls vector_reserve(n) the target capacity is 'n'
 * itself.
 * We have provided get_new_capacity() to help make this less ambigious.
 */

static size_t get_new_capacity(size_t target) {
    /**
     * This function works according to 'automatic reallocation'.
     * Start at 1 and keep multiplying by the GROWTH_FACTOR untl
     * you have exceeded or met your target capacity.
     */
    size_t new_capacity = 1;
    while (new_capacity < target) {
        new_capacity *= GROWTH_FACTOR;
    }
    return new_capacity;
}

vector *vector_create(copy_constructor_type copy_constructor,
                      destructor_type destructor,
                      default_constructor_type default_constructor) {
    // your code here
    // Casting to void to remove complier error. Remove this line when you are
    // ready.
    struct vector *this = malloc (sizeof(struct vector));
    this->array = malloc(INITIAL_CAPACITY * sizeof(void*));
    this->capacity = INITIAL_CAPACITY;
    if (!copy_constructor) {
        copy_constructor = shallow_copy_constructor;
    }
    if (!destructor) {
        destructor = shallow_destructor;
    }
    if (!default_constructor) {
        default_constructor = shallow_default_constructor;
    }
    this->copy_constructor = copy_constructor;
    this->destructor = destructor;
    this->default_constructor = default_constructor;
    this->size = 0;
    return this;
}

void vector_destroy(vector *this) {
    // your code here
    if (!this) {
        this->size = 0;
        return;
    }
    for (size_t i = 0; i < vector_size(this); i++) {
        if (this->array[i]) {
            this->destructor(this->array[i]);
        }
    }
    this->size = 0;
    if (this->array) {
        free(this->array);
    }
    if (this) {
        free(this);
    }
}

void **vector_begin(vector *this) {
    assert(this);
    return this->array;
}

void **vector_end(vector *this) {
    assert(this);
    return this->array + (vector_size(this) - 1);
}

size_t vector_size(vector *this) {
    assert(this);
    // your code here
    return this->size;
}

void vector_resize(vector *this, size_t n) {
    assert(this);
    // your code here
    assert(n >= 0);
    if (n == vector_size(this)) {
        return;
    } 
    if (n == 0) {
        vector_clear(this);
        this->size = n;
        return;
    }
    // resize 
    // If 'n' is smaller than the current container size,
    if (n < vector_size(this)) {
        for (size_t i = n; i < vector_size(this); i++) {
            if (this->array[i]) {
                this->destructor(this->array[i]);
            }
        }
        this->size = n;
        return;
    }
    // If 'n' is also greater than the current container capacity
    if (n > vector_capacity(this)) {
        this->capacity = get_new_capacity(n);
        this->array = realloc(this->array, vector_capacity(this) * sizeof(void*));
        for (size_t i = vector_size(this); i < n; i++) { 
            this->array[i] = this->default_constructor();
        }
    }
    // If 'n' is greater than the current container size
    if (n > vector_size(this)) {
        for (size_t i = vector_size(this); i < n; i++) {
            this->array[i] = this->default_constructor();
        }
        this->size = n;
    }
}

size_t vector_capacity(vector *this) {
    assert(this);
    // your code here
    return this->capacity;
}

bool vector_empty(vector *this) {
    assert(this);
    // your code here
    if (this->size == 0) {
        return true;
    }
    return false;
}

void vector_reserve(vector *this, size_t n) {
    assert(this);
    // your code here
    assert(n >= 0);
    if (n > vector_capacity(this)) {
        this->capacity = get_new_capacity(n);
        this->array = realloc(this->array, vector_capacity(this) * sizeof(void*));
    }
}

void **vector_at(vector *this, size_t position) {
    assert(this);
    // your code here
    assert(position < vector_size(this));
    assert(position >= 0);
    return this->array + position;
}

void vector_set(vector *this, size_t position, void *element) {
    assert(this);
    assert(element);
    assert(position >= 0);
    assert(position < vector_size(this));
    // your code here
    this->array[position] = this->copy_constructor(element);
    return;
}

void *vector_get(vector *this, size_t position) {
    assert(this);
    // your code here
    assert(position < vector_size(this));
    assert(position >= 0);
    return this->array[position];
}

void **vector_front(vector *this) {
    assert(this);
    // your code here
    return this->array;
}

void **vector_back(vector *this) {
    // your code here
    assert(this);
    return this->array + (this->size - 1);
}

void vector_push_back(vector *this, void *element) {
    assert(this);
    assert(element);
    // your code here
    if (this->size < vector_capacity(this)) {
        this->array[this->size] = this->copy_constructor(element);
        this->size++;
        return;
    }
    this->capacity = get_new_capacity(vector_capacity(this) + 1);
    this->array = realloc(this->array, vector_capacity(this) * sizeof(void*));
    this->array[this->size] = this->copy_constructor(element);
    this->size++;
    return;
}

void vector_pop_back(vector *this) {
    assert(this);
    // your code here
    assert(vector_size(this) != 0);
    this->destructor(this->array[vector_size(this) - 1]);
    this->size--;
}


void vector_insert(vector *this, size_t position, void *element) {
    assert(this);
    assert(element);
    // your code here
    assert(position >= 0);

    if (vector_size(this) == 0) {
        this->array[0] = this->copy_constructor(element);
        this->size++;
        return;
    }
    // last index
    if (position == vector_size(this)) {
        vector_push_back(this, element);
        return;
    }
    // 1. if size equals to capacity -> resize
    // 2. move the data after that position forward by 1
    //    for (size_t i = this->size - 1; i >= position; i--) { vec[i+1] = vec[i]; }
    // 3. vec[position] = elem;
    // 4. increament vector size by 1;
    if (vector_size(this) == vector_capacity(this)) {
        this->capacity = get_new_capacity(vector_capacity(this) + 1);
        this->array = realloc(this->array, this->capacity * sizeof(void *));
    }
    if (position > 0) {
        for (size_t i = vector_size(this) - 1; i >= position; i--) {
            this->array[i + 1] = this->copy_constructor(this->array[i]);
        }
        this->array[position] = this->copy_constructor(element);
        this->size++;
        return;
    } 
    // edge case: insert front
    for (size_t i = vector_size(this); i > 0; i--) {
        this->array[i] = this->copy_constructor(this->array[i - 1]);
    }
    this->array[0] = this->copy_constructor(element);
    this->size++;
    return;
}

void vector_erase(vector *this, size_t position) {
    assert(this);
    assert(position < vector_size(this));
    // your code here
    this->destructor(this->array[position]);
    for (size_t i = position; i < vector_size(this); i++) {
        this->array[i] = this->copy_constructor(this->array[i+1]);
    }
    this->size--;
}

void vector_clear(vector *this) { 
    // your code here
    if (!this) {
        this->size = 0;
        return;
    }
    for (size_t i = 0; i < vector_size(this); i++) {
        if (this->array[i]) {
            this->destructor(this->array[i]);
        }
    }
    this->size = 0;
}

// The following is code generated:
vector *shallow_vector_create() {
    return vector_create(shallow_copy_constructor, shallow_destructor,
                         shallow_default_constructor);
}
vector *string_vector_create() {
    return vector_create(string_copy_constructor, string_destructor,
                         string_default_constructor);
}
vector *char_vector_create() {
    return vector_create(char_copy_constructor, char_destructor,
                         char_default_constructor);
}
vector *double_vector_create() {
    return vector_create(double_copy_constructor, double_destructor,
                         double_default_constructor);
}
vector *float_vector_create() {
    return vector_create(float_copy_constructor, float_destructor,
                         float_default_constructor);
}
vector *int_vector_create() {
    return vector_create(int_copy_constructor, int_destructor,
                         int_default_constructor);
}
vector *long_vector_create() {
    return vector_create(long_copy_constructor, long_destructor,
                         long_default_constructor);
}
vector *short_vector_create() {
    return vector_create(short_copy_constructor, short_destructor,
                         short_default_constructor);
}
vector *unsigned_char_vector_create() {
    return vector_create(unsigned_char_copy_constructor,
                         unsigned_char_destructor,
                         unsigned_char_default_constructor);
}
vector *unsigned_int_vector_create() {
    return vector_create(unsigned_int_copy_constructor, unsigned_int_destructor,
                         unsigned_int_default_constructor);
}
vector *unsigned_long_vector_create() {
    return vector_create(unsigned_long_copy_constructor,
                         unsigned_long_destructor,
                         unsigned_long_default_constructor);
}
vector *unsigned_short_vector_create() {
    return vector_create(unsigned_short_copy_constructor,
                         unsigned_short_destructor,
                         unsigned_short_default_constructor);
}



