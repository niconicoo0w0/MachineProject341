/**
 * perilous_pointers
 * CS 341 - Spring 2023
 */
#include "part2-functions.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * (Edit this function to print out the "Illinois" lines in
 * part2-functions.c in order.)
 */
int main() {
    // your code here
    first_step(81);

    int* num;
    num = malloc(sizeof(int));
    *num = 132;
    second_step(num);
    free(num);

    int** numarray;
    numarray = malloc(sizeof(int));
    numarray[0] = malloc(sizeof(int));
    *numarray[0] = 8942;
    double_step(numarray);
    free(numarray[0]);
    free(numarray);

    char* s;
    int fifteen = 15;
    s = (char*)(&fifteen) - 5 * sizeof(char);
    strange_step(s);

    char* empty = malloc(4);
    empty[3] = 0;
    empty_step((void*) empty);
    free(empty);

    char* s2 = malloc(4);
    s2[3] = 'u';
    two_step((void*)s2, s2);
    free(s2);

    char* first = malloc(1);
    char* second = malloc(1);
    char* third = malloc(1);
    second = first + 2;
    third =  second + 2;
    three_step(first, second, third);
    free(first);
    // free(second);
    // free(third);

    char* first1 = malloc(4);
    char* second1 = malloc(4);
    char* third1 = malloc(4);
    first1[1] = 'a';
    second1[2] = first1[1] + 8;
    third1[3] = second1[2] + 8;
    step_step_step(first1, second1, third1);
    free(first1);
    free(second1);
    free(third1);

    char *a = malloc(1);
    *a = '0';
    it_may_be_odd(a, (int)'0');
    free(a);

    char str[10] = "omg,CS241";
    tok_step(str);

    char * color = malloc(4);
    color[0] = 1;
    color[1] = 0;
    color[2] = 0;
    color[3] = 8;
    the_end(color, color);
    free(color);
    
    return 0;
}