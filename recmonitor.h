
#ifndef __RECMONITOR_H__
#define __RECMONITOR_H__

int recmonitor_start();
void recmonitor_poll();
void recmonitor_stop();

struct stringlist
{
    const char *string;
    struct stringlist *next;
};

extern struct stringlist *device_strings;
extern struct stringlist *signal_strings;
extern int n_device_strings;
extern int n_signal_strings;

int recmonitor_add_device_string(const char *str);
int recmonitor_remove_device_string(const char *str);

int recmonitor_add_signal_string(const char *str);
int recmonitor_remove_signal_string(const char *str);

extern int send_device_names;
extern int send_signal_names;

const char *get_device_name();
const char *get_signal_name();

#endif // __RECMONITOR_H__
