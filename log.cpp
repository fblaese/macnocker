#include "log.h"

#include <iostream>
#include <mutex>

namespace log {

std::mutex g_output_mutex;

#ifdef DEBUG
void debug(const std::string &dbg)
{
    std::lock_guard<std::mutex> guard(g_output_mutex);
    std::cout << dbg << std::endl;
}
#else
void debug(const std::string &)
{

}
#endif

void error(const std::string &dbg)
{
    std::lock_guard<std::mutex> guard(g_output_mutex);
    std::cerr << dbg << std::endl;
}

} // namespace log
