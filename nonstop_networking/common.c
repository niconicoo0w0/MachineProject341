/**
 * nonstop_networking
 * CS 341 - Spring 2023
 */
#include "common.h"

static const size_t MESSAGE_SIZE_DIGITS = 4;

// From Lecture
int connect_to_server(char* host, char* port) {
    struct addrinfo hints, *res;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // int getaddrinfo(char* host, char* service, addrinfo* hints, addrinfo** res);
    int s = getaddrinfo(host, port, &hints, &res);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(1);
    }
    int sock_fd = socket(res->ai_family, res->ai_socktype, res->ai_protocol);
    if (sock_fd == -1) {
        perror("socket failed!");
        exit(1);
    }
    int ok = connect(sock_fd, res->ai_addr, res->ai_addrlen);
    if (ok == -1) {
        perror("connect failed!");
        exit(1);
    }
    if (res) {
        freeaddrinfo(res);
    }
    return sock_fd;
}



// From Lab Charming Chatroom
ssize_t read_all_from_socket(int socket, char *buffer, size_t count) {
    size_t returnit = 0;
    while (returnit < count) {
        ssize_t br = read(socket, buffer + returnit, count - returnit);
        if (br == 0) {
            break;
        }
        if (br > 0) {
            returnit += br;
        } else if (br == -1 && errno != EINTR) {
            return -1;
        } else {
            continue;
        }
    }
    return returnit;
}

// From Lab Charming Chatroom
ssize_t write_all_to_socket(int socket, const char *buffer, size_t count) {
    size_t returnit = 0;
    while (returnit < count) {
        ssize_t br = write(socket, buffer + returnit, count - returnit);
        if (br == 0) {
            break;
        }
        if (br > 0) {
            returnit += br;
        } else if (br == -1 && errno != EINTR) {
            return -1;
        } else {
            continue;
        }
    }
    return returnit;
}

ssize_t help_write_read_file(ssize_t size, FILE* fp, int sock_fd, int write) {
    ssize_t count = 0;
    if (write) {
        while (count < size) {
            ssize_t len = BLK_SIZE;
            if (size <= BLK_SIZE + count) {
                len = size - count;
            }
            char buffer[len];
            fread(buffer, 1, len, fp);
            ssize_t write_size = write_all_to_socket(sock_fd, buffer, len);
            if (write_size == -1 || write_size == 0) {
                return 1;
            }
            if (write_size < len) {
                return 1;
            }
            count += len;
        }
        return 0;
    }
    while (count < size + BLK_SIZE) {
        size_t len = BLK_SIZE;
        if (size <= count) {
            len = size + BLK_SIZE - count;
        }
        char buffer[len];
        ssize_t read_size = read_all_from_socket(sock_fd, buffer, len);
        if (read_size == -1 || read_size == 0) {
            break;
        }
        fwrite(buffer, 1, read_size, fp);
        count += read_size;
    }
    return count;
}

// From Lab Charming Chatroom
ssize_t write_message_size(size_t size, int socket) {
    ssize_t tmp = htonl(size);
    return write_all_to_socket(socket, (char*)&tmp, MESSAGE_SIZE_DIGITS);
}

// From Lab Charming Chatroom
ssize_t get_message_size(int socket) {
    int32_t size;
    ssize_t returnit = read_all_from_socket(socket, (char*)&size, MESSAGE_SIZE_DIGITS);
    if (returnit == 0) {
        return returnit;
    }
    if (returnit == -1) {
        return returnit;
    }
    return (ssize_t)ntohl(size);
}

ssize_t read_until_newline(int fd, char* buffer, size_t count) {
    size_t returnit = 0;
    while (returnit < count) {
        ssize_t result = read(fd, buffer + returnit, 1);
        if (result == 0) {
            break;
        } else if (result == -1 && errno == EINTR) {
            continue;
        } else if (result == -1) {
            return -1;
        }
        returnit += result;
        if (buffer[returnit - 1] == '\n') {
            break;
        }
    }
    if (returnit < 0) {
        perror("read");
        exit(1);
    }
    return returnit;
}



