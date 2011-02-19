
#include <stdio.h>
#include <string.h>
#include <mapper/mapper.h>

#include "recmonitor.h"

const char *device_name = 0;

mapper_monitor *mon;
mapper_db *db;

static void device_callback(mapper_db_device dev,
                            mapper_db_action_t action,
                            void *user)
{
    if (action == MDB_NEW) {
        if (strstr(dev->name, device_name)!=0)
            mapper_monitor_request_signals_by_name(mon, dev->name);
    }
}

static void signal_callback(mapper_db_signal sig,
                            mapper_db_action_t action,
                            void *user)
{
    if (action == MDB_NEW) {
        if (sig->is_output && strstr(sig->device_name, device_name)!=0) {
            printf("Found: %s%s\n", sig->device_name, sig->name);
        }
    }
}

int recmonitor_start()
{
    mon = mapper_monitor_new();
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
}

void recmonitor_stop()
{
    mapper_monitor_free(mon);
}
