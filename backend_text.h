
#ifndef __BACKEND_TEXT_H__
#define __BACKEND_TEXT_H__

#include <mapper/mapper.h>

typedef struct {
    const char *file_path;
} backend_text_options_t;

extern backend_text_options_t backend_text_options;

void text_defaults();
int text_start();
void text_stop();
int text_poll();
void text_write_value(mapper_signal msig, void *v);

#endif // __BACKEND_TEXT_H__
