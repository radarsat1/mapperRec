
#include <stdlib.h>
#include <stdio.h>
#include <sys/param.h>
#include <string.h>
#include <sys/time.h>
#include <lo/lo.h>

#include "backend_binary.h"
#include "command.h"

backend_binary_options_t backend_binary_options;

static FILE* output_file = 0;

static lo_timetag last_write = {0,0};

void binary_defaults()
{
    memset(&backend_binary_options, 0,
           sizeof(backend_binary_options));
    backend_binary_options.file_path = 0;
}

int binary_start()
{
    if (!backend_binary_options.file_path) {
        printf("No output filename specified.\n");
        return 1;
    }

    output_file = fopen(backend_binary_options.file_path, "ab");

    if (!output_file) {
        printf("Error opening file `%s' for writing.\n",
               backend_binary_options.file_path);
        return 1;
    }

    return 0;
}

void binary_stop()
{
    if (output_file)
    {
        fclose(output_file);
        output_file = 0;
    }
}

int binary_poll()
{
    return 0;
}

/* TODO: Bundle messages together that happen in the same call to poll(). */
void binary_write_value(mapper_signal msig, void *v)
{
    char str[1024], *path = str;
    msig_full_name(msig, path, 1024); 

    if (path[0]=='/')
        path ++;

    while (path[0] && path[0]!='/')
        path ++;

    lo_timetag now;
    lo_timetag_now(&now);

    mapper_db_signal mprop = msig_properties(msig);

    fwrite(&now, sizeof(lo_timetag), 1, output_file);

    int len = strlen(path), wrote=len, i;
    len = (len / 4 + 1) * 4;
    int wlen = lo_htoo32(len);
    fwrite(&wlen, 4, 1, output_file);
    fwrite(path, wrote, 1, output_file);
    while (wrote < len) {
        fwrite("", 1, 1, output_file);
        wrote ++;
    }

    fwrite(&mprop->type, 1, 1, output_file);
    wlen = lo_htoo32(mprop->length);
    fwrite(&wlen, 4, 1, output_file);

    if (mprop->type == 'i' || mprop->type == 'f') {
        for (i=0; i<mprop->length; i++) {
            int wi = lo_htoo32(((uint32_t*)v)[i]);
            fwrite(&wi, 4, 1, output_file);
        }
    }

    fflush(output_file);

    if (now.sec > last_write.sec) {
        printf(".");
        fflush(stdout);
        last_write = now;
    }
}
