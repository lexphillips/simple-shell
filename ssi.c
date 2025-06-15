/**
 * @file ssi.c
 * @brief A simple shell interpreter
 *
 * 
 * @author lexph
 * @date June 14, 2025
 * 
 * @section Acknowledgment
*/

#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 700
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <string.h>
#include <sys/wait.h>
#include <readline/readline.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

pid_t create_process(char *cmd, char **argv, int is_bg);
void build_full_command(char **argv, char *buff, size_t buff_size);

typedef enum {
    STATUS_CONTINUE = 0,
    STATUS_EXIT_SUCCESS = 1,
    STATUS_EXIT_FAILURE = 2
}shell_status;

typedef shell_status (*builtin_func)(char **argv);

typedef struct builtin_command {
    const char *name;
    builtin_func func;
}builtin_command;


typedef struct bg_process {
    pid_t pid;
    char *command;
    char *cwd;
    struct bg_process *next;
}bg_process;

bg_process *bg_head = NULL;

// Built-in cmd prototypes
shell_status cmd_exit(char **argv);
shell_status cmd_cd(char **argv);
shell_status cmd_pwd(char **argv);
shell_status cmd_bg(char **argv);
shell_status cmd_bglist(char **argv);

// Built-in cmd table
const builtin_command builtins[] = {
    {"exit", cmd_exit},
    {"cd", cmd_cd},
    {"pwd", cmd_pwd},
    {"bg", cmd_bg},
    {"bglist", cmd_bglist},
    {NULL, NULL}
};

builtin_func get_builtin(const char *name) {
    for(int i=0; builtins[i].name != NULL; i++) {
        if(strcmp(name, builtins[i].name) == 0) {
            return builtins[i].func;
        }
    }
    return NULL;
}

shell_status cmd_exit(char **argv) {
    return STATUS_EXIT_SUCCESS;
}

// To do:
shell_status cmd_cd(char **argv) {
    static char prev_dir[PATH_MAX] = "";
    char cwd[PATH_MAX];
    const char* arg = argv[1];

    if(!getcwd(cwd, sizeof(cwd))) {
        perror("cwd");
        return STATUS_CONTINUE;
    }
    // No argument
    if(!arg) {
        arg = getenv("HOME");
        if(!arg) {
            fprintf(stderr, "cd: $HOME not set\n");
            return STATUS_CONTINUE;
        }
    }
    // Argument starts with dash(-) -> go to previous directory
    if(arg[0] == '-' && (!arg[1] || arg[1] == '\0')) {
        printf("%s\n", prev_dir);
        arg = prev_dir;
    }
    // Argument starts with tilde(~) -> go to root directory
    char path[PATH_MAX];
    if(arg[0] == '~') {
        const char *home_dir = getenv("HOME");
        if(!home_dir) {
            fprintf(stderr, "cd: $HOME not set\n");
            return STATUS_CONTINUE;
        }
        if (home_dir[strlen(home_dir) - 1] == '/') {
            snprintf(path, sizeof(path), "%s%s", home_dir, arg + 1);
        }else {
            snprintf(path, sizeof(path), "%s/%s", home_dir, arg + 1);
        }
        arg = path;
    }
    if(chdir(arg) !=0 ) {
        perror("cd");
        return STATUS_CONTINUE;
    }
    strncpy(prev_dir, cwd, sizeof(prev_dir) - 1);
    prev_dir[sizeof(prev_dir) - 1] = '\0';

    return STATUS_CONTINUE;
}

shell_status cmd_pwd(char **argv) {
    char cwd[PATH_MAX];
    if(!getcwd(cwd, sizeof(cwd))) {
        perror("cwd");
        return STATUS_EXIT_FAILURE;
    }
    printf("%s\n", cwd);
    return STATUS_CONTINUE;
}

shell_status cmd_bg(char **argv) {
    if(!argv[1]) {
        fprintf(stderr, "bg: missing command\n");
        return STATUS_CONTINUE;
    }
    char **sub_argv = &argv[1];
    pid_t pid = create_process(sub_argv[0], sub_argv, 1);
    if(pid < 0) return STATUS_EXIT_FAILURE;
    
    printf("Starting background process: PID %d\n", pid); // REMOVE BEFORE SUBMIT

    bg_process *new_node = malloc(sizeof(bg_process));
    if(!new_node) {
        perror("new_node malloc");
        return STATUS_EXIT_FAILURE;
    }
    new_node->pid = pid;
    char full_cmd[PATH_MAX];
    build_full_command(argv, full_cmd, sizeof(full_cmd));
    new_node->command = strdup(full_cmd);
    char *cwd = getcwd(NULL, 0);
    new_node->cwd = cwd ? cwd : strdup("<cwd-failed>");

    new_node->next = bg_head;
    bg_head = new_node;

    return STATUS_CONTINUE;
}

shell_status cmd_bglist(char **argv) {
    bg_process *curr = bg_head;
    int count = 0;
    while(curr) {
        printf("%d:  %s/%s\n", curr->pid, curr->cwd, curr->command);
        curr = curr->next;
        count++;
    }
    printf("Total Background jobs:  %d\n", count);
    return STATUS_CONTINUE;
}


static inline void safe_free(void **ptr) {
    if(ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

char *generate_prompt() {
    char *prompt = NULL;
    char *cwd = getcwd(NULL, 0);
    if(!cwd) {
        perror("getcwd");
        goto cleanup;
    }

    char *username = getlogin();
    if(!username) {
        perror("getlogin");
        goto cleanup;
    }

    char hostname[HOST_NAME_MAX+1];
    if(gethostname(hostname, sizeof(hostname)) != 0) {
        perror("gethostname");
        goto cleanup;
    }

    ssize_t size = snprintf(NULL, 0, "%s@%s: %s > ", username, hostname, cwd) + 1;
    prompt = malloc(size);
    if(!prompt) {
        fprintf(stderr, "Failed to allocate memory");
        goto cleanup;
    }
    snprintf(prompt, size, "%s@%s: %s > ", username, hostname, cwd);

    // Error handle and return
    cleanup: 
    safe_free((void**)&cwd);
    return prompt;
}

char **tokenize(char *input) {
    int count = 0;
    int cap = 8;

    char **argv = malloc(sizeof(char*) * cap);
    if(!argv) {
        perror("argv malloc");
        return NULL;
    }

    char *token = strtok(input, " \r\t\n");
    while(token != NULL) {
        if(count >= cap) {
            cap *= 2;
            char **temp= realloc(argv, sizeof(char *) * cap);
            if(!temp) {
                free(argv);
                perror("argv realloc");
                return NULL;
            }
            argv = temp;
        }
        argv[count++] = token;
        token = strtok(NULL, " \r\t\n");
    }
    argv[count] = NULL;
    return argv;
}

void build_full_command(char **argv, char *buff, size_t buff_size) {
    buff[0] = '\0'; 

    for (int i = 0; argv[i] != NULL; ++i) {
        strncat(buff, argv[i], buff_size - strlen(buff) - 1);
        if (argv[i + 1] != NULL) {
            strncat(buff, " ", buff_size - strlen(buff) - 1); 
        }
    }
}

pid_t create_process(char *cmd, char **argv, int is_bg) {
    pid_t pid = fork();
    if(pid < 0) { // fork error
        perror("fork");
        return -1;
    }else if(pid == 0){ // child process
        signal(SIGINT, SIG_DFL);
        if(is_bg) { // redirect output
            int fd = open("/dev/null", O_WRONLY);
            if(fd != -1) {
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }


        }

        execvp(argv[0], argv);
        perror(argv[0]);
        exit(1);
    }
    return pid;
}

// To do:
shell_status run_command(char *cmd, char **argv, int is_bg) {
    // Create a new process, copy of the original process    
    pid_t pid = create_process(cmd, argv, 0);
    if(pid < 0) return STATUS_EXIT_FAILURE;

    if(is_bg == 0) {
        int status;
        waitpid(pid, &status, 0);
    }else {
        printf("Starting background process: PID %d", pid);
    }
    return STATUS_CONTINUE;
}

void reap_bg_processes() {
    pid_t pid_to_reap;
    int status;

    while((pid_to_reap = waitpid(-1, &status, WNOHANG)) > 0) {
        bg_process **curr = &bg_head;
        while(*curr) {
            if((*curr)->pid == pid_to_reap) {
                printf("%d: %s/%s has terminated.\n", pid_to_reap, (*curr)->cwd, (*curr)->command);                
                bg_process *to_free = *curr;
                *curr = (*curr)->next;
                free(to_free->command);
                free(to_free);
                break;
            }
            else {
                curr = &((*curr)->next);
            }
        }
    }
}

void cleanup() {
    bg_process *curr = bg_head;
    while(curr) {
        kill(curr->pid, SIGTERM);
        free(curr->command);
        free(curr->cwd);
        bg_process *to_free = curr;
        curr = curr->next;
        free(to_free);
    }
    bg_head = NULL;

    while(waitpid(-1, NULL, WNOHANG) > 0);
}

int main() {
    signal(SIGINT, SIG_IGN);
    while(1){
        reap_bg_processes();

        char *prompt = generate_prompt();
        if(!prompt) {
            return EXIT_FAILURE;
        }
        char *input = readline(prompt);
        safe_free((void**)&prompt);
        if (!input) {
            printf("\n");
            return EXIT_SUCCESS;
        }

        shell_status status = STATUS_CONTINUE;
        char **argv = tokenize(input);
        if(argv && argv[0]) {
            builtin_func func = get_builtin(argv[0]);
            if(func) {
                status = func(argv);
            }else {
                status = run_command(argv[0], argv, 0);
            }
        }

        free(argv);
        free(input);

        if(status == STATUS_EXIT_SUCCESS) {
            cleanup();
            return EXIT_SUCCESS;
        }else if(status == STATUS_EXIT_FAILURE) {
            cleanup();
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}