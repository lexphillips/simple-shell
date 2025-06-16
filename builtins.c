#define _POSIX_C_SOURCE 200112L
#include "ssi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <limits.h>

static const builtin_command builtins[] = {
    {"exit", cmd_exit},
    {"cd",   cmd_cd},
    {"pwd",  cmd_pwd},
    {"bg",   cmd_bg},
    {"bglist", cmd_bglist},
    {NULL, NULL}
};

/**
 * @brief Retrieves built-in command function.
 * @param name Command name.
 * @return Pointer to the built-in function or NULL if not found.
 */
builtin_func get_builtin(const char *name) {
    for (int i = 0; builtins[i].name != NULL; i++) {
        if (strcmp(name, builtins[i].name) == 0) {
            return builtins[i].func;
        }
    }
    return NULL;
}

/**
 * @brief Handles the exit built-in command.
 * @param argv Argument list.
 * @return Exit status.
 */
shell_status cmd_exit(char **argv) {
    return STATUS_EXIT_SUCCESS;
}

/**
 * @brief Handles the cd built-in command.
 * @param argv Argument list.
 * @return Status code.
 */
shell_status cmd_cd(char **argv) {
    static char prev_dir[PATH_MAX] = "";
    char cwd[PATH_MAX];
    const char *arg = argv[1];

    if (!getcwd(cwd, sizeof(cwd))) {
        perror("cwd");
        return STATUS_CONTINUE;
    }
    if (!arg) {
        arg = getenv("HOME");
        if (!arg) {
            fprintf(stderr, "cd: $HOME not set\n");
            return STATUS_CONTINUE;
        }
    } else if (strcmp(arg, "-") == 0) {
        printf("%s\n", prev_dir);
        arg = prev_dir;
    } else if (arg[0] == '~') {
        const char *home = getenv("HOME");
        if (!home) {
            fprintf(stderr, "cd: $HOME not set\n");
            return STATUS_CONTINUE;
        }
        snprintf(cwd, sizeof(cwd), "%s/%s", home, arg + 1);
        arg = cwd;
    }

    if (chdir(arg) != 0) {
        perror("cd");
        return STATUS_CONTINUE;
    }

    strncpy(prev_dir, cwd, sizeof(prev_dir) - 1);
    prev_dir[sizeof(prev_dir) - 1] = '\0';
    return STATUS_CONTINUE;
}

/**
 * @brief Prints the current working directory.
 * @param argv Argument list.
 * @return Status code.
 */
shell_status cmd_pwd(char **argv) {
    char cwd[PATH_MAX];
    if (!getcwd(cwd, sizeof(cwd))) {
        perror("cwd");
        return STATUS_EXIT_FAILURE;
    }
    printf("%s\n", cwd);
    return STATUS_CONTINUE;
}
