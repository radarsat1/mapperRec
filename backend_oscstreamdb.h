
#ifndef __BACKEND_OSCSTREAMDB_H__
#define __BACKEND_OSCSTREAMDB_H__

typedef struct {
    const char *stream;
    const char *database;
    const char *executable_path;
} backend_oscstreamdb_options_t;

extern backend_oscstreamdb_options_t backend_oscstreamdb_options;

int oscstreamdb_start();
void oscstreamdb_stop();
int oscstreamdb_poll();

#endif // __BACKEND_OSCSTREAMDB_H__
