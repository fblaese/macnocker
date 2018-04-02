#include "macstorage.h"
#include "log.h"
#include "tc.h"

#include <stdint.h>
#include <pthread.h>
#include <time.h>
#include <string.h>
#include <unistd.h>

static uint8_t stop;

#define MAXENTRIES 30

struct Entry
{
    uint8_t mac[6];
    time_t timestamp;
} storage[MAXENTRIES];

size_t count = 0;

pthread_mutex_t mux;

void _addEntry(uint8_t mac[6])
{
    for (size_t i=0; i<count; i++)
    {
        if (memcmp(storage[i].mac, mac, 6) == 0)
        {
            storage[i].timestamp = time(NULL);
            return;
        }
    }

    if (count != MAXENTRIES)
    {
        memcpy(storage[count].mac, mac, 6);
        storage[count].timestamp = time(NULL);
        count++;

        tc_allow_mac(mac);
    }
}

void macStorage_add(uint8_t mac[6])
{
    pthread_mutex_lock(&mux);
    _addEntry(mac);
    pthread_mutex_unlock(&mux);
}

void _checkTimeout()
{
    pthread_mutex_lock(&mux);

    time_t t = time(NULL)-5;

    for (size_t i=0; i<count; i++)
    {
        if (storage[i].timestamp < t)
        {
            tc_disallow_mac(storage[i].mac);
            count--;
            memcpy(storage[i].mac, storage[count].mac, 6);
            storage[i].timestamp = storage[count].timestamp;
        }
    }

    pthread_mutex_unlock(&mux);
}

void macStorage_stop()
{
    log_debug("Stopping Storage\n");
    stop = 1;
}

void macStorage_run()
{
    stop = 0;

    while (!stop)
    {
        _checkTimeout();
        usleep(5 * 1000 * 1000);
    }

    log_debug("Storage closed\n");
    return;
}
