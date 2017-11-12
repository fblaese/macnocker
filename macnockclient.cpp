#include "macnockclient.h"
#include "nockpackage.h"
#include "log.h"

#include <vector>

#include <arpa/inet.h>
#include <linux/if_packet.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/ether.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>

#include <thread>
#include <chrono>

macNockClient::macNockClient(const std::string &interface, const std::string &hood)
    : m_interface(interface)
    , m_hood(hood)
{

}

void macNockClient::stop()
{
    log::debug("Stopping Client");
    m_stop = true;
}

bool macNockClient::run()
{
    const std::string host{"ff02::1"};

    m_stop = false;

    int fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0)
    {
        log::error(std::string("[c] ERROR: Can't create socket (") + strerror(errno) + ").");
        return false;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, m_interface.c_str(), m_interface.size()) < 0)
    {
        log::error(std::string("[c] WARNING: Can't bind to device (") + strerror(errno) + ").");
    }

    ifreq ifr;
    memset(&ifr, 0, sizeof(ifreq));
    strncpy(ifr.ifr_name, m_interface.c_str(), IFNAMSIZ);

    if (!((ioctl(fd, SIOCGIFFLAGS, &ifr) == 0) && (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)))
    {
        log::error(std::string("[c] ERROR: Can't read MAC (") + strerror(errno) + ").");
        return false;
    }

    sockaddr_in6 servaddr;
    memset(&servaddr, 0, sizeof(sockaddr_in6));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PORT);

    if (inet_pton(AF_INET6, host.c_str(), &servaddr.sin6_addr) == 0)
    {
        log::error(std::string("[c] ERROR: Can't resolve host (") + strerror(errno) + ").");
        close(fd);
        return false;
    }

    const Mac mac{ static_cast<uint8_t>(ifr.ifr_hwaddr.sa_data[0]), static_cast<uint8_t>(ifr.ifr_hwaddr.sa_data[1]),
                static_cast<uint8_t>(ifr.ifr_hwaddr.sa_data[2]), static_cast<uint8_t>(ifr.ifr_hwaddr.sa_data[3]),
                static_cast<uint8_t>(ifr.ifr_hwaddr.sa_data[4]), static_cast<uint8_t>(ifr.ifr_hwaddr.sa_data[5]), };
    const NockPackage nock{mac, m_hood};

    while (!m_stop)
    {
        log::debug("[c] sending");
        uint8_t buf[256];
        size_t len = nock.serialize(buf, 256);

        int sent = sendto(fd, buf, len, 0, reinterpret_cast<sockaddr *>(&servaddr), sizeof(servaddr));
        if (sent == -1)
        {
            log::error(std::string("[c] ERROR: Can't send data (") + strerror(errno) + ").");
        }
        else if (static_cast<size_t>(sent) != len)
        {
            log::error("[c] ERROR: Can't send all data.");
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    close(fd);

    log::debug("Client closed");
    return true;
}
