/**
 * nonstop_networking
 * CS 341 - Spring 2023
 */
#include "format.h"
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

static size_t ok_size = strlen(OK);
static size_t err_size = strlen(ERROR);

char **parse_args(int argc, char **argv);
verb check_args(char **args);
void check_all_to_socket(ssize_t size);
void check_size(size_t size, size_t actual_size);

int main(int argc, char **argv) {
    // Good luck!
    char** args = parse_args(argc, argv);
    // does not exist
    if (!args || argc < 3) {
        print_client_usage();
        exit(EXIT_FAILURE);
    }
    verb command = check_args(args);

    char* host = args[0];
    char* port = args[1];
    int sock_fd = connect_to_server(host, port);

    // Write Response
    ssize_t write_size;
    ssize_t request_write_size;
    char* write_buffer;
    
    if (command != LIST) {
        write_buffer = calloc(1, strlen(args[2]) + strlen(args[3]) + SPACE_SIZE);
        sprintf(write_buffer, "%s %s\n", args[2], args[3]);
        request_write_size = strlen(write_buffer);
        write_size = write_all_to_socket(sock_fd, write_buffer, request_write_size);
    } else {
        write_buffer = calloc(1, strlen(args[2]) + SPACE_SIZE - 1);
        sprintf(write_buffer, "%s\n", args[2]);
        request_write_size = strlen(write_buffer);
        write_size = write_all_to_socket(sock_fd, write_buffer, request_write_size);
    }
    if (write_buffer) { free(write_buffer); }
    // 0 if socket is disconnected, -1 on failure.
    check_all_to_socket(write_size);
    if (write_size < request_write_size) {
        print_connection_closed();
        exit(EXIT_FAILURE);
    }
    if (command == DELETE || command == LIST || command == GET) {
        int shut_wr = shutdown(sock_fd, SHUT_WR);
        if (shut_wr != 0 || shut_wr == -1) {
            perror("DELETE / LIST / GET fail to shutdown write");
        }
    } else if (command == PUT) {
        struct stat buf;
        if (stat(args[4], &buf) == -1) {
            perror("bad status");
            exit(EXIT_FAILURE);
        }
        ssize_t size = buf.st_size;
        size_t count = sizeof(size_t);
        write_size = write_all_to_socket(sock_fd, ((char*)&size), count);
        check_all_to_socket(write_size);
        FILE* fp = fopen(args[4], "r");
        if (!fp) {
            perror(err_no_such_file);
            exit(EXIT_FAILURE);
        }
        ssize_t failed = help_write_read_file(size, fp, sock_fd, 1);
        if (failed) {
            print_connection_closed();
            exit(EXIT_FAILURE);
        }
        fclose(fp);
        int shut_wr = shutdown(sock_fd, SHUT_WR);
        if (shut_wr != 0 || shut_wr == -1) {
            perror("PUT fail to shutdown write");
        }
    }
    // if (shut_wr != 0 || shut_wr == -1) {
    //     perror("Mwa Mwa Mwa");
    // }
    // Read Response
    ssize_t read_size;
    char* read_buffer = calloc(1, ok_size + 1);
    read_size = read_all_from_socket(sock_fd, read_buffer, ok_size);
    check_all_to_socket(read_size);
    if (!strcmp(read_buffer, OK)) {
        fprintf(stdout, "%s", read_buffer);
        // GET - Client downloads (GETs) a file from the server 
        if (command == GET) {
            FILE *fp = fopen(args[4], "w+");
            if (!fp) {
                perror(err_no_such_file);
                exit(EXIT_FAILURE);
            }
            ssize_t size;
            read_size = read_all_from_socket(sock_fd, (char*) &size, sizeof(size_t));
            check_all_to_socket(read_size);
            // write to local file
            ssize_t count = help_write_read_file(size, fp, sock_fd, 0);
            check_size(size, count);
            fclose(fp);
        } 
        // DELETE - Client deletes a file from the server
        if (command == DELETE) {
            print_success();
            FILE *fp = fopen(args[4], "r");
            if (!fp) {
                perror(err_no_such_file);
                exit(EXIT_FAILURE);
            }
            fclose(fp);
        }
        // PUT - Client uploads (PUTs) a file to the server 
        if (command == PUT) {
            print_success();
        }
        // LIST - Client receives a list of files from the server 
        if (command == LIST) {
            size_t size;
            size_t count = sizeof(size_t);
            read_size = read_all_from_socket(sock_fd, (char*)&size, count);
            check_all_to_socket(read_size);
            const size_t buffer_size = size + EXP_SIZE + 1;
            char list_buffer[buffer_size];
            memset(list_buffer, 0, buffer_size);
            size_t num_read = read_all_from_socket(sock_fd, list_buffer, buffer_size - 1);
            check_size(size, num_read);
            fprintf(stdout, "%zu%s", size, list_buffer);
        }
    }
    if (strcmp(read_buffer, OK)) {
        read_buffer = realloc(read_buffer, err_size + 1);
        read_size = read_all_from_socket(sock_fd, read_buffer + read_size, err_size - read_size);
        check_all_to_socket(read_size);
        if (strcmp(read_buffer, ERROR)) {
            print_invalid_response();
            exit(EXIT_FAILURE);
        } else {
            char message[50];
            read_size = read_all_from_socket(sock_fd, message, 50);
            check_all_to_socket(read_size);
            print_error_message(message);
        }
    }
    // destroy
    shutdown(sock_fd, SHUT_RD);
    close(sock_fd);
    if (args) { free(args); }
    if (read_buffer) { free(read_buffer); }
}


