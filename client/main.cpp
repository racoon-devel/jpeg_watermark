
#include <iostream>
#include <string>
#include <vector>
#include <list>
using namespace std;

#include "asio.hpp"
#include "easylogging++.h"
#include "client.hpp"

INITIALIZE_EASYLOGGINGPP

int main(int argc, char **argv)
{
    // TODO: сделать парсинг аргументов командной строки и загрузку изображения
    (void) argc; (void)argv;
    // cout << "Usage: <jpeg_source_filepath> <server_addr> <server_port> <timeout> <jpeg_output_filename>" << endl;
    
    string addr = "127.0.0.1";
    int port = 9001;
    string text = "I am not a lamer!!!";
    uint timeout = 10;
    uint clients_count = 50;
    
    vector<uint8_t> m_source; m_source.resize(10 * 1024 * 1024, 0);
    asio::io_service io;

    list<ProtoClient> clients; 

    for (uint i = 0; i < clients_count; i++)
    {
        clients.emplace_back(io, m_source, addr, port, text, timeout);
        clients.back().Run();
    }

    io.run();

    return 0;
}