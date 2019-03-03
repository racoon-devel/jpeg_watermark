
#include <iostream>
#include <string>
#include <vector>
#include <list>
using namespace std;

#include "asio.hpp"
#include "easylogging++.h"
#include "client.hpp"

INITIALIZE_EASYLOGGINGPP

bool load_image(const string& filepath, vector<uint8_t>& image)
{
    ifstream is(filepath, ios::binary);
    
    if (is.fail())
        return false;

    streampos fsize = is.tellg();
    is.seekg(0, is.end);
    fsize = is.tellg() - fsize;
    is.seekg(0, is.beg);

    image.resize(fsize);

    is.read(reinterpret_cast<char*>(&image[0]), fsize);

    is.close();
    return true;
}

void save_image(const string& filepath, const vector<uint8_t>& image)
{
    ofstream os(filepath, ios::binary);

    if (os.fail())
    {
        cerr << "Open file for write failed" << endl;
        return ;
    }

    os.write(reinterpret_cast<const char*>(&image[0]), image.size());

    os.close();
}


int main(int argc, char **argv)
{
    // TODO: сделать парсинг аргументов командной строки и загрузку изображения
    (void) argc; (void)argv;
    // cout << "Usage: <jpeg_source_filepath> <server_addr> <server_port> <timeout> <jpeg_output_filename>" << endl;
    
    string addr = "127.0.0.1";
    int port = 9001;
    string text = "I am not a lamer!!!";
    uint timeout = 10;
    uint clients_count = 1;
    string image_path = "racoon.jpg";
    
    vector<uint8_t> image;

    if (!load_image(image_path, image))
    {
        cerr << "Load " << image_path << " failed" << endl;
        return 1;
    }

    asio::io_service io;

    list<ProtoClient> clients; 

    for (uint i = 0; i < clients_count; i++)
    {
        clients.emplace_back(io, image, addr, port, text, timeout);
        clients.back().Run();
    }

    io.run();

    for (const auto& client : clients)
    {
        if (client.is_success())
        {
            save_image("output.jpg", client.image());
        }
    }

    return 0;
}