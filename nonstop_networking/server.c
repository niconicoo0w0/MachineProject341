/**
 * nonstop_networking
 * CS 341 - Spring 2023
 */

#include "format.h"
#include "common.h"

static char* directory;
static vector* file_name_vec;
static dictionary* connect_status_dict;
static dictionary* file_size_dict;
static int epoll;

void signal_handler() { }

void update_epoll_events(int fd, uint32_t events) {
    struct epoll_event ev;
    memset(&ev, '\0', sizeof(struct epoll_event));
    ev.events = events;
    ev.data.fd = fd;
    if (epoll_ctl(epoll, EPOLL_CTL_MOD, fd, &ev) == -1) {
        perror("epoll failed!");
        exit(EXIT_FAILURE);
    }
}

void disconnect_server() {
    close(epoll);
    vector *infos = dictionary_values(connect_status_dict);
    for (size_t i = 0; i < vector_size(infos); i++) {
        free(vector_at(infos, i));
    }
    vector_destroy(infos);
    vector_destroy(file_name_vec);
    dictionary_destroy(file_size_dict);
    dictionary_destroy(connect_status_dict);
    DIR* temp = opendir(directory);
    if (!temp) {
        perror("opendir failed!");
        rmdir(directory);
        exit(EXIT_FAILURE);
    }
    for (struct dirent* ptr = readdir(temp); ptr; ptr = readdir(temp)) {
        size_t len = strlen(directory) + strlen(ptr->d_name) + 1;
        if (strcmp(ptr->d_name, ".") || strcmp(ptr->d_name, "..")) {
            char path[len];
            sprintf(path, "%s/%s", directory, ptr->d_name);
            unlink(path);
        }
    }
    closedir(temp);
    rmdir(directory);
    exit(EXIT_SUCCESS);
}

int help_put(int fd) { 
    client_info* info = dictionary_get(connect_status_dict, &fd);
    size_t path_len = strlen(directory) + strlen(info->file_name) + SPACE_SIZE - 1;
    if (path_len > MAX_FILENAME_SIZE || path_len < 1) {
        exit(EXIT_FAILURE);
    }
    char path[path_len];
    memset(path, 0, path_len);
    sprintf(path, "%s/%s", directory, info->file_name);

    if (access(path, F_OK) == 0) {
        fclose(fopen(path, "r"));
    } else {
        vector_push_back(file_name_vec, info->file_name);
    }
    FILE* fp = fopen(path, "w");
    ssize_t size;
    ssize_t num_read = read_all_from_socket(fd, (char*)&size, sizeof(size_t));
    if (num_read == 0 || num_read == -1) {
        fclose(fp);
        remove(path);
        return 1;
    }
    ssize_t count = help_write_read_file(size, fp, fd, 0);
    fclose(fp);
    if (count - size == 0) {
        dictionary_set(file_size_dict, info->file_name, &size);
        return 0;
    }
    if (count < size) {
        remove(path);
        print_too_little_data();
        exit(EXIT_FAILURE);
    }
    remove(path);
    print_received_too_much_data();
    exit(EXIT_FAILURE);
}

void help_get(int client_fd) {
    client_info* info = (client_info*)dictionary_get(connect_status_dict, &client_fd);
    size_t path_len = strlen(directory) + strlen(info->file_name) + SPACE_SIZE - 1;
    if (path_len > MAX_FILENAME_SIZE || path_len < 1) {
        exit(EXIT_FAILURE);
    }
    char path[path_len];
    memset(path, 0, path_len);
    sprintf(path, "%s/%s", directory, info->file_name);
    FILE* fp = fopen(path, "r");
    if (fp) {
        write_all_to_socket(client_fd, OK, OK_SIZE);
        ssize_t size = *((ssize_t*)dictionary_get(file_size_dict, info->file_name));
        if (write_all_to_socket(client_fd, (char*) &size, sizeof(size_t)) == -1) {
            print_connection_closed();
            exit(EXIT_FAILURE);
        }
        ssize_t error = help_write_read_file(size, fp, client_fd, 1);
        if (error) {
            print_connection_closed();
            exit(EXIT_FAILURE);
        }
    }
    // File not found
    info->error_type = 3;
    return;
}

