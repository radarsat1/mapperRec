
#ifndef __BACKEND_FILE_H__
#define __BACKEND_FILE_H__

#include <mapper/mapper.h>

typedef struct {
    const char *file_path;
} backend_file_options_t;

extern backend_file_options_t backend_file_options;

void file_defaults();
int file_start();
void file_stop();
int file_poll();
void file_write_value(mapper_signal msig, void *v);

#endif // __BACKEND_FILE_H__
