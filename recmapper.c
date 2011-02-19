/* Program that listens for libmapper output signals and allows
 * creating a corresponding input signal, maps it directly, and sends
 * the values straight to oscstreamdb. */

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "backend_oscstreamdb.h"

typedef enum
{
    BACKEND_FILE,
    BACKEND_OSCSTREAMDB,
    N_BACKENDS
} backends_t;

const char *backend_strings[N_BACKENDS] = { "file", "oscstreamdb" };

backends_t backend = BACKEND_FILE;

int (*backend_start)();
void (*backend_stop)();
int (*backend_poll)();

void help()
{
    printf("recmapper -s <stream name> [-d <database string>] "
           "[-b <backend=file,oscstreamdb>] [-r <path to oscsstreamdb>]\n");
}

int cmdline(int argc, char *argv[])
{
    int c, i;
    while (1)
    {
        static struct option long_options[] =
        {
            {"help",        no_argument,       0, 'h'},
            {"database",    required_argument, 0, 'd'},
            {"stream",      required_argument, 0, 's'},
            {"backend",     required_argument, 0, 'b'},
            {"oscstreamdb", required_argument, 0, 'r'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long (argc, argv, "hd:s:b:r:",
                         long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            break;

        case 'd':
            backend_oscstreamdb_options.database = optarg;
            break;

        case 's':
            backend_oscstreamdb_options.stream = optarg;
            break;

        case 'r':
            backend_oscstreamdb_options.executable_path = optarg;
            break;

        case 'b':
            for (i=0; i<N_BACKENDS; i++)
                if (strcmp(optarg, backend_strings[i])==0) {
                    backend = i;
                    break;
                }
            if (i==N_BACKENDS) {
                printf("Backend `%s' unknown.\n", optarg);
                return 1;
            }
            break;

        case 'h':
            help();
            break;

        default:
            return 1;
        }
    }

    return 0;
}

int main(int argc, char *argv[])
{
    memset(&backend_oscstreamdb_options, 0,
           sizeof(backend_oscstreamdb_options));

    if (cmdline(argc, argv))
        return 1;

    switch (backend) {
    case BACKEND_FILE:
        printf("File backend not yet implemented.\n");
        return 1;
    case BACKEND_OSCSTREAMDB:
        if (backend_oscstreamdb_options.stream==0) {
            help();
            return 1;
        }
        backend_start = oscstreamdb_start;
        backend_stop = oscstreamdb_stop;
        backend_poll = oscstreamdb_poll;
        break;
    }

    if (backend_start()) {
        printf("Error starting backend.\n");
        return 1;
    }

    while (!backend_poll())
        sleep(1);

    backend_stop();

    return 0;
}
