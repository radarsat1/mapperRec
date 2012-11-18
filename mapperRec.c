/* Program that listens for libmapper output signals and allows
 * creating a corresponding input signal, maps it directly, and sends
 * the values straight to oscstreamdb. */

#include <getopt.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <libgen.h>

#include "mapperRec.h"
#include "backend.h"
#include "command.h"
#include "recmonitor.h"
#include "recdevice.h"
#include "backend_text.h"
#include "backend_binary.h"
#include "backend_oscstreamdb.h"
#include "playback.h"

int done = 0;
int playback_mode = 0;

void help()
{
    printf("mapperRec -d <mapper device> [-b <backend=text,binary,oscstreamdb>]\n"
           "          [-k <backend options>] "
                     "[-o <output file>]\n"
           "          [-s <mapper or OSC signal to match>]\n"
           "          [-p <playback mode>] "
                     "[-v] [-h]\n\n"
           "Backend-specific options are comma-separated key=value pairs.\n"
           "Options are,\n"
           "  OSCStreamDB: \"path\" (to oscsstreamdb executable), "
                          "\"database\", \"stream\"\n");
}

int parse_options(const char *optarg)
{
    char *opts = alloca(strlen(optarg));
    strcpy(opts, optarg);
    char *p, *s = strtok_r(opts, ",", &p);
    while (s)
    {
        char *q, *t = strtok_r(s, "=", &q);
        t = strtok_r(0, "=", &q);
        if (strncmp(s, "path", 4)==0
            && backend==BACKEND_OSCSTREAMDB)
        {
            backend_oscstreamdb_options.executable_path = strdup(t);
        }
        else if (strncmp(s, "database", 8)==0
                 && backend==BACKEND_OSCSTREAMDB)
        {
            backend_oscstreamdb_options.database = strdup(t);
        }
        else if (strncmp(s, "stream", 6)==0
                 && backend==BACKEND_OSCSTREAMDB)
        {
            backend_oscstreamdb_options.stream = strdup(t);
        }
        else {
            printf("Unknown option `%s' for %s backend.\n", s,
                backend_strings[backend]);
            return 1;
        }
        s = strtok_r(0, ",", &p);
    }
    return 0;
}

int cmdline(int argc, char *argv[])
{
    int c, i;
    while (1)
    {
        static struct option long_options[] =
        {
            {"help",        no_argument,       0, 'h'},
            {"version",     no_argument,       0, 'v'},
            {"options",     required_argument, 0, 'k'},
            {"backend",     required_argument, 0, 'b'},
            {"device",      required_argument, 0, 'd'},
            {"output",      required_argument, 0, 'o'},
            {"signal",      required_argument, 0, 's'},
            {"playback",    no_argument, 0, 'p'},
            {0, 0, 0, 0}
        };
        int option_index = 0;

        c = getopt_long (argc, argv, "hvd:b:k:d:o:s:p",
                         long_options, &option_index);
        if (c == -1)
            break;

        switch (c)
        {
        case 0:
            break;

        case 'k':
            if (parse_options(optarg))
                exit(1);
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

        case 'd':
            recmonitor_add_device_string(optarg);
            break;

        case 's':
            recmonitor_add_signal_string(optarg);
            break;

        case 'o':
            backend_text_options.file_path = optarg;
            backend_binary_options.file_path = optarg;
            break;

        case 'p':
            // Toggle playback mode.
            // i.e., if started as mapperPlay, turns playback mode off.
            playback_mode = !playback_mode;
            break;

        case 'v':
            printf("mapperRec v%s, (C) " __DATE__ " Stephen Sinclair; http://idmil.org\n", PACKAGE_VERSION);
            exit(0);

        case 'h':
            help();
            exit(0);

        default:
            return 1;
        }
    }

    return 0;
}

// If we are executed as mapperPlay, set playback mode by default.
void init_playmode_default(const char *str)
{
    char *s = alloca(strlen(str));
    strcpy(s, str);
    if (strncmp(basename(s), "mapperPlay", 10)==0) {
        playback_mode = 1;
    }
}

void ctrlc(int sig)
{
    done = 1;
}

int main(int argc, char *argv[])
{
    int rc=0;

    signal(SIGINT, ctrlc);

    oscstreamdb_defaults();

    init_playmode_default(argv[0]);

    if (cmdline(argc, argv))
        return 1;

    if (n_device_strings < 1) {
        printf("You must specify a device name to record. (-d)\n");
        return 1;
    }

    switch (backend) {
    case BACKEND_TEXT:
        backend_start = text_start;
        backend_stop = text_stop;
        backend_poll = text_poll;
        backend_write_value = text_write_value;
        backend_write_generic = text_write_generic;
        backend_seek_start = text_seek_start;
        backend_read = text_read;
        break;
    case BACKEND_BINARY:
        backend_start = binary_start;
        backend_stop = binary_stop;
        backend_poll = binary_poll;
        backend_write_value = binary_write_value;
        //WIP
        //backend_write_generic = binary_write_generic;
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
        //WIP
        //backend_write_generic = oscstreamdb_write_generic;
        break;
    default:
        printf("Unknown backend selected.\n");
        return 1;
    }

    if (playback_mode) {
        playback(0);
        return 0;
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

    while (!(backend_poll() || command_poll() || done)) {
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
