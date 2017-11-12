#include "macnockserver.h"
#include "macnockclient.h"
#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <unistd.h>
#include <csignal>

macNockServer *server = nullptr;
macNockClient *client = nullptr;

void sigHandler(int)
{
    if (server)
    {
        server->stop();
    }
    if (client)
    {
        client->stop();
    }
}

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        std::cout << "Send and listen for nock requests" << std::endl;
        std::cout << std::endl;
        std::cout << argv[0] << " interface code command" << std::endl;
        std::cout << "Example:" << std::endl;
        std::cout << argv[0] << " eth0.3 \"FuerthV2\"" << std::endl;
        return -1;
    }

    const std::string interface{argv[1]};
    const std::string hood{argv[2]};

    server = new macNockServer{interface, hood};
    client = new macNockClient{interface, hood};

    signal(SIGINT, &sigHandler);
    signal(SIGTERM, &sigHandler);

    std::thread serverThread{[](){server->run();}};

    std::this_thread::sleep_for(std::chrono::milliseconds(10));

    std::thread clientThread{[](){client->run();}};

    serverThread.join();
    clientThread.join();

    signal(SIGINT, SIG_DFL);
    signal(SIGTERM, SIG_DFL);

    delete client;
    client = nullptr;

    delete server;
    server = nullptr;

    return 0;
}
