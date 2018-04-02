#include "macnockclient.h"
#include "nockpackage.h"
#include "log.h"

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
#include <string.h>
#include <stdint.h>

extern const char *g_hood;
extern const char *g_interface;

extern const int PORT;
extern const uint8_t VERSION;

static uint8_t stop;

void macNockClient_stop()
{
    log_debug("Stopping Client\n");
    stop = 1;
}

void macNockClient_run()
{
    stop = 0;

    const char host[] = "ff02::1";

    int fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);
    if (fd < 0)
    {
        perror("[c] ERROR: Can't create socket");
        return;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, g_interface, strlen(g_interface)) < 0)
    {
        perror("[c] WARNING: Can't bind to device");
    }

    struct ifreq ifr;
    memset(&ifr, 0, sizeof(ifr));
    strncpy(ifr.ifr_name, g_interface, IFNAMSIZ);

    if (!((ioctl(fd, SIOCGIFFLAGS, &ifr) == 0) && (ioctl(fd, SIOCGIFHWADDR, &ifr) == 0)))
    {
        perror("[c] ERROR: Can't read MAC");
        close(fd);
        return;
    }

    struct sockaddr_in6 servaddr;
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port = htons(PORT);

    if (inet_pton(AF_INET6, host, &servaddr.sin6_addr) == 0)
    {
        perror("[c] ERROR: Can't resolve host");
        close(fd);
        return;
    }

    const size_t hoodLen = strlen(g_hood)+1;
    struct NockPackage *nock = malloc(sizeof(struct NockPackage) + hoodLen);
    memcpy(nock->mac, ifr.ifr_hwaddr.sa_data, sizeof(nock->mac));
    nock->version = VERSION;
    nock->hoodLen = hoodLen;
    strcpy(nock->hood, g_hood);
    const size_t len = sizeof(struct NockPackage) + hoodLen;

    while (!stop)
    {
        log_debug("[c] sending\n");

        int sent = sendto(fd, nock, len, 0, (struct sockaddr *)&servaddr, sizeof(servaddr));
        if (sent == -1)
        {
            perror("[c] ERROR: Can't send data");
        }
        else if ((size_t)sent != len)
        {
            perror("[c] ERROR: Can't send all data");
        }

        usleep(1 * 1000 * 1000); // sleep 1 s
    }

    free(nock);

    close(fd);

    log_debug("Client closed\n");
    return;
}
