
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <linux/limits.h>

#include "include/builtins.h"


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
    
    size_t argc = 0;
    char *arg = NULL, **argv = NULL;
    while ((arg = strtok(NULL, " "))) {
        argc += 1;
        argv = realloc(argv, argc);
        argv[argc - 1] = arg;
    }

    return init_command(name, argc, argv);
}

int find_command(command_t *command) {
    if (!strcmp(command->name, "clear")) {
        clean();
    } else if (!strcmp(command->name, "cd")) {
        cd(command->argv[0]);
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
                if (find_command(pc)) {

                }
            }
        } else {
            perror("Couldn't Get Current Working Directory");
            return 1;
        }
    }

    clean();    
    return 0;
}
