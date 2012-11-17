
#include "backend.h"

const char *backend_strings[N_BACKENDS] = { "file", "binary", "oscstreamdb" };

backends_t backend = BACKEND_FILE;

int (*backend_start)() = 0;
void (*backend_stop)() = 0;
int (*backend_poll)() = 0;
void (*backend_write_value)(mapper_signal msig, void *v,
                            mapper_timetag_t *tt) = 0;
void (*backend_write_generic)(const char *path,
                              const char *types,
                              lo_message m) = 0;
