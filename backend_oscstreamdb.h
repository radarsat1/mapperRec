
#ifndef __BACKEND_OSCSTREAMDB_H__
#define __BACKEND_OSCSTREAMDB_H__

#include <mapper/mapper.h>

typedef struct {
    const char *stream;
    const char *database;
    const char *executable_path;
} backend_oscstreamdb_options_t;

extern backend_oscstreamdb_options_t backend_oscstreamdb_options;

void oscstreamdb_defaults();
int oscstreamdb_start();
void oscstreamdb_stop();
int oscstreamdb_poll();
void oscstreamdb_write_value(mapper_signal msig, void *v,
                             mapper_timetag_t *tt);

#endif // __BACKEND_OSCSTREAMDB_H__