void help_delete(int fd) {
    client_info* info = dictionary_get(connect_status_dict, &fd);
    char path[MAX_FILENAME_SIZE];
    snprintf(path, MAX_FILENAME_SIZE, "%s/%s", directory, info->file_name);
    if (unlink(path) == -1) {
        info->error_type = 3;
        return;
    }
    dictionary_remove(file_size_dict, info->file_name);
    size_t i = 0;
    bool file_found = false;
    while (i < vector_size(file_name_vec)) {
        char* file = vector_get(file_name_vec, i);
        if (!strcmp(info->file_name, file)) {
            file_found = true;
            break;
        }
        i++;
    }
    if (!file_found) {
        info->error_type = 3;
        return;
    }
    vector_erase(file_name_vec, i);
    write_all_to_socket(fd, OK, OK_SIZE);
}

void help_list(int fd) {
    write_all_to_socket(fd, OK, OK_SIZE);
    size_t num_files = vector_size(file_name_vec);
    if (num_files == 0) {
        size_t response_size = 0;
        write_all_to_socket(fd, (char*) &response_size, sizeof(size_t));
        return;
    }
    size_t response_size = num_files - 1;
    for (char** curr_filename = (char**)vector_begin(file_name_vec); curr_filename != (char**)vector_end(file_name_vec); ++curr_filename) {
        response_size += strlen(*curr_filename);
    }
    write_all_to_socket(fd, (char*) &response_size, sizeof(size_t));
    for (char** curr_filename = (char**)vector_begin(file_name_vec); curr_filename != (char**)vector_end(file_name_vec); ++curr_filename) {
        write_all_to_socket(fd, *curr_filename, strlen(*curr_filename));
        if (curr_filename != (char**)vector_end(file_name_vec) - 1) {
            write_all_to_socket(fd, "\n", 1);
        }
    }
}

int parse_helper(client_info* info, char* file_name, int fd, int num) {
    if (num < 2 || strlen(file_name) >= MAX_FILENAME_SIZE) {
        info->error_type = 1;
        update_epoll_events(fd, EPOLLOUT);
        return 1;
    }
    strncpy(info->file_name, file_name, MAX_FILENAME_SIZE);
    info->file_name[MAX_FILENAME_SIZE - 1] = '\0';
    return 0;
}

void parse_header(int fd) {
    client_info* info = dictionary_get(connect_status_dict, &fd);
    char buffer[BLK_SIZE];
    memset(buffer, 0, sizeof(buffer));
    read_until_newline(fd, buffer, BLK_SIZE);
    char command[10];
    char file_name[MAX_FILENAME_SIZE];
    memset(command, 0, sizeof(command));
    memset(file_name, 0, sizeof(file_name));
    int num = sscanf(buffer, "%s %s", command, file_name);
    if (num < 1) {
        info->error_type = 1;
        update_epoll_events(fd, EPOLLOUT);
        return;
    }
    if (!strcmp(command, "PUT")) {
        info->command = PUT;
        if (parse_helper(info, file_name, fd, num)) { return; } 
        if (help_put(fd) != 0) {
            info->error_type = 2;
            update_epoll_events(fd, EPOLLOUT);
            return;
        }
    }
    if (!strcmp(command, "GET")) {
        info->command = GET;
        if (parse_helper(info, file_name, fd, num)) { return; } 
    }
    if (!strcmp(command, "DELETE")) {
        info->command = DELETE;
        if (parse_helper(info, file_name, fd, num)) { return; } 
    }
    if (!strcmp(command, "LIST")) {
        info->command = LIST;
        if (num > 1) {
            info->error_type = 1;
            update_epoll_events(fd, EPOLLOUT);
            return;
        }
    }
    if (!info->command) {
        info->error_type = 1;
        update_epoll_events(fd, EPOLLOUT);
        return;
    }
    // Parsed successfully, ready to execute
    info->ready_parse = 0;
    info->ready_exec = 1;
    update_epoll_events(fd, EPOLLOUT);
}

