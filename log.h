#ifndef _DEBUG_H
#define _DEBUG_H

#include <string>

namespace log {

void debug(const std::string &dbg);

void error(const std::string &dbg);

} // namespace log

#endif // _DEBUG_H
