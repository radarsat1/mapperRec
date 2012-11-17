
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include <sys/time.h>
#include <lo/lo.h>

#include "backend_oscstreamdb.h"
#include "command.h"

backend_oscstreamdb_options_t backend_oscstreamdb_options;

FILE* oscstreamdb_process = 0;

lo_address a_write = 0;

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

        if (!a_write && strncmp(s, "Ports:", 6)==0) {
            char *w = strstr(s, "Write RX=");
            if (w) {
                char rx[10];
                snprintf(rx, 10, "%d", atoi(w + 9));
                printf("Creating address for write port %d\n", atoi(rx));
                a_write = lo_address_new("localhost", rx);
            }
        }

        need_prompt = 1;

        FD_ZERO(&set);
        FD_SET(fd, &set);
        t.tv_sec = 0;
        t.tv_usec = 0;
    }

    return feof(oscstreamdb_process);
}

/* TODO: Bundle messages together that happen in the same call to poll(). */
void oscstreamdb_write_value(mapper_signal msig, void *v,
                             mapper_timetag_t *tt)
{
    int i;
    char str[1024], *path = str;
    msig_full_name(msig, path, 1024); 

    if (path[0]=='/')
        path ++;

    while (path[0] && path[0]!='/')
        path ++;

    mapper_db_signal mprop = msig_properties(msig);

    if (!a_write)
        printf("No write port yet. :(\n");
    else {
        lo_bundle b;
        if (!tt || !tt->sec) {
            lo_timetag now;
            lo_timetag_now(&now);
            b = lo_bundle_new(now);
        }
        else
            b = lo_bundle_new(*tt);
        if (!b) {
            printf("Could not create lo_bundle.\n");
            return;
        }

        lo_message m = lo_message_new();
        if (!m) {
            printf("Could not create lo_message.\n");
            lo_bundle_free(b);
            return;
        }

        if (mprop->type == 'i') {
            for (i=0; i<mprop->length; i++)
                lo_message_add_int32(m, ((int*)v)[i]);
        }
        else if (mprop->type == 'f') {
            for (i=0; i<mprop->length; i++)
                lo_message_add_float(m, ((float*)v)[i]);
        }

        lo_bundle_add_message(b, path, m);
        lo_send_bundle(a_write, b);

        lo_bundle_free_messages(b);
    }
}
