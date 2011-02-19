
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include <sys/time.h>

#include "backend_oscstreamdb.h"
#include "command.h"

backend_oscstreamdb_options_t backend_oscstreamdb_options;

FILE* oscstreamdb_process = 0;

void oscstreamdb_defaults()
{
    memset(&backend_oscstreamdb_options, 0,
           sizeof(backend_oscstreamdb_options));
    backend_oscstreamdb_options.executable_path = "oscstreamdb";
    backend_oscstreamdb_options.stream = 0;
    backend_oscstreamdb_options.database = "oscstreamdb";
}

int oscstreamdb_start()
{
    /* Note: We are using "script" here to force oscstreamdb to flush
     * its output more quickly. */

    char cmd[MAXPATHLEN];
    if (backend_oscstreamdb_options.database) {
        snprintf(cmd, MAXPATHLEN, "script -q -c "
                 "\'%s -d \"%s\" -s \"%s\"\' /dev/null",
                 backend_oscstreamdb_options.executable_path,
                 backend_oscstreamdb_options.database,
                 backend_oscstreamdb_options.stream);
    }
    else {
        snprintf(cmd, MAXPATHLEN, "script -q -c "
                 "\'%s -s \"%s\"\' /dev/null",
                 backend_oscstreamdb_options.executable_path,
                 backend_oscstreamdb_options.stream);
    }

    oscstreamdb_process = popen(cmd, "r");

    if (!oscstreamdb_process) {
        printf("Error running oscstreamdb.\n");
        return 1;
    }

    return 0;
}

void oscstreamdb_stop()
{
    if (oscstreamdb_process)
    {
        pclose(oscstreamdb_process);
        oscstreamdb_process = 0;
    }
}

int oscstreamdb_poll()
{
    struct timeval t = {0,0};
    int fd = fileno(oscstreamdb_process);
    fd_set set;
    FD_ZERO(&set);
    FD_SET(fd, &set);

    while (!feof(oscstreamdb_process)
           && select(fd+1, &set, NULL, NULL, &t)>0)
    {
        char s[1024];
        if (fgets(s, 1024, oscstreamdb_process)) {
            fputs(s, stdout);
            fflush(stdout);
        }

        need_prompt = 1;

        FD_ZERO(&set);
        FD_SET(fd, &set);
        t.tv_sec = 0;
        t.tv_usec = 0;
    }

    return feof(oscstreamdb_process);
}

void oscstreamdb_write_value(mapper_signal msig, void *v)
{
    char str[1024], *name = str;
    msig_full_name(msig, name, 1024); 

    if (name[0]=='/')
        name ++;

    while (name[0] && name[0]!='/')
        name ++;

    struct timeval now;
    gettimeofday(&now, NULL);

    printf("Writing %s\n", name);
}
