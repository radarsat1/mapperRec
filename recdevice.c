
#include <stdio.h>
#include <string.h>
#include <mapper/mapper.h>

#include "recdevice.h"
#include "mapperRec.h"

mapper_device *recdev;

struct
{
    int take;
    int frame;
} framestamp = { 0, 0 };

static void frame_handler(mapper_signal msig, void *v)
{
    int *i = (int*)v;
    framestamp.take  = i[0];
    framestamp.frame = i[1];
}

void recdevice_get_frame(int *take, int *frame)
{
    *take = framestamp.take;
    *frame = framestamp.frame;
}

int recdevice_start()
{
    recdev = mdev_new("mapperRec", 9000, 0);
    if (recdev)
        mdev_add_input(recdev, "frame", 2, 'i', 0, 0, 0, frame_handler, 0);
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

void input_handler(struct _mapper_signal *msig, mapper_db_signal props,
                   mapper_timetag_t *timetag, void *v)
{
    backend_write_value(msig, v);
}

void recdevice_add_input(const char *devname, const char *signame,
                         char type, int length)
{
    char name[1024];
    snprintf(name, 1024, "%s%s", devname, signame);
    printf("Adding signal %s\n", name);
    mdev_add_input(recdev, name, length, type, 0, 0, 0, input_handler, 0);
}