/**
 * Check the result for write_all_to_socket and read_all_from_socket
 */
void check_all_to_socket(ssize_t size) {
    if (size == 0 || size == -1) {
        print_connection_closed();
        exit(EXIT_FAILURE);
    }
} 

/**
 * Check num_read and num_write to avoid too little / too much data
*/
void check_size(size_t size, size_t actual_size) {
    if (actual_size == 0 && size != 0) {
        print_connection_closed();
        exit(EXIT_FAILURE);
    }
    // size, count
    if (actual_size < size) {
        print_too_little_data();
        exit(EXIT_FAILURE);
    }
    if (actual_size > size) {
        print_received_too_much_data();
        exit(EXIT_FAILURE);
    }
}

/**
 * Given commandline argc and argv, parses argv.
 *
 * argc argc from main()
 * argv argv from main()
 *
 * Returns char* array in form of {host, port, method, remote, local, NULL}
 * where `method` is ALL CAPS
 */
char **parse_args(int argc, char **argv) {
    if (argc < 3) {
        return NULL;
    }

    char *host = strtok(argv[1], ":");
    char *port = strtok(NULL, ":");
    if (port == NULL) {
        return NULL;
    }

    char **args = calloc(1, 6 * sizeof(char *));
    args[0] = host;
    args[1] = port;
    args[2] = argv[2];
    char *temp = args[2];
    while (*temp) {
        *temp = toupper((unsigned char)*temp);
        temp++;
    }
    if (argc > 3) {
        args[3] = argv[3];
    }
    if (argc > 4) {
        args[4] = argv[4];
    }
    return args;
}

/**
 * Validates args to program.  If `args` are not valid, help information for the
 * program is printed.
 *
 * args     arguments to parse
 *
 * Returns a verb which corresponds to the request method
 */
verb check_args(char **args) {
    if (args == NULL) {
        print_client_usage();
        exit(EXIT_FAILURE);
    }

    char *command = args[2];

    if (strcmp(command, "LIST") == 0) {
        return LIST;
    }

    if (strcmp(command, "GET") == 0) {
        if (args[3] != NULL && args[4] != NULL) {
            return GET;
        }
        print_client_help();
        exit(EXIT_FAILURE);
    }

    if (strcmp(command, "DELETE") == 0) {
        if (args[3] != NULL) {
            return DELETE;
        }
        print_client_help();
        exit(EXIT_FAILURE);
    }

    if (strcmp(command, "PUT") == 0) {
        if (args[3] == NULL || args[4] == NULL) {
            print_client_help();
            exit(EXIT_FAILURE);
        }
        return PUT;
    }

    // Not a valid Method
    print_client_help();
    exit(EXIT_FAILURE);
}

