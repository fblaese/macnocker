#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <stdint.h>

struct NockPackage
{
    uint8_t version;
    uint8_t mac[6];
    uint8_t hoodLen;
    char hood[];
};

//size_t NockPackage_serialize(uint8_t *buf, size_t maxlen, const NockPackage *p);

//uint8_t NockPackage_deserialize(NockPackage *p, const uint8_t *buf, size_t len);

#endif // _PROTOCOL_H
