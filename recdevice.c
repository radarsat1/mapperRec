
#include <stdio.h>
#include <string.h>
#include <mapper/mapper.h>

#include "backend.h"
#include "recdevice.h"
#include "mapperRec.h"

mapper_device *recdev;

int recdevice_start()
{
    recdev = mdev_new("mapperRec", 9000, 0);
    return recdev==0;
}

void recdevice_poll()
{
    mdev_poll(recdev, 0);
}

void recdevice_stop()
{
    if (recdev)
        mdev_free(recdev);
}

void input_handler(mapper_signal msig, mapper_db_signal props,
                   int instance_id, void *value, int count,
                   mapper_timetag_t *timetag)
{
    if (value)
        backend_write_value(msig, value);
}

int generic_handler(const char *path, const char *types, lo_arg ** argv,
                    int argc, void *data, void *user_data)
{
    backend_write_generic(path, types, data);
    return 0;
}

void recdevice_add_input(const char *devname, const char *signame,
                         char type, int length)
{
    char name[1024];
    snprintf(name, 1024, "%s%s", devname, signame);
    printf("Adding signal %s\n", name);
    mdev_add_input(recdev, name, length, type, 0, 0, 0, input_handler, 0);
}

/*! This function ensures that the "generic" handler is always last,
 *  otherwise it would block mapper signal handlers. */
void recdevice_reset_generic_handler()
{
    /* If backend does not support generic messages, just ignore this
     * command. */
    if (!backend_write_generic)
        return;

    lo_server s = mdev_get_lo_server(recdev);
    if (s) {
        lo_server_del_method(s, 0, 0);
        lo_server_add_method(s, 0, 0, generic_handler, 0);
    }
}
