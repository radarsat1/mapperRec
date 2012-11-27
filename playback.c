
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "mapperRec.h"
#include "playback.h"
#include "backend.h"

playback_options_t playback_options =
{
    .dest_url = 0,
};

void playback(int just_print)
{
    if (!backend_seek_start || !backend_read)
    {
        printf("The %s backend does not support reading.\n",
            backend_strings[backend]);
        exit(1);
    }

    if (!playback_options.dest_url && !just_print)
    {
        printf("No destination URL was provided for playback.\n");
        exit(1);
    }

    lo_address a = lo_address_new_from_url(playback_options.dest_url);
    if (!a) {
        printf("Error opening destination OSC address `%s'.\n",
               playback_options.dest_url);
        exit(1);
    }

    if (backend_seek_start())
        exit(0);

    lo_message m;
    lo_timetag tt, first_tt;
    char *path;

    if (backend_read(&path, &m, &first_tt))
        return;

    if (just_print)
    {
        printf("%d %d %s ", tt.sec, tt.frac, path);
        lo_message_pp(m);

        while (!backend_read(&path, &m, &tt))
        {
            printf("%d %d %s ", tt.sec, tt.frac, path);
            lo_message_pp(m);
        }
    }
    else
    {
        lo_send_message(a, path, m);

        free(path);
        lo_message_free(m);

        while (!backend_read(&path, &m, &tt) && !done)
        {
            double wait_time = mapper_timetag_difference(tt, first_tt);

            if (wait_time > 0.)
                usleep(wait_time * 1000000);

            lo_send_message(a, path, m);

            free(path);
            lo_message_free(m);

            first_tt.sec = tt.sec;
            first_tt.frac = tt.frac;
        }
    }

    backend_stop();
    lo_address_free(a);
}
