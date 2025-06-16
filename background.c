#define _POSIX_C_SOURCE 200112L
#define _XOPEN_SOURCE 700
#include "ssi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>

bg_process *bg_head = NULL;

/**
 * @brief Starts a background process.
 * @param argv Argument list.
 * @return Status code.
 */
shell_status cmd_bg(char **argv) {
    if (!argv[1]) {
        fprintf(stderr, "bg: missing command\n");
        return STATUS_CONTINUE;
    }

    char **sub_argv = &argv[1];
    pid_t pid = create_process(sub_argv[0], sub_argv, 1);
    if (pid < 0) return STATUS_EXIT_FAILURE;

    // printf("Starting background process: PID %d\n", pid);

    bg_process *new_node = malloc(sizeof(bg_process));
    if (!new_node) {
        perror("malloc");
        return STATUS_EXIT_FAILURE;
    }

    char full_cmd[PATH_MAX];
    build_full_command(argv, full_cmd, sizeof(full_cmd));
    new_node->pid = pid;
    new_node->command = strdup(full_cmd);
    new_node->cwd = getcwd(NULL, 0);
    new_node->next = bg_head;
    bg_head = new_node;

    return STATUS_CONTINUE;
}

/**
 * @brief Lists active background processes.
 * @param argv Argument list.
 * @return Status code.
 */
shell_status cmd_bglist(char **argv) {
    bg_process *curr = bg_head;
    int count = 0;
    while (curr) {
        printf("%d:  %s/%s\n", curr->pid, curr->cwd, curr->command);
        curr = curr->next;
        count++;
    }
    printf("Total Background jobs:  %d\n", count);
    return STATUS_CONTINUE;
}

/**
 * @brief Reaps terminated background processes.
 */
void reap_bg_processes() {
    pid_t pid;
    int status;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        bg_process **curr = &bg_head;
        while (*curr) {
            if ((*curr)->pid == pid) {
                printf("%d: %s/%s has terminated.\n", pid, (*curr)->cwd, (*curr)->command);
                bg_process *to_free = *curr;
                *curr = (*curr)->next;
                free(to_free->command);
                free(to_free->cwd);
                free(to_free);
                break;
            }
            curr = &(*curr)->next;
        }
    }
}

/**
 * @brief Cleans up background processes before exit.
 */
void cleanup() {
    bg_process *curr = bg_head;
    while (curr) {
        kill(curr->pid, SIGTERM);
        free(curr->command);
        free(curr->cwd);
        bg_process *to_free = curr;
        curr = curr->next;
        free(to_free);
    }
    bg_head = NULL;
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
