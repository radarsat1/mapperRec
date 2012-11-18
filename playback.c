
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

        const uint64_t max_frac = ((uint64_t)1)<<32;
        const double max_fracd = (double)max_frac;
        while (!backend_read(&path, &m, &tt) && !done)
        {
            uint64_t frac_to_wait, sec_to_wait = tt.sec - first_tt.sec;
            if (sec_to_wait <= 0)
            {
                sec_to_wait = 0;
                frac_to_wait = tt.frac - first_tt.frac;
            }
            else
            {
                frac_to_wait = (max_frac - first_tt.frac) + tt.frac;
            }

            useconds_t wait_time =
                sec_to_wait*1000000 + (useconds_t)((frac_to_wait
                                                    / max_fracd)*1e6);

            if (wait_time)
                usleep(wait_time);

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
