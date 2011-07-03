
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mapper/mapper.h>

#include "recmonitor.h"
#include "recdevice.h"

const char *device_name = 0;
const char *path_name = 0;

mapper_monitor *mon;
mapper_db *db;
int dev_ready = 0;

typedef struct _signal_list_t {
    const char *device_name;
    const char *signal_name;
    char type;
    int length;
    struct _signal_list_t *next;
} signal_list_t;

signal_list_t *signal_stack = 0;

void push_signal_stack(const char *devname, const char *signame,
                       char type, int length);
signal_list_t *pop_signal_stack();
void free_signal(signal_list_t *sig);
void record_signals_on_stack();

static void device_callback(mapper_db_device dev,
                            mapper_db_action_t action,
                            void *user)
{
    if (action == MDB_NEW) {
        if (strstr(dev->name, device_name)!=0
            && strcmp(dev->name, mdev_name(recdev)?mdev_name(recdev):"")!=0)
            mapper_monitor_request_signals_by_name(mon, dev->name);
    }
}

static void signal_callback(mapper_db_signal sig,
                            mapper_db_action_t action,
                            void *user)
{
    if (action == MDB_NEW) {
        if (sig->is_output && strstr(sig->device_name, device_name)!=0
            && strcmp(sig->device_name,
                      mdev_name(recdev)?mdev_name(recdev):"")!=0
            && (path_name?strncmp(sig->name, path_name, strlen(path_name))==0:1))
            push_signal_stack(sig->device_name, sig->name,
                              sig->type, sig->length);
    }
}

int recmonitor_start()
{
    mon = mapper_monitor_new(0, 0);
    if (mon) {
        db = mapper_monitor_get_db(mon);
        mapper_db_add_device_callback(db, device_callback, 0);
        mapper_db_add_signal_callback(db, signal_callback, 0);
        return 0;
    }
    return 1;
}

void recmonitor_poll()
{
    mapper_monitor_poll(mon, 0);
    if (!dev_ready && mdev_ready(recdev)) {
        mapper_monitor_request_devices(mon);
        dev_ready = 1;
    }
    record_signals_on_stack();
}

void recmonitor_stop()
{
    if (!mon)
        return;
    if (mdev_ready(recdev)) {
        const char *devname = mdev_name(recdev);
        if (devname) {
            mapper_db_device *dev = mapper_db_match_devices_by_name(db, device_name);
            while (dev) {
                printf("Unlinking %s %s\n", (*dev)->name, devname);
                mapper_monitor_unlink(mon, (*dev)->name, devname);
                dev = mapper_db_device_next(dev);
            }
        }
    }
    mapper_monitor_free(mon);
}

void push_signal_stack(const char *devname, const char *signame,
                       char type, int length)
{
    signal_list_t **q = &signal_stack;
    signal_list_t *n = malloc(sizeof(signal_list_t));
    n->next = *q;
    *q = n;
    n->device_name = strdup(devname);
    n->signal_name = strdup(signame);
    n->type = type;
    n->length = length;
}

signal_list_t *pop_signal_stack()
{
    signal_list_t **q = &signal_stack;
    signal_list_t *n = *q;
    if (*q)
        *q = (*q)->next;
    return n;
}

void free_signal(signal_list_t *sig)
{
    if (sig) {
        free((void*)sig->device_name);
        free((void*)sig->signal_name);
        free(sig);
    }
}

void record_signals_on_stack()
{
    if (!mdev_ready(recdev))
        return;

    const char *devname = mdev_name(recdev);
    if (!devname) return;

    signal_list_t *n = pop_signal_stack();
    while (n) {
        printf("Recording %s%s\n", n->device_name, n->signal_name);

        /* TODO only do this if necessary -- will be ignored otherwise */
        mapper_monitor_link(mon, n->device_name, devname);

        recdevice_add_input(n->device_name, n->signal_name,
                            n->type, n->length);

        char signame[1024], recsigname[1024];
        snprintf(signame, 1024, "%s%s", n->device_name, n->signal_name);
        snprintf(recsigname, 1024, "%s%s%s", devname, n->device_name,
                 n->signal_name);

        mapper_monitor_connect(mon, signame, recsigname, 0, 0);

        free_signal(n);
        n = pop_signal_stack();
    }
}
