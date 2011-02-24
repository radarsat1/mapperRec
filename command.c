
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>

#include "command.h"

int need_prompt = 1;

void prompt()
{
    if (need_prompt) {
        fputs("> ", stdout);
        fflush(stdout);
        need_prompt = 0;
    }
}

int command_poll()
{
    struct timeval t = {0,0};
    prompt();

    int fd = fileno(stdin);
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);

    if (select(fd+1, &set, NULL, NULL, &t)>0) {
        char s[1024];
        if (fgets(s, 1024, stdin)) {
            fputs(s, stdout);
            fflush(stdout);
            need_prompt = 1;
        }
    }
    return feof(stdin);
}
