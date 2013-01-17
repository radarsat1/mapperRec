
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <mapper/mapper.h>

#include "recmonitor.h"
#include "recdevice.h"

struct stringlist *device_strings = 0;
struct stringlist *signal_strings = 0;
int n_device_strings = 0;
int n_signal_strings = 0;

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

int send_device_names = 0;
char *device_name_space = 0;
int device_name_space_size = 512;
char *dev_name_wptr = 0;
char *dev_name_rptr = 0;

static void put_device_name(const char *name, int action)
{
    if (!send_device_names)
        return;

    int L = strlen(name);

    if (!device_name_space
        || dev_name_wptr + L + 2 > device_name_space + device_name_space_size)
    {
        device_name_space_size *= 2;
        char *old_device_name_space = device_name_space;
        device_name_space = realloc(device_name_space,
                                    device_name_space_size);
        dev_name_wptr =
            dev_name_wptr - old_device_name_space + device_name_space;
        dev_name_rptr =
            dev_name_rptr - old_device_name_space + device_name_space;
    }

    *(dev_name_wptr++) = 1;
    *(dev_name_wptr++) = action;
    strcpy(dev_name_wptr, name);
    dev_name_wptr += L;
    *(dev_name_wptr++) = 0;
    *dev_name_wptr = 0;
}

const char *get_device_name()
{
    if (!dev_name_rptr || !*dev_name_rptr)
        return 0;

    const char *s = dev_name_rptr+1;
    while (*dev_name_rptr
           && dev_name_rptr < device_name_space + device_name_space_size)
        dev_name_rptr++;

    dev_name_rptr++;

    // End of list
    if (!*dev_name_rptr) {
        dev_name_rptr = device_name_space;
        dev_name_wptr = device_name_space;
        (*dev_name_wptr) = 0;
    }

    return s;
}

int send_signal_names = 0;
char *signal_name_space = 0;
int signal_name_space_size = 512;
char *sig_name_wptr = 0;
char *sig_name_rptr = 0;

static void put_signal_name(const char *devname, const char *signame,
                            int action)
{
    if (!send_signal_names)
        return;

    int L = strlen(devname) + strlen(signame);

    if (!signal_name_space
        || sig_name_wptr + L + 2 > signal_name_space + signal_name_space_size)
    {
        signal_name_space_size *= 2;
        char *old_signal_name_space = signal_name_space;
        signal_name_space = realloc(signal_name_space,
                                    signal_name_space_size);
        sig_name_wptr =
            sig_name_wptr - old_signal_name_space + signal_name_space;
        sig_name_rptr =
            sig_name_rptr - old_signal_name_space + signal_name_space;
    }

    *(sig_name_wptr++) = 1;
    *(sig_name_wptr++) = action;
    strcpy(sig_name_wptr, devname);
    strcpy(sig_name_wptr + strlen(devname), signame);
    sig_name_wptr += L;
    *(sig_name_wptr++) = 0;
    *sig_name_wptr = 0;
}

const char *get_signal_name()
{
    if (!sig_name_rptr || !*sig_name_rptr)
        return 0;

    const char *s = sig_name_rptr+1;
    while (*sig_name_rptr
           && sig_name_rptr < signal_name_space + signal_name_space_size)
        sig_name_rptr++;

    sig_name_rptr++;

    // End of list
    if (!*sig_name_rptr) {
        sig_name_rptr = signal_name_space;
        sig_name_wptr = signal_name_space;
        (*sig_name_wptr) = 0;
    }

    return s;
}

static void device_callback(mapper_db_device dev,
                            mapper_db_action_t action,
                            void *user)
{
    if (action == MDB_NEW) {
        put_device_name(dev->name, 1);
        struct stringlist **node = &device_strings;
        while (*node) {
            if (strstr(dev->name, (*node)->string)!=0
                && strcmp(dev->name,
                          mdev_name(recdev)?mdev_name(recdev):"")!=0)
            {
                mapper_monitor_request_signals_by_name(mon, dev->name);
                break;
            }
            node = &(*node)->next;
        }
    }
    else if (action == MDB_REMOVE) {
        put_device_name(dev->name, -1);
    }
}

