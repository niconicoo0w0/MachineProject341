/**
 * extreme_edge_cases
 * CS 341 - Spring 2023
 */
#include "camelCaser.h"

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>


int count_sentence(const char *input_str) {
	if (input_str == NULL || strlen(input_str) == 0) {
		return 0;
	}
	int flag = 0;
	int count = 0;
	size_t i;
	for (i = 0; i < strlen(input_str); i++) {
		if (isalpha(input_str[i])) {
			flag = 1;
		}
		if (ispunct(input_str[i]) && flag == 1) {
			count++;
			flag = 0;
		}
	}
	return count;
}

int count_punct(const char *input_str) {
	if (input_str == NULL || strlen(input_str) == 0) {
		return 0;
	}
	int count = 0;
	size_t i;
	for (i = 0; i < strlen(input_str); i++) {
		if (ispunct(input_str[i])) {
			count++;
		}
	}
	return count;
}

char* Trim_and_Camel(char *input) {
	if (input == NULL || strlen(input) == 0) {
		return input;
	}
	char* returnit = malloc(strlen(input) + 1);
	memset(returnit, 0, strlen(input) + 1); 

	size_t i;
	for (i = 0; i < strlen(input); i++) {
		returnit[i] = tolower(input[i]);
		
	}
	for (i = 0; i < strlen(input); i++) {
		if (i != 0) {
			if (isalpha(input[i]) && isspace(input[i-1])) {
				returnit[i] = toupper(input[i]);
			}
		}
	}
	size_t j, k;
	char* new_returnit = malloc(strlen(input) + 1);
	memset(new_returnit, 0, strlen(input) + 1); 

  	for (k = j = 0; returnit[k]; ++k) {
		if (!isspace(returnit[k]) || (k > -1 && !isspace(returnit[k-1]))) {
			new_returnit[j] = returnit[k];
			j++;
		}
	}
	new_returnit[j] = '\0';
	for (i = 0; i < strlen(new_returnit)-1; i++) {
		if (ispunct(new_returnit[i])) {
			new_returnit[i+1] = tolower(new_returnit[i+1]);
		}
	}
	if (input) {
		free(input);
	}
	if (returnit) {
		free(returnit);
	}
	return new_returnit;
}


char **camel_caser(const char *input_str) {
	// TODO: Implement me!
	if (input_str == NULL) {
		return NULL;
	}
	char* output_str = malloc(strlen(input_str) + 1);
	strcpy(output_str, input_str);
	output_str = Trim_and_Camel(output_str);
	
	char** returnit = (char **)malloc(sizeof(char *) * (count_punct(input_str) + 1));
	memset(returnit, 0, sizeof(char*) * (count_punct(input_str) + 1));

	size_t i, j, k;
	for (i = 0, j = 0, k = 0; i < strlen(output_str); i++) {
        if (ispunct(output_str[i])) {
            if (i > j) {
                returnit[k] = (char*) malloc(sizeof(char) * (i - j + 1));
                strncpy(returnit[k], output_str + j, i - j);
                returnit[k][i - j] = '\0';
                k++;
            } else {
                returnit[k] = (char*) malloc(sizeof(char));
                returnit[k][0] = '\0';
                k++;
            }
            j = i + 1;
        }
    }
    returnit[count_punct(output_str)] = NULL;
	if (output_str) {
		free(output_str);
	}
	return returnit;
}

void destroy(char **input) {
    // TODO: Implement me!
	if (!input) {   
		return;
	}
    char **tmp;
    for (tmp = input; *tmp != NULL; tmp++) {
        free(*tmp);
    }
	free(input);
}