#include "../include/Server.hpp"

int main()
{
    Server server = Server("127.0.0.1", 8080);
    server.runLoop();

    return 0;
}
