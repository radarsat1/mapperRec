
#ifndef __RECDEVICE_H__
#define __RECDEVICE_H__

#include <mapper/mapper.h>

int recdevice_start();
void recdevice_poll();
void recdevice_stop();

void recdevice_add_input(const char *devname, const char *signame,
                         char type, int length);

void recdevice_reset_generic_handler();

extern mapper_device *recdev;

#endif // __RECDEVICE_H__
