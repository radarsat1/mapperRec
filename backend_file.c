
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include <sys/time.h>
#include <lo/lo.h>

#include "backend_file.h"
#include "command.h"

backend_file_options_t backend_file_options;

FILE* output_file = 0;

lo_timetag last_write = {0,0};

void file_defaults()
{
    memset(&backend_file_options, 0,
           sizeof(backend_file_options));
    backend_file_options.file_path = 0;
}

int file_start()
{
    if (!backend_file_options.file_path) {
        printf("No output filename specified.\n");
        return 1;
    }

    output_file = fopen(backend_file_options.file_path, "a");

    if (!output_file) {
        printf("Error opening file `%s' for writing.\n",
               backend_file_options.file_path);
        return 1;
    }

    return 0;
}

void file_stop()
{
    if (output_file)
    {
        fclose(output_file);
        output_file = 0;
    }
}

int file_poll()
{
    return 0;
}

/* TODO: Bundle messages together that happen in the same call to poll(). */
void file_write_value(mapper_signal msig, void *v)
{
    int i;
    char str[1024], *path = str;
    msig_full_name(msig, path, 1024); 

    if (path[0]=='/')
        path ++;

    while (path[0] && path[0]!='/')
        path ++;

    lo_timetag now;
    lo_timetag_now(&now);

    mapper_db_signal mprop = msig_properties(msig);

    fprintf(output_file, "%u.%u %s %c ",
            now.sec, now.frac, path, mprop->type);

    if (mprop->type == 'i') {
        for (i=0; i<mprop->length; i++)
            fprintf(output_file, "%d", ((int*)v)[i]);
    }
    else if (mprop->type == 'f') {
        for (i=0; i<mprop->length; i++)
            fprintf(output_file, "%g", ((float*)v)[i]);
    }

    fprintf(output_file, "\n");
    fflush(output_file);

    if (now.sec > last_write.sec) {
        printf(".");
        fflush(stdout);
        last_write = now;
    }
}
