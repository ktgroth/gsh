
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <pthread.h>
#include <linux/limits.h>

#include "include/builtins.h"


int parallel = 0;

typedef struct {
    char *name;
    
    size_t argc;
    char **argv;
} command_t;

char *read_command() {
    char *command = NULL;
    size_t len = 0;

    size_t nread = getline(&command, &len, stdin);
    if (nread == (size_t)-1) {
        perror("Couldn't Read Command ");
        free(command);
        return NULL;
    }

    return command;
}

command_t *init_command(char *name, size_t argc, char **argv) {
    command_t *command = (command_t *)malloc(sizeof(command_t));
    if (!command) {
        perror("Couldn't Allocate Command ");
        return NULL;
    }

    command->name = name;
    command->argc = argc;
    command->argv = argv;

    return command;
}

command_t *parse_command(char *line) {
    char *name = strtok(line, " ");
    
    size_t argc = 1;
    char *arg = NULL, **argv = calloc(2, sizeof(char *));
    argv[0] = "--";
    while ((arg = strtok(NULL, " "))) {
        if (!strcmp(arg, "&")) {
            parallel = 1;
            break;
        }

        argc += 1;
        argv = realloc(argv, (argc + 2) * sizeof(char *));
        argv[argc - 1] = arg;
    }
    argv[argc] = NULL;

    return init_command(name, argc + 1, argv);
}

void *follow_child(void *arg) {
    int pid = *(int *)arg;
    
    int status;
    waitpid(pid, &status, 0);
    kill(pid, SIGINT);
    printf("{%d} Exitted with status %d\n", pid, status);

    return arg;
}

int run_parallel(command_t *command) {
    int pid = fork();

    if (pid) {
        printf("Child created %d\n", pid);
        if (parallel) {
            pthread_t ct;
            if (pthread_create(&ct, NULL, follow_child, &pid) != 0) {
                perror("Failed to create thread ");
                kill(pid, SIGINT);
                return 1;
            }
            usleep(10000);
        }
    } else {
        printf("Running %s\n", command->name);
        for (size_t i = 0; i < command->argc; ++i)
            printf("%s ", command->argv[i]);

        execvp(command->name, command->argv);        
        perror(" ");
        exit(-1);
    }

    return pid;
}

int find_command(command_t *command) {
    if (!strcmp(command->name, "clear")) {
        return clean();
    } else if (!strcmp(command->name, "cd")) {
        return cd(command->argv[0]);
    } else {
        int pid = run_parallel(command);
        if (pid < 0)
            return pid;

        if (!parallel) {
            int status;
            waitpid(pid, &status, 0);
            printf("Exitted with status %d\n", status);
            
            return status;
        } else
            parallel = 0;
        
        return 0;
    }
}

int main() {
    while (1) {
        char cwd[PATH_MAX];
        if (getcwd(cwd, sizeof(cwd))) {
            printf("%s$> ", cwd);
            char *command = read_command();
            command[strcspn(command, "\n")] = '\0';
            if (!strlen(command)) {
                errno = 22;
                perror("No Such Command ");
            } else {
                command_t *pc = parse_command(command);
                int ec = find_command(pc);

                free(pc->argv);
                free(pc);
            }
        } else {
            perror("Couldn't Get Current Working Directory ");
            return 1;
        }
    }

    return 0;
}
