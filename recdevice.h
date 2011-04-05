
#ifndef __RECDEVICE_H__
#define __RECDEVICE_H__

#include <mapper/mapper.h>

int recdevice_start();
void recdevice_poll();
void recdevice_stop();

void recdevice_add_input(const char *devname, const char *signame,
                         char type, int length);

extern mapper_device *recdev;

void recdevice_get_frame(int *take, int *frame);

#endif // __RECDEVICE_H__
