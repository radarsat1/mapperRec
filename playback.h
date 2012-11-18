
#ifndef _PLAYBACK_H_
#define _PLAYBACK_H_

typedef struct {
    const char *dest_url;
} playback_options_t;

extern playback_options_t playback_options;

void playback(int just_print);

#endif // _PLAYBACK_H_