static void signal_callback(mapper_db_signal sig,
                            mapper_db_action_t action,
                            void *user)
{ 
    if (!sig->is_output)
        return;
    if (action == MDB_NEW) {
        put_signal_name(sig->device_name, sig->name, 1);
        struct stringlist **devnode = &device_strings;
        while (*devnode) {
            if (strstr(sig->device_name, (*devnode)->string)!=0
                && strcmp(sig->device_name,
                          mdev_name(recdev)?mdev_name(recdev):"")!=0)
            {
                struct stringlist **signode = &signal_strings;
                if (!*signode) {
                    push_signal_stack(sig->device_name, sig->name,
                                      sig->type, sig->length);
                    break;
                }
                while (*signode) {
                    if ((strncmp(sig->name, (*signode)->string,
                                 strlen((*signode)->string))==0)
                        || ((*signode)->string[0]=='/'
                            && strncmp(sig->name, (*signode)->string+1,
                                       strlen((*signode)->string))==0))
                    {
                        push_signal_stack(sig->device_name, sig->name,
                                          sig->type, sig->length);
                        signode = 0;
                        break;
                    }
                    else
                        signode = &(*signode)->next;
                }
                if (!signode)
                    break;
            }
            devnode = &(*devnode)->next;
        }
    }
    else if (action == MDB_REMOVE) {
        put_signal_name(sig->device_name, sig->name, -1);
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
            struct stringlist **node = &device_strings;
            while (*node) {
                mapper_db_device *dev =
                    mapper_db_match_devices_by_name(db, (*node)->string);
                while (dev) {
                    printf("Unlinking %s %s\n", (*dev)->name, devname);
                    mapper_monitor_unlink(mon, (*dev)->name, devname);
                    dev = mapper_db_device_next(dev);
                }
                node = &(*node)->next;
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
    char destname[256] = "";
    if (!devname) return;

    signal_list_t *n = pop_signal_stack();
    if (!n)
        return;

    while (n) {
        printf("Recording %s%s\n", n->device_name, n->signal_name);

        if (strncmp(n->device_name, destname, 256)) {
            mapper_monitor_link(mon, n->device_name, devname, 0, 0);
            snprintf(destname, 256, "%s", n->device_name);
        }

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

    recdevice_reset_generic_handler();
}

int recmonitor_add_device_string(const char *str)
{
    struct stringlist **node = &device_strings;
    struct stringlist *prevnode = device_strings;
    *node = malloc(sizeof(struct stringlist));
    (*node)->string = strdup(str);
    (*node)->next = prevnode;
    return ++n_device_strings;
}

int recmonitor_remove_device_string(const char *str)
{
    struct stringlist **node = &device_strings;
    while (*node && !strcmp(str, (*node)->string))
        node = &(*node)->next;
    if (!*node)
        return n_device_strings;
    struct stringlist *del = *node;
    *node = (*node)->next;
    free((void*)del->string);
    free((void*)del);
    return --n_device_strings;
}

int recmonitor_add_signal_string(const char *str)
{
    struct stringlist **node = &signal_strings;
    struct stringlist *prevnode = signal_strings;
    *node = malloc(sizeof(struct stringlist));
    (*node)->string = strdup(str);
    (*node)->next = prevnode;
    return ++n_signal_strings;
}

int recmonitor_remove_signal_string(const char *str)
{
    struct stringlist **node = &signal_strings;
    while (*node && !strcmp(str, (*node)->string))
        node = &(*node)->next;
    if (!*node)
        return n_signal_strings;
    struct stringlist *del = *node;
    *node = (*node)->next;
    free((void*)del->string);
    free((void*)del);
    return --n_signal_strings;
}
