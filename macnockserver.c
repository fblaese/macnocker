#include "macnockserver.h"
#include "macstorage.h"
#include "nockpackage.h"
#include "log.h"
#include "tc.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include <stdint.h>
#include <string.h>
#include <stdio.h>

extern const char *g_hood;
extern const char *g_interface;

extern const int PORT;
extern const uint8_t VERSION;

static uint8_t stop;
int fd;

void macNockServer_stop()
{
    log_debug("[s] Stopping Server\n");
    stop = 1;
    shutdown(fd, SHUT_RDWR);
}

void macNockServer_run()
{
    stop = 0;

    tc_start();

    fd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_UDP);

    if (fd < 0)
    {
        perror("[s] ERROR: Can't create socket");
        return;
    }

    if (setsockopt(fd, SOL_SOCKET, SO_BINDTODEVICE, g_interface, strlen(g_interface)) < 0)
    {
        perror("[c] WARNING: Can't bind to device");
    }

    // allow to reuse the port immediately as soon as the service exits.
    int sockoptval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &sockoptval, sizeof(int)) != 0)
    {
        perror("[s] WARNING: Can't set socket options");
    }

    struct sockaddr_in6 my_addr;
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin6_family = AF_INET6;
    my_addr.sin6_port = htons(PORT);
    my_addr.sin6_addr = in6addr_any;

    if (bind(fd, (struct sockaddr *)&my_addr, sizeof(my_addr)) < 0)
    {
        perror("[s] ERROR: Can't bind to address");
        return;
    }

    while (!stop)
    {
        log_trace("[s] waiting for connection\n");

        struct sockaddr_in6 client_addr;
        socklen_t addrlen = sizeof(client_addr);

        static const size_t BUFSIZE = 2048;
        uint8_t buf[BUFSIZE];

        int recvlen = recvfrom(fd, buf, BUFSIZE, 0, (struct sockaddr *)&client_addr, &addrlen);

        if (recvlen <= 0)
        {
            continue;
        }

        char addrBuf[INET6_ADDRSTRLEN];
        const char *ret = inet_ntop(AF_INET6, &client_addr.sin6_addr, addrBuf, sizeof(addrBuf));
        log_trace("[s] received %d bytes from %s\n", recvlen, ret);

        struct NockPackage *nock = (struct NockPackage *)buf;
        nock->hood[nock->hoodLen] = '\0';

        if (nock->version != VERSION)
        {
            fprintf(stderr, "Received package with wrong version from %s\n", ret);
            continue;
        }

        log_trace("[s] The MAC: %02x:%02x:%02x:%02x:%02x:%02x, the Hood: %s\n",
                  nock->mac[0], nock->mac[1], nock->mac[2], nock->mac[3], nock->mac[4], nock->mac[5], nock->hood);

        if (strcmp(nock->hood, g_hood) == 0)
        {
            log_trace("[s] allowing %02x:%02x:%02x:%02x:%02x:%02x\n",
                  nock->mac[0], nock->mac[1], nock->mac[2], nock->mac[3], nock->mac[4], nock->mac[5]);
            macStorage_add(nock->mac);
        }
        else
        {
            log_debug("[s] Not allowing %02x:%02x:%02x:%02x:%02x:%02x. Wrong hood: \"%s\"\n",
                  nock->mac[0], nock->mac[1], nock->mac[2], nock->mac[3], nock->mac[4], nock->mac[5], nock->hood);
        }
    }

    close(fd);
    tc_stop();

    log_debug("[s] Server closed\n");
    return;
}
