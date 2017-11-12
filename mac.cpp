#include "mac.h"

#include <sstream>
#include <iomanip>

std::string to_string(const Mac &o)
{
    std::ostringstream stream;
    for (Mac::const_iterator it=o.begin(); it!=o.end(); ++it)
    {
        stream << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(*it);
    }
    return stream.str();
}
