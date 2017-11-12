#include "macnockserver.h"
#include "nockpackage.h"
#include "log.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <algorithm>

macNockServer::macNockServer(const std::string &interface, const std::string &hood)
    : m_interface(interface)
    , m_hood(hood)
    , m_stop(false)
    , m_tc(interface)
{

}

bool macNockServer::run()
{
    m_sock = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (m_sock < 0)
    {
        log::error(std::string("[s] ERROR: Can't create socket (") + strerror(errno) + ").");
        return false;
    }

    // allow to reuse the port immediately as soon as the service exits.
    int sockoptval = 1;
    if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int)) != 0)
    {
        log::error(std::string("[s] WARNING: Can't set socket options (") + strerror(errno) + ").");
    }

    sockaddr_in6 my_addr;
    memset(&my_addr, 0, sizeof(sockaddr_in6));
    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_port = htons(PORT);
    my_addr.sin6_addr = in6addr_any;

    if (bind(m_sock, reinterpret_cast<sockaddr *>(&my_addr), sizeof(sockaddr_in6)) < 0)
    {
        log::error(std::string("[s] ERROR: Can't bind to address (") + strerror(errno) + ")");
        return false;
    }

    while (!m_stop)
    {
        log::debug(std::string("[s] waiting on port ") + std::to_string(PORT));

        sockaddr_in6 client_addr;
        socklen_t addrlen = sizeof(client_addr);

        static const size_t BUFSIZE = 2048;
        uint8_t buf[BUFSIZE];

        int recvlen = recvfrom(m_sock, buf, BUFSIZE, 0, reinterpret_cast<sockaddr *>(&client_addr), &addrlen);

        if (recvlen <= 0)
        {
            continue;
        }

        char addrBuf[INET6_ADDRSTRLEN];
        const char *ret = inet_ntop(AF_INET6, &client_addr.sin6_addr, addrBuf, sizeof(addrBuf));
        log::debug(std::string("[s] received ") + std::to_string(recvlen) + " bytes from " + ret);

        NockPackage nock;
        if (nock.deserialize(buf, recvlen))
        {
            const std::string mac{to_string(nock.getMac())};
            log::debug(std::string("The MAC: ") + mac + ", the Hood: " + nock.getHood());

            if (nock.getHood() == m_hood)
            {
                log::debug(std::string("[s] allowing ") + mac);
                MacList::iterator found = std::find(m_filterList.begin(), m_filterList.end(), nock.getMac());
                if (found == m_filterList.end())
                {
                    m_filterList.push_back(nock.getMac());
                    m_tc.allow_mac(mac);
                }
            }
            else
            {
                log::debug(std::string("[s] not allowing ") + mac + ", wrong hood");
                MacList::iterator found = std::remove(m_filterList.begin(), m_filterList.end(), nock.getMac());
                if (found != m_filterList.end())
                {
                    m_filterList.erase(found);
                    m_tc.disallow_mac(mac);
                }
            }
        }
        else
        {
            log::error(std::string("[s] can't deserialize message \"") + std::string(buf, buf+recvlen) + "\"");
        }
    }

    close(m_sock);
    log::debug("Server closed");
    return true;
}

void macNockServer::stop()
{
    log::debug("Stopping Server");
    m_stop = true;
    shutdown(m_sock, SHUT_RDWR);
}
