
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <sys/ioctl.h>

#include "include/builtins.h"

int clean() {
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == -1) {
        perror("ioctl ");
        return 1;
    }

    for (uint16_t i = 0; i < w.ws_row; ++i)
        printf("\r\033[A");
    for (uint16_t i = 0; i < w.ws_row; ++i)
        for (uint16_t j = 0; j < w.ws_col; ++j)    
            printf(" ");
    for (uint16_t i = 0; i < w.ws_row; ++i)
        printf("\r\033[A");

    return 0;
}

int cd(char *dir) {
    if (!chdir(dir))
        return 0;

    perror("Directory doesn't exist ");
    return 1;
}
