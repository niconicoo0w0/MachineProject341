/**
 * utilities_unleashed
 * CS 341 - Spring 2023
 */

#include "format.h"

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>

#include <sys/types.h>
#include <sys/wait.h>

int valid_string(char *str);
int valid_value(char *str);
void set_env_variable(char *key, char *value);

#define MAX_NAME_LEN 128

int main(int argc, char *argv[]) {
    if (argc < 4) {
        print_env_usage();
        exit(1);
    }
    pid_t pid = fork();
    if (pid > 0) {
        int status;
        waitpid(pid, &status, 0);
        return 0;
    } else if (pid < 0) {
        print_fork_failed();
        exit(1);
    } else {
        int i = 1;
        while (argv[i] && strcmp(argv[i], "--")) {
            char *key = strtok(argv[i], "=");
            char *val = strtok(NULL, "");
            if (!val) {
                print_env_usage();
                exit(1);
            }
            set_env_variable(key, val);
            i++;
        }
        if (!argv[i] || !argv[i + 1]) {
            print_env_usage();
            exit(1);
        }
        execvp(argv[i + 1], argv + i + 1);
        print_exec_failed();
        exit(1);
    }
    return 0;
}

int valid_string(char *str) {
    while (*str++) {
        if (!isalpha(*str) && !isdigit(*str) && *str != '_') {
            return 0;
        }
    }
    return 1;
}

int valid_value(char *value) {
    if (value[0] == '%') {
        value = getenv(value + 1);
    } else if (!valid_string(value)) {
        return 0;
    }
    return 1;
}

void set_env_variable(char *key, char *value) {
    if (value[0] == '%') {
        value = getenv(value + 1);
        if (!value) {
            print_environment_change_failed();
            exit(1);
        }
    }
    char *ptr = key;
    while (*ptr) {
        if (!(isalnum(*ptr) || *ptr == '_')) {
            print_env_usage();
            exit(1);
        }
        ptr++;
    }
    ptr = value;
    while (*ptr) {
        if (!(isalnum(*ptr) || *ptr == '_')) {
            print_env_usage();
            exit(1);
        }
        ptr++;
    }
    if (setenv(key, value, 1) < 0) {
        print_environment_change_failed();
        exit(1);
    }
}

// void destroy(char **str, char **val, char ***token, size_t num1, int num2) {
//     if (str) {
// 		for (size_t i = 0; i < num1; i++) {
// 			if (str[i]) {
//                 free(str[i]);
//             }
// 		}
// 		free(str);
// 	}
//     if (val) {
// 		for (size_t i = 0; i < num1; i++) {
// 			if (val[i]) {
//                 free(val[i]);
//             }
// 		}
// 		free(val);
// 	}
// 	if (token) {
//         for (size_t i = 0; i < num1; i++) {
//             if (token[i]) {
//                 for (int j = 0; j < num2; j++) {
//                     if (token[i][j]) {
//                         free(token[i][j]);
//                     }
//                 }
//                 free(token[i]);
//             }
//         }
//     }
// 	free(token);
// }
