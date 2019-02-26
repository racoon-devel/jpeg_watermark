
#include <iostream>
#include <string>
using namespace std;

#include "protocol.hpp"

#include "asio.hpp"

int main(int argc, char **argv)
{
    (void) argc; (void)argv;
    // cout << "Usage: <jpeg_source_filepath> <server_addr> <server_port> <timeout> <jpeg_output_filename>" << endl;
    asio::io_context io;
    asio::ip::tcp::socket sock(io); 
    asio::ip::tcp::endpoint ep(asio::ip::address::from_string("127.0.0.1"), 9001);

    vector<uint8_t> text; text.resize(100, 0);
    vector<uint8_t> data; data.resize(1000, 0);

    ProtoDataHeader header;
    header.sign = PROTO_VALID_SIGN;
    header.image_size = data.size();
    header.text_size = text.size();

    try 
    {
        sock.open(asio::ip::tcp::v4());
        sock.connect(ep);

        sock.send(asio::buffer((void*) &header, sizeof(header)));
        sock.send(asio::buffer(&text[0], text.size()));
        sock.send(asio::buffer(&data[0], data.size()));

        ProtoResponseHeader response;
        sock.receive(asio::buffer((void *) &response, sizeof(response)));

        cout << "Response: " << response.code << " " << response.image_size << endl;
        vector<uint8_t> v; v.resize(response.image_size);
        sock.receive(asio::buffer(&v[0], response.image_size));
        sock.close();
    }
    catch (const exception& e)
    {
        cerr << "Connect failed: " << e.what() << endl;
    }


    return 0;
}