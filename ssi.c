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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <readline/readline.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

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
    fprintf(stdout, "cd action\n");
    return STATUS_CONTINUE;
}

shell_status cmd_pwd(char **argv) {
    fprintf(stdout, "pwd action\n");
    return STATUS_CONTINUE;
}

shell_status cmd_bg(char **argv) {
    fprintf(stdout, "bg action\n");
    return STATUS_CONTINUE;
}

shell_status cmd_bglist(char **argv) {
    fprintf(stdout, "bglist action\n");
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

// To do:
shell_status run_command(char *cmd, char **argv) {
    fprintf(stdout, "Command: %s\nArgs:", cmd);
    for (int i = 0; argv[i]; i++) {
        printf(" [%s]", argv[i]);
    }
    printf("\n");
    return STATUS_CONTINUE;
}

int main() {

    while(1){
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
                status = run_command(argv[0], argv);
            }
        }

        free(argv);
        free(input);

        if(status == STATUS_EXIT_SUCCESS) {
            return EXIT_SUCCESS;
        }else if(status == STATUS_EXIT_FAILURE) {
            return EXIT_FAILURE;
        }
    }
    return EXIT_SUCCESS;
}