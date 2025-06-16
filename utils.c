#include "ssi.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>

#ifndef HOST_NAME_MAX
#define HOST_NAME_MAX 255
#endif

/**
 * @brief Generates shell prompt string.
 * @return Pointer to the prompt string.
 */
char *generate_prompt() {
    char *cwd = getcwd(NULL, 0);
    char *username = getlogin();
    char hostname[HOST_NAME_MAX + 1];
    gethostname(hostname, sizeof(hostname));

    ssize_t size = snprintf(NULL, 0, "%s@%s: %s > ", username, hostname, cwd) + 1;
    char *prompt = malloc(size);
    if (prompt) snprintf(prompt, size, "%s@%s: %s > ", username, hostname, cwd);

    free(cwd);
    return prompt;
}

/**
 * @brief Splits input string into argument tokens.
 * @param input Input string.
 * @return Pointer to tokenized argument list.
 */
char **tokenize(char *input) {
    int count = 0, cap = 8;
    char **argv = malloc(cap * sizeof(char *));
    if (!argv) return NULL;

    char *token = strtok(input, " \t\r\n");
    while (token) {
        if (count >= cap) {
            cap *= 2;
            char **temp = realloc(argv, cap * sizeof(char *));
            if (!temp) {
                free(argv);
                return NULL;
            }
            argv = temp;
        }
        argv[count++] = token;
        token = strtok(NULL, " \t\r\n");
    }
    argv[count] = NULL;
    return argv;
}

/**
 * @brief Safely frees allocated memory.
 * @param ptr Pointer to the allocated memory.
 */
void safe_free(void **ptr) {
    if (ptr && *ptr) {
        free(*ptr);
        *ptr = NULL;
    }
}

/**
 * @brief Constructs a full command string from arguments.
 * @param argv Argument list.
 * @param buff Buffer to store the constructed command.
 * @param buff_size Size of the buffer.
 */
void build_full_command(char **argv, char *buff, size_t buff_size) {
    buff[0] = '\0';
    for (int i = 0; argv[i] != NULL; ++i) {
        strncat(buff, argv[i], buff_size - strlen(buff) - 1);
        if (argv[i + 1] != NULL)
            strncat(buff, " ", buff_size - strlen(buff) - 1);
    }
}

/**
 * @brief Creates a new process and executes the given command.
 * @param cmd Command to execute.
 * @param argv Argument list.
 * @param is_bg Flag indicating if process runs in background.
 * @return Process ID of the child.
 */
pid_t create_process(char *cmd, char **argv, int is_bg) {
    pid_t pid = fork();
    if (pid == 0) {
        signal(SIGINT, SIG_DFL);
        if (is_bg) {
            int fd = open("/dev/null", O_WRONLY);
            if (fd != -1) {
                dup2(fd, STDOUT_FILENO);
                dup2(fd, STDERR_FILENO);
                close(fd);
            }
        }
        execvp(cmd, argv);
        perror(cmd);
        exit(1);
    }
    return pid;
}

/**
 * @brief Executes a command by creating a new process.
 * @param cmd Command to execute.
 * @param argv Argument list.
 * @param is_bg Flag indicating if process runs in background.
 * @return Status code indicating success or failure.
 */
shell_status run_command(char *cmd, char **argv, int is_bg) {
    pid_t pid = create_process(cmd, argv, is_bg);
    if (pid < 0) return STATUS_EXIT_FAILURE;

    if (!is_bg) {
        int status;
        waitpid(pid, &status, 0);
    } else {
        printf("Starting background process: PID %d", pid);
    }
    return STATUS_CONTINUE;
}
