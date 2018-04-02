#ifndef _TC_H
#define _TC_H

#include <stdint.h>

void tc_start();

void tc_stop();

void tc_allow_mac(const uint8_t mac[]);

void tc_disallow_mac(const uint8_t mac[]);

#endif // _TC_H
