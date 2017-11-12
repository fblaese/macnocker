#ifndef _MAC_H
#define _MAC_H

#include <array>
#include <string>
#include <list>

using Mac = std::array<uint8_t, 6>;
using MacList = std::list<Mac>;

std::string to_string(const Mac &o);

#endif // _MAC_H
