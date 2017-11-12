#include "tc.h"
#include "mac.h"

#include <string>
#include <list>

class macNockServer
{
public:
    macNockServer(const std::string &interface, const std::string &hood);
    bool run();
    void stop();

private:
    void sendData(const int socket, const std::string &data) const;

private:
    const std::string &m_interface;
    const std::string &m_hood;
    bool m_stop;
    int m_sock;
    tc m_tc;
    MacList m_filterList;
};
