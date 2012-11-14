
#include "backend.h"

const char *backend_strings[N_BACKENDS] = { "file", "binary", "oscstreamdb" };

backends_t backend = BACKEND_FILE;

int (*backend_start)();
void (*backend_stop)();
int (*backend_poll)();
void (*backend_write_value)(mapper_signal msig, void *v);
