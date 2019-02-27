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

        /* Таймер нужен для очистки завершенных сессий - не очень красиво, но здесь большой нагрузки не ожидается */
        m_timer.expires_from_now(boost::posix_time::milliseconds(m_clear_timeout));
        m_timer.async_wait(std::bind(&Server::on_tick, this, std::placeholders::_1));

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

    auto session_ptr = std::make_shared<ProtoSession>(m_proc, std::move(m_sock));

    m_sessions.push_back(session_ptr);

    session_ptr->start();

    m_acceptor.async_accept(m_sock, std::bind(&Server::on_accept, this, std::placeholders::_1));
}

void Server::on_tick(const asio::error_code& ec)
{
    if (ec)
    {
        std::cerr << "Timer error: " << ec.message() << std::endl;
        return ;
    }

    std::cerr << "Clear resources" << std::endl;

    m_sessions.remove_if([] (SessionPtr& session) { return session->is_done(); });

    m_timer.expires_from_now(boost::posix_time::milliseconds(m_clear_timeout));
    m_timer.async_wait(std::bind(&Server::on_tick, this, std::placeholders::_1));

    std::cerr << "Active sessions: " << m_sessions.size() << std::endl;
}