#include <string>
#include <vector>
using namespace std;

#include "protocol.hpp"
#include "asio.hpp"

class ProtoClient final
{
public:
    using Image = vector<uint8_t>;
    
    ProtoClient(asio::io_service& io, const Image& image, const string& server_addr, int server_port, const string& text, uint timeout);

    ProtoClient(const ProtoClient&) = delete;
    ProtoClient& operator==(const ProtoClient&) = delete;

    void Run();

    uint id() const { return m_id; }

private:
    asio::io_service& m_io;
    const Image& m_image;
    string m_addr;
    int m_port;
    string m_text;
    uint m_reconnect_time;

    asio::ip::tcp::socket m_sock;
    asio::deadline_timer m_timer;

    uint m_id;
    vector<uint8_t> m_buffer;

    ProtoResponseHeader m_header;
    vector<uint8_t> m_result_image;

    void send_request();
    void on_receive(const asio::error_code& ec, size_t bytes);
    void on_receive_image(const asio::error_code& ec, size_t bytes);
};