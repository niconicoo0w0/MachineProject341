/**
 * shell
 * CS 341 - Spring 2023
 */

#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"

#include <signal.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

#define PROC_CONT "running"
#define PROC_KILL "killed"
#define PROC_STOP "stopped"

typedef struct process {
    char *command;
    pid_t pid;
    char *status;
} process;


static vector* vec;
static vector* commands;
static vector* start;
static vector* processes;
static FILE* file_ = NULL;
static int hist_check = 0;
static int script_check = 0;
static pid_t curr_pid = 0;

//return current local time
char* curr_time() {
    char* start_str = malloc(sizeof(char) * 6);
    FILE *uptime_file = fopen("/proc/stat", "r");
    if (!uptime_file) {
        print_redirection_file_error();
        free(start_str);
        exit(1);
    }
    long long boot_time;
    fscanf(uptime_file, "btime %lld", &boot_time);
    fclose(uptime_file);
    time_t start_seconds = time(NULL);
    struct tm *start_tm = localtime(&start_seconds);
    time_struct_to_string(start_str, 6, start_tm);
    return start_str;
}

//background helper
//return 0 if success, -1 if failed
int switch_status(pid_t pid, char* status) {
    for (size_t i = 0; i < vector_size(processes); i++) {
        process *p = vector_get(processes, i);
        if (p->pid == pid) {
            if (strcmp(p->status, PROC_KILL) != 0) {
                p->status = status;
                return 0;
            }
        }
    }
    return -1;
}

void add_process(pid_t pid, char* command) {
    for (size_t i = 0; i < vector_size(processes); i++) {
        process *p = vector_get(processes, i);
        if (p->pid == pid) {
            p->status = PROC_CONT;
            vector_push_back(start, curr_time());
            vector_push_back(commands, command);
            return;
        }
    }
    process *p = malloc(sizeof(process));
    p->status = PROC_CONT;
    p->pid = pid;
    vector_push_back(start, curr_time());
    vector_push_back(commands, command);
    vector_push_back(processes, p);
    return;
}

//ps helper
process_info* get_pinfo(process* p, char* command, char* start_str) {
    //copy and initialize
    process_info *pinfo = malloc(sizeof(process_info));
    pinfo->pid = p->pid;
    pinfo->command = strdup(command);
    pinfo->start_str = strdup(start_str);
    pinfo->time_str = malloc(sizeof(char) * 5);
    char proc_stat_path[50];
    sprintf(proc_stat_path, "/proc/%d/stat", p->pid);
    FILE *proc_stat_file = fopen(proc_stat_path, "r");
    if (!proc_stat_file) {
        print_redirection_file_error();
        exit(1);
    }
    unsigned long utime, stime;
    fscanf(proc_stat_file, "%*d %*s %c %*d %*d %*d %*d %*d %*u %*lu %*lu %*lu %*lu %lu %lu %*ld %*ld %*ld %*ld %ld %*ld %*llu %lu",
           &pinfo->state, &utime, &stime, &pinfo->nthreads, &pinfo->vsize);
    fclose(proc_stat_file);
    unsigned long total_time = utime + stime;
    unsigned long seconds = total_time / sysconf(_SC_CLK_TCK);
    size_t minutes = seconds / 60;
    seconds = seconds % 60;
    execution_time_to_string(pinfo->time_str, 5, minutes, seconds);
    pinfo->vsize /= 1024;
    return pinfo;
}

//return 1 if need write
int system_(char *command, int logic) {
    if (strcmp(command, "exit") != 0 && !logic) {
        vector_push_back(vec, command);
    }
    int status;
    pid_t pid = fork();
    add_process(pid, command);
    curr_pid = pid;
    if (pid == 0) {
        if (command[strlen(command)-1] == '&') {
            //remove &
            command[strlen(command)-1] = '\0';
            if (setsid() == -1) {
                print_setpgid_failed();
                return -1;
            }
        }
        if (execl("/bin/sh", "sh", "-c", command, (char *)0) == -1) {
            print_exec_failed(command);
            return -1;
        }
    }
    if (pid < 0) {
        print_fork_failed();
        return -1;
    }
    if (pid > 0) {
        if (command[strlen(command)-1] != '&') {
            // print_command_executed(pid);
            if (waitpid(pid, &status, 0) != pid) {
                return -1;
            }
            if (WIFEXITED(status)) {
                int exit_status = WEXITSTATUS(status);
                // invalid command
                if (exit_status == 127) {
                    print_invalid_command(command);
                    return -1;
                }
                // make failed
                if (exit_status != 0) {
                    print_invalid_command(command);
                    return -1;
                }
            } 
            print_command_executed(pid);
        } else {
            if (!logic) {
                waitpid(pid, &status, WNOHANG);
                print_command_executed(pid);
                return 0;
            }
        }
    }
    return 0;
}

int logic_type(char* command) {
    int and = 0;
    int or = 0;
    int separator = 0;
    if (strstr(command, "&&")) {
        and++;
    }
    if (strstr(command, "||")) {
        or++;
    }
    if (strstr(command, ";")) {
        separator++;
    }
    if ((and == 1) && (or+separator) == 0) {
        return 0;
    }
    if ((or == 1) && (and+separator) == 0) {
        return 1;
    }
    if ((separator == 1) && (or+and) == 0) {
        return 2;
    }
    return -1;
}

int command_type(char* command) {
    if (logic_type(command) != -1) {
        return 4;
    }
    if (strncmp(command,"cd ", 3) == 0 || strcmp(command, "cd") == 0) {
        return 0;
    }
    if (strncmp(command, "!history", 8) == 0) {
        return 1;
    }
    if (command[0] == '#' && strlen(command) > 1) {
        return 2;
    }
    if (command[0] == '!') {
        return 3;
    }
    if (strncmp(command, "ps", 2) == 0 && strlen(command) == 3) {
        return 5;
    }
    if (strncmp(command, "stop", 4) == 0) {
        return 6;
    }
    if (strncmp(command, "cont", 4) == 0) {
        return 7;
    }
    if (strncmp(command, "kill", 4) == 0) {
        return 8;
    }
    return -1;
}

char* trim_command(char* command) {
    int is_space = 1;
    for (int i = 0; command[i] != '\0'; i++) {
        if (!isspace(command[i])) {
            is_space = 0;
        }
    }
    if (is_space == 1) {
        if (command[strlen(command)-1] == '\n') {
            command[strlen(command)-1] = '\0';
        }
        return command;
    }
    size_t i, j;
    size_t len = strlen(command);
    for (i = 0; i < len && isspace(command[i]); i++);
    for (j = len - 1; j >= 0 && isspace(command[j]); j--);
    len = j - i + 1;
    memmove(command, command + i, len);
    command[len] = '\0';
    return command;
}

int run_command(char* command, char* history_file, int logic);

int run_command_with_logic(char *command, char *history_file) {
    // -1 => NULL
    // 0 => &&
    // 1 => ||
    // 2 => ;
    if (!command || strlen(command) < 1) {
        return -1;
    }
    vector_push_back(vec, command);
    command = trim_command(command);
    int type = logic_type(command);
    // *NULL*
    if (type == -1) {
        return -1;
    }
    char* token = NULL;
    int status = 0;
    // *and*
    if (type == 0) {
        status = 1;
        token = strtok(command, "&&");
        while(token) {
            status = run_command(token, history_file, 1);
            if (status == -1) {
                break;
            }
            token = strtok(NULL, "&&");
        }
    }
    // *or*
    if (type == 1) {
        token = strtok(command, "||");
        status = -1;
        while(token) {
            status = run_command(token, history_file, 1);
            if (status != -1) {
                break;
            }
            token = strtok(NULL, "||");
        }
    }
    // *sprt*
    if (type == 2) {
        token = strtok(command, ";");
        while(token) {
            status = run_command(token, history_file, 1);
            token = strtok(NULL, ";");
        }
    }
    return status;
}


//return 0 if success
//return -1 if failed
int run_command(char* command, char* history_file, int logic) {
    if (!command || strlen(command) < 1) {
        return -1;
    }
    int type = command_type(command);
    ssize_t size = vector_size(vec);
    //cd
    if (type == 0) {
        if (command[strlen(command) - 1] == '\n') {
            command[strlen(command) - 1] = '\0';
        }
        if (!logic) { 
            vector_push_back(vec, command); 
        }
        if (strcmp("cd", command) == 0) {
            print_no_directory("");
            return -1;
        }
        char* p = get_full_path(command + 3);
        int result = chdir(p);
        //no such directory
        if (result == -1) {
            for (size_t i = 0; p[i]; i++) {
                if (!isspace(p[i])) {
                    print_no_directory(p);
                    if (p) {
                        free(p);
                    }
                    return -1;
                }
            }
            print_no_directory(command + 3);
            if (p) {
                free(p);
            }
            return -1;
        }
        print_command_executed(curr_pid);
        if (p) {
            free(p);
        }
        return 0;
    }
    command = trim_command(command);
    if (!command || strlen(command) < 1) {
        return -1;
    }
    //!history
    if (type == 1) {
        if (vec) {
            for (ssize_t i = 0; i < size; i++) {
                print_history_line(i, vector_get(vec, i));
            }
        }
        return 0;
    }
    //#n
    if (type == 2) {
        if (size <= 0) {
            print_no_history_match();
            return -1;
        }
        ssize_t i = strtol(command+1, NULL, 10);
        if (i >= size || i < 0) {
            print_invalid_index();
            return -1;
        }
        char* str = vector_get(vec, i);
        char* str_copy = strdup(str);
        print_command(str_copy);
        run_command(str_copy, history_file, 0);
        if (str_copy) {
            free(str_copy);
        }
        return 0;
    }
    //!preflix
    if (type == 3) {
        if (size <= 0) {
            print_no_history_match();
            return -1;
        }
        if (strcmp(command, "!") == 0) {
            if (vector_get(vec, size - 1)) {
                print_command(vector_get(vec, size - 1));
                run_command(vector_get(vec, size - 1), history_file, 0);
                return 0;
            }
        }
        char* copy;
        char* match = strdup(command + 1);
        if (size > 1) {
            for (ssize_t i = size - 1; i >= 0; i--) {
                char* str = strdup(vector_get(vec, i));
                copy = str;
                for (size_t j = 0; j < strlen(match); j++) {
                    if (strncmp(str, match, strlen(match)) == 0) {
                        print_command(str);
                        run_command(str, history_file, 0);
                        if (copy) { free(copy);}
                        if (match) { free(match); }
                        return 0;
                    }
                }
            }
        }
        if (size == 1) {
            char* str = strdup(vector_get(vec, 0));
            copy = str;
            if (strncmp(str, match, strlen(match)) == 0) {
                print_command(str);
                run_command(str, history_file, 0);
                if (copy) { free(copy);}
                if (match) { free(match); }
                return 0;
            }
        } 
        print_no_history_match();
        if (copy) { free(copy);}
        if (match) { free(match); }
        return -1;
    }
    //logic
    if (type == 4) {
        return run_command_with_logic(command, history_file);
    }
    //ps
    if (type == 5) {
        print_process_info_header();
        for (size_t i = 0; i < vector_size(processes); i++) {
            process* p = (process*)vector_get(processes, i);
            if (kill(p->pid, 0) != -1 && strcmp(p->status, PROC_KILL) != 0) {
                process_info* pinfo = get_pinfo(p, vector_get(commands, i), vector_get(start, i));
                print_process_info(pinfo);
            }
        }
        return 0;
    }
    //stop
    if (type == 6) {
        vector_push_back(vec, command);
        //TODO
        pid_t pid = strtol(command + 5, NULL, 10);
        if (!pid) {
            print_invalid_command(command);
            return -1;
        }
        for (size_t i = 0; i < vector_size(processes); i++) {
            process* p = (process*)vector_get(processes, i);
            if (p->pid == pid) {
                if (switch_status(pid, PROC_STOP) == 0) {
                    kill(pid, SIGSTOP);
                    print_stopped_process(p->pid, vector_get(commands, i));
                    switch_status(pid, PROC_STOP);
                    return 0;
                } else {
                    print_no_process_found(pid);
                    return -1;
                }
            }
        }
        print_no_process_found(pid);
        return -1;
    }
    //cont
    if (type == 7) {
        //TODO
        pid_t pid = strtol(command + 5, NULL, 10);
        if (!pid) {
            print_invalid_command(command);
            return -1;
        }
        for (size_t i = 0; i < vector_size(processes); i++) {
            process* p = (process*)vector_get(processes, i);
            if (p->pid == pid) {
                if (switch_status(pid, PROC_CONT) == 0) {
                    kill(pid, SIGCONT);
                    print_continued_process(p->pid, vector_get(commands, i));
                    switch_status(pid, PROC_CONT);
                    return 0;
                } else {
                    print_no_process_found(pid);
                    return -1;
                }
            }
        }
        print_no_process_found(pid);
        return -1;
    } 
    //kill
    if (type == 8) {
        //TODO remove from the vector after killed
        pid_t pid = strtol(command + 5, NULL, 10);
        if (!pid) {
            print_invalid_command(command);
            return -1;
        }
        process* p;
        for (size_t i = 0; i < vector_size(processes); i++) {
            p = (process*)vector_get(processes, i);
            if (p->pid == pid) { 
                if (switch_status(pid, PROC_KILL) == 0) {
                    kill(pid, SIGKILL);
                    print_killed_process(p->pid, vector_get(commands, i));
                    return 0;
                } else {
                    print_no_process_found(pid);
                    return -1;
                }
            }
        }
        print_no_process_found(pid);
        return -1;
    } 
    return system_(command, logic);
}

