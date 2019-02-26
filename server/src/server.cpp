#include <iostream>

#include "server.hpp"
#include "session.hpp"

int Server::Run()
{
    try
    {
        asio::signal_set signals(m_io, SIGINT, SIGTERM);

        signals.async_wait(std::bind(&Server::on_stop, this, 
            std::placeholders::_1, std::placeholders::_2));

        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(m_addr), m_port);
        
        m_acceptor.open(endpoint.protocol());
        m_acceptor.set_option(asio::ip::tcp::acceptor::reuse_address(true));
        m_acceptor.bind(endpoint);
        m_acceptor.listen();

        m_acceptor.async_accept(m_sock, std::bind(&Server::on_accept, this, std::placeholders::_1));

        m_proc.Run();

        m_io.run();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Run server failed: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

void Server::shutdown()
{
    std::cerr << "Shutdowning..." << std::endl;

    m_proc.Stop();

    m_io.stop();
}

void Server::on_stop(const asio::error_code& ec, int signal_number)
{
    (void) ec; (void) signal_number;

    shutdown();
}

void Server::on_accept(const asio::error_code& ec)
{
    if (ec)
    {
        std::cerr << "Accept failed: " << ec.message() << std::endl;
        shutdown();
        return;
    }

    std::cerr << "Client accepted: " << m_sock.remote_endpoint().address().to_string()  
        << std::endl;

    Session * session = new ProtoSession(m_proc, std::move(m_sock));
    session->start();

    m_acceptor.async_accept(m_sock, std::bind(&Server::on_accept, this, std::placeholders::_1));
}