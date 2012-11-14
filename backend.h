
#ifndef __BACKEND_H__
#define __BACKEND_H__

#include <mapper/mapper.h>

typedef enum
{
    BACKEND_FILE,
    BACKEND_BINARY,
    BACKEND_OSCSTREAMDB,
    N_BACKENDS
} backends_t;

extern const char *backend_strings[N_BACKENDS];

extern backends_t backend;

extern int (*backend_start)();
extern void (*backend_stop)();
extern int (*backend_poll)();
extern void (*backend_write_value)(mapper_signal msig, void *v,
                                   mapper_timetag_t *tt);

#endif // __BACKEND_H__