void signal_handler(int check) {
    pid_t pid;
    int status;
    if (check == SIGCHLD) {
        while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
            //...
        }
    }
}

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    // check valid input
    signal(SIGINT, SIG_IGN);

    // File name!!
    char *history_file = NULL;
    char *script_file = NULL;
    FILE *history_file_;
    FILE *script_file_;
    file_ = stdin;
    
    // Initializing vector
    vec = string_vector_create();
    commands = string_vector_create();
    start = string_vector_create();
    processes = shallow_vector_create();

    // Load Name
    int c;
    while ((c = getopt(argc, argv, "h:f:")) != -1) {
        switch (c) {
        //if c == f, -f is passed to the program
        case 'f':
            if(!script_file && optarg) {
                script_file = strdup(optarg);
                script_check = 1;
                script_file_ = fopen(script_file, "r");
                if (!script_file_) {
                    print_script_file_error();
                    exit(1);
                }
                file_ = script_file_; 
            } else {
                print_usage();
                exit(1);
            }
            break;
        //if c == h, -h is passed to the program
        case 'h':
            if (!history_file && optarg) {
                history_file = strdup(optarg);
                hist_check = 1;
                history_file_ = fopen(history_file, "a");
                if (!history_file_) {
                    print_history_file_error();
                    history_file_ = fopen(history_file, "w+");
                }
                char *line = NULL;
                size_t size;
                while ((getline(&line, &size, history_file_)) != -1) {
                    int len = strlen(line);
                    if (line[len-1] == '\n') {
                        line[len-1] = '\0';
                    }
                    vector_push_back(vec, line);
                }
            } else {
                print_usage();
                exit(1);
            }
            break;
        //Neither
        default:
            print_usage();
            exit(1);
        }
    }

    int len = 0;
    for (int i = 0; i < argc; i++) {
        len += strlen(argv[i]);
    }
    char* result = (char*)malloc(len+argc);
    char* str = strdup(" ");
    pid_t pid = getpid();
    for (int i = 0; i < argc; i++) {
        strcat(result, argv[i]);
        strcat(result, str);
    }
    add_process(pid, result);
    free(result);
    free(str);

    // Exec commands
    int type = -1;
    char cwd[PATH_MAX]; 
    char *command = NULL;
    size_t command_len;
    int count;

    while (1) {
        signal(SIGINT,signal_handler);
        signal(SIGCHLD,signal_handler);
        pid_t prompt_pid = getpid();
        if (curr_pid == 0) {
            curr_pid = prompt_pid;
        }
        curr_pid++;

        //(pid=<pid>)<path>$
        if (getcwd(cwd, sizeof(cwd))) {
            print_prompt(cwd, prompt_pid);
        }
        count = getline(&command, &command_len, file_);
        //EOF
        if (count == -1) {
            if (hist_check == 1) {
                for (size_t i = 0; i < vector_size(vec); i++) {
                    fprintf(history_file_, "%s\n", (char*)vector_get(vec, i));
                }
                fclose(history_file_);
            }
            if (vec) { vector_destroy(vec); }
            if (start) { vector_destroy(start); }
            if (processes) { vector_destroy(processes); }
            if (commands) { vector_destroy(commands); }
            if (script_file) { free(script_file); }
            if (command) { free(command); }
            if (history_file) { free(history_file); }
            exit(0);
        }

        if (command) {
            type = command_type(command);
            run_command(command, history_file, 0);
        }

        //exit => end loop
        if (command) {
            if (strcmp(command, "exit") == 0) {
                if (hist_check == 1) {
                    for (size_t i = 0; i < vector_size(vec); i++) {
                        fprintf(history_file_, "%s\n", (char*)vector_get(vec, i));
                    }
                    fclose(history_file_);
                }
                if (vec) { vector_destroy(vec); }
                if (start) { vector_destroy(start); }
                if (processes) { vector_destroy(processes); }
                if (commands) { vector_destroy(commands); }
                if (script_file) { free(script_file); }
                if (command) { free(command); }
                if (history_file) { free(history_file); }
                exit(0);
            }
        }
    }
    return 0;
}
