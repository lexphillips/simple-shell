#ifndef SSI_H
#define SSI_H

#define _POSIX_C_SOURCE 200112L
#include <unistd.h>
#include <limits.h>

typedef enum {
    STATUS_CONTINUE = 0,
    STATUS_EXIT_SUCCESS = 1,
    STATUS_EXIT_FAILURE = 2
} shell_status;

typedef shell_status (*builtin_func)(char **argv);

typedef struct builtin_command {
    const char *name;
    builtin_func func;
} builtin_command;

typedef struct bg_process {
    pid_t pid;
    char *command;
    char *cwd;
    struct bg_process *next;
} bg_process;

// Externs
extern bg_process *bg_head;

// Function declarations
shell_status cmd_exit(char **argv);
shell_status cmd_cd(char **argv);
shell_status cmd_pwd(char **argv);
shell_status cmd_bg(char **argv);
shell_status cmd_bglist(char **argv);

builtin_func get_builtin(const char *name);

char *generate_prompt();
char **tokenize(char *input);
void safe_free(void **ptr);
void build_full_command(char **argv, char *buff, size_t buff_size);

pid_t create_process(char *cmd, char **argv, int is_bg);
shell_status run_command(char *cmd, char **argv, int is_bg);

void reap_bg_processes();
void cleanup();

#endif
