
#include "server.hpp"

int main()
{
    Server server("127.0.0.1", 9001, 8);
    return server.Run();
}