#ifndef _MACSTORAGE_H
#define _MACSTORAGE_H

#include <stdint.h>

void macStorage_add(uint8_t mac[]);
void macStorage_stop();
void macStorage_run();

#endif // _MACSTORAGE_H
