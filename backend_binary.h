
#ifndef __BACKEND_BINARY_H__
#define __BACKEND_BINARY_H__

#include <mapper/mapper.h>

typedef struct {
    const char *file_path;
} backend_binary_options_t;

extern backend_binary_options_t backend_binary_options;

void binary_defaults();
int binary_start();
void binary_stop();
int binary_poll();
void binary_write_value(mapper_signal msig, void *v,
                        mapper_timetag_t *tt);

#endif // __BACKEND_BINARY_H__
