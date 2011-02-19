
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>

#include "backend_oscstreamdb.h"

backend_oscstreamdb_options_t backend_oscstreamdb_options;

FILE* oscstreamdb_process = 0;

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
    char s[1024];
    while (fgets(s, 1024, oscstreamdb_process)) {
        fputs(s, stdout);
        fflush(stdout);
    }
    return feof(oscstreamdb_process);
}
