#include "macnockserver.h"
#include "macnockclient.h"
#include "macstorage.h"
#include "log.h"

#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdint.h>
#include <pthread.h>

const int PORT = 2342;
const uint8_t VERSION = 1;

const char *g_interface;
const char *g_hood;

void sigHandler(int sig)
{
    (void)sig;
    macNockServer_stop();
    macNockClient_stop();
    macStorage_stop();
}

void* serverRoutine(void *arg)
{
    macNockServer_run();
    return arg;
}

void* clientRoutine(void *arg)
{
    macNockClient_run();
    return arg;
}

void* storageRoutine(void *arg)
{
    macStorage_run();
    return arg;
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        printf("Send and listen for nock requests\n");
        printf("\n");
        printf("%s interface code command\n", argv[0]);
        printf("Example:\n");
        printf("%s eth0.3 \"FuerthV2\"\n", argv[0]);
        return -1;
    }

    g_interface = argv[1];
    g_hood = argv[2];

    printf("%s: Running for hood %s on interface %s\n", argv[0], g_hood, g_interface);

    signal(SIGINT, &sigHandler);
    signal(SIGTERM, &sigHandler);

    pthread_t serverThread;
    pthread_t clientThread;
    pthread_t storageThread;

    if (pthread_create(&serverThread, NULL, serverRoutine, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    if (pthread_create(&clientThread, NULL, clientRoutine, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    if (pthread_create(&storageThread, NULL, storageRoutine, NULL))
    {
        fprintf(stderr, "Error creating thread\n");
        return 1;
    }

    if (pthread_join(serverThread, NULL))
    {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    if (pthread_join(clientThread, NULL))
    {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    if (pthread_join(storageThread, NULL))
    {
        fprintf(stderr, "Error joining thread\n");
        return 2;
    }

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    return 0;
}
