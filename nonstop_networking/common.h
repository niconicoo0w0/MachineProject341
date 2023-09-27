/**
 * nonstop_networking
 * CS 341 - Spring 2023
 */
#pragma once
#include <stddef.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <arpa/inet.h>
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <dirent.h>
#include <signal.h>
#include <sys/epoll.h>
#include "./includes/vector.h"
#include "./includes/dictionary.h"


#define LOG(...)                      \
    do {                              \
        fprintf(stderr, __VA_ARGS__); \
        fprintf(stderr, "\n");        \
    } while (0);

typedef enum { GET, PUT, DELETE, LIST, V_UNKNOWN } verb;


#define MIN(x, y) (x < y ? x : y)
#define MAX(x, y) (x > y ? x : y)

// The largest size the message can be that a client sends to the server.
#define MAX_FILENAME_SIZE 255
#define BLK_SIZE 1024
#define SPACE_SIZE 3
#define EXP_SIZE 5
#define OK "OK\n"
#define ERROR "ERROR\n"
#define OK_SIZE 3
#define ERROR_SIZE 6
#define MAX_EVENTS 100


typedef struct client_info {
    char header[BLK_SIZE];
    /* Name of the file */
    char file_name[MAX_FILENAME_SIZE];
    /* 1 if ready to parse, 0 if not*/
    int ready_parse;
    /* 1 if ready to exec, 0 if not*/
    int ready_exec;

    int error_type;
    verb command;
} client_info;

/**
 * Attempts to read all count bytes from socket into buffer.
 * Assumes buffer is large enough.
 *
 * Returns the number of bytes read, 0 if socket is disconnected,
 * or -1 on failure.
 */
ssize_t read_all_from_socket(int socket, char *buffer, size_t count);

/**
 * Attempts to write all count bytes from buffer to socket.
 * Assumes buffer contains at least count bytes.
 *
 * Returns the number of bytes written, 0 if socket is disconnected,
 * or -1 on failure.
 */
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count);

/**
 * Writes the bytes of size to the socket
 *
 * Returns the number of bytes successfully written,
 * 0 if socket is disconnected, or -1 on failure
 */
ssize_t write_message_size(size_t size, int socket);

ssize_t get_message_size(int socket);

/**
 * Connect to a server with a TCP/IP socket
 * Return 0 if success
 */
int connect_to_server(char* host, char* port);

/**
 * write == 1, write response
 * Helper function when command == PUT
 * Return 1 print_connection_closed();
 * Return 0 if no problem
*/
ssize_t help_write_read_file(ssize_t size, FILE* fp, int sock_fd, int write);

/**
 * Helper function for server.c, return the size
*/
ssize_t read_until_newline(int fd, char* buffer, size_t count);

/**
 * Signal handler for server.c
*/
void signal_handler();