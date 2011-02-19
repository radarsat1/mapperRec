/* Program that listens for libmapper output signals and allows
 * creating a corresponding input signal, maps it directly, and sends
 * the values straight to oscstreamdb. */

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#include "command.h"
#include "recmonitor.h"
#include "recdevice.h"
#include "backend_file.h"
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
void (*backend_write_value)(mapper_signal msig, void *v);

void help()
{
    printf("recmapper -s <stream name> -m <mapper device> "
                     "[-d <database string>]\n"
           "          [-b <backend=file,oscstreamdb>] "
                     "[-r <path to oscsstreamdb>]\n"
           "          [-f <output file>]\n");
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
            {"device",      required_argument, 0, 'm'},
            {"file",        required_argument, 0, 'f'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long (argc, argv, "hd:s:b:r:m:f:",
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

        case 'm':
            device_name = optarg;
            break;

        case 'f':
            backend_file_options.file_path = optarg;
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
    int rc=0;

    oscstreamdb_defaults();

    if (cmdline(argc, argv))
        return 1;

    if (!device_name) {
        printf("You must specify a device name to record. (-m)\n");
        return 1;
    }

    switch (backend) {
    case BACKEND_FILE:
        backend_start = file_start;
        backend_stop = file_stop;
        backend_poll = file_poll;
        backend_write_value = file_write_value;
        break;
    case BACKEND_OSCSTREAMDB:
        if (backend_oscstreamdb_options.stream==0) {
            help();
            return 1;
        }
        backend_start = oscstreamdb_start;
        backend_stop = oscstreamdb_stop;
        backend_poll = oscstreamdb_poll;
        backend_write_value = oscstreamdb_write_value;
        break;
    default:
        printf("Unknown backend selected.\n");
        return 1;
    }

    if (backend_start()) {
        printf("Error starting backend.\n");
        rc = 1;
        goto done;
    }

    if (recmonitor_start()) {
        printf("Error starting monitor.\n");
        rc = 1;
        goto done;
    }

    if (recdevice_start()) {
        printf("Error starting device.\n");
        rc = 1;
        goto done;
    }

    while (!(backend_poll() || command_poll())) {
        recmonitor_poll();
        recdevice_poll();
        usleep(100000);
    }

  done:
    printf("Exiting.\n");

    recmonitor_stop();
    recdevice_stop();
    backend_stop();

    return rc;
}
