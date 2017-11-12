#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include "mac.h"

#include <string>

static const int PORT{2342};
static const int8_t VERSION{1};

class NockPackage
{
public:
    using Hood = std::string;

    NockPackage();

    NockPackage(const Mac &sourceMac, const Hood &hoodName);

    size_t serialize(uint8_t *buf, size_t maxlen) const;

    bool deserialize(const uint8_t *buf, size_t len);

    Mac getMac() const;

    std::string getHood() const;

private:
    Mac m_sourceMac;
    Hood m_hoodName;
};

#endif // _PROTOCOL_H