void process_client(client_info* info, int fd) {
    if (info->ready_exec == 1) {
        if (info->command == PUT) { write_all_to_socket(fd, OK, OK_SIZE); }
        if (info->command == LIST) { help_list(fd); }
        if (info->command == DELETE) { help_delete(fd); }
        if (info->command == GET) { help_get(fd); }
    } else if (info->error_type > 0) {
        write_all_to_socket(fd, ERROR, ERROR_SIZE);
        if (info->error_type == 1) { write_all_to_socket(fd, err_bad_request, 12); }
        if (info->error_type == 2) { write_all_to_socket(fd, err_bad_file_size, 14); }
        if (info->error_type == 3) { write_all_to_socket(fd, err_no_such_file, 13); }
    }
    epoll_ctl(epoll, 2, fd, NULL);
    shutdown(fd, SHUT_RDWR);
    close(fd);
    free(dictionary_get(connect_status_dict, &fd));
    dictionary_remove(connect_status_dict, &fd);
}

void send_request(int sock_fd) {
    int optval = 1;
    int retval_addr = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
    if (retval_addr == -1) {
        perror("setsockopt retval_addr");
        exit(EXIT_FAILURE);
    }
    int retval_port = setsockopt(sock_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
    if (retval_port == -1) {
        perror("setsockopt retval_port");
        exit(EXIT_FAILURE);
    }
}

void run_server(char* port) {
    struct addrinfo hints, *result;
    memset(&hints, 0, sizeof(struct addrinfo));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    int s = getaddrinfo(NULL, port, &hints, &result);
    if (s != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
        exit(EXIT_FAILURE);
    }
    int listen_sock = socket(AF_INET, SOCK_STREAM, 0);
    send_request(listen_sock);
    // Bind and listen
    if (bind(listen_sock, result->ai_addr, result->ai_addrlen) != 0) {
        perror("bind()");
        exit(EXIT_FAILURE);
    }
    if (listen(listen_sock, 128) != 0) {
        perror("listen()");
        exit(EXIT_FAILURE);
    }
    freeaddrinfo(result);
    // From epoll example
    epoll = epoll_create1(0);
    if (epoll == -1) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    struct epoll_event ev;
    memset(&ev, '\0', sizeof(struct epoll_event));
    // This file object will be `read` from (connect is technically a read operation)
    ev.events = EPOLLIN;
    ev.data.fd = listen_sock;
    // Add the socket in with all the other fds. Everything is a file descriptor
    if (epoll_ctl(epoll, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        exit(EXIT_FAILURE);
    }
    // Then in a loop, we wait and see if epoll has any events.
    do {
        struct epoll_event events[MAX_EVENTS];
        int nfds = epoll_wait(epoll, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait()");
            exit(EXIT_FAILURE);
        }
        for (struct epoll_event* event = events; event < events + nfds; ++event) {
            if (event->data.fd != listen_sock) {
                client_info* info = dictionary_get(connect_status_dict, &event->data.fd);
                info->ready_parse = 1;
                parse_header(event->data.fd);
                process_client(info, event->data.fd);
            } else {
                int conn_sock = accept(listen_sock, NULL, NULL);
                if (conn_sock == -1) {
                    perror("accept()");
                    exit(EXIT_FAILURE);
                }
                struct epoll_event ev_conn;
                memset(&ev_conn, '\0', sizeof(struct epoll_event));
                ev_conn.events = EPOLLIN | EPOLLET;
                ev_conn.data.fd = conn_sock;
                epoll_ctl(epoll, EPOLL_CTL_ADD, conn_sock, &ev_conn);
                client_info* info = malloc(sizeof(client_info));
                dictionary_set(connect_status_dict, &conn_sock, info);
            }
        }
    } while (1);
}

int main(int argc, char **argv) {
    // good luck!
    if (argc != 2) {
        print_server_usage();
        exit(EXIT_FAILURE);
    }
    signal(SIGPIPE, signal_handler);
    struct sigaction sigint_act;
    memset(&sigint_act, '\0', sizeof(struct sigaction));
    sigint_act.sa_handler = disconnect_server;
    if (sigaction(SIGINT, &sigint_act, NULL) == -1) {
        perror("sigaction fail");
        exit(EXIT_FAILURE);
    }
    char template[] = "XXXXXX";
    directory = mkdtemp(template);
    print_temp_directory(directory);
    connect_status_dict = int_to_shallow_dictionary_create();
    file_size_dict = string_to_unsigned_long_dictionary_create();
    file_name_vec = string_vector_create();
    run_server(argv[1]); 
}
