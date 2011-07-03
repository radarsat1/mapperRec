
#ifndef __RECMONITOR_H__
#define __RECMONITOR_H__

extern const char *device_name;
const char *path_name;

int recmonitor_start();
void recmonitor_poll();
void recmonitor_stop();

#endif // __RECMONITOR_H__
