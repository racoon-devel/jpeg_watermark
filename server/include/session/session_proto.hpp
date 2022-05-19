#pragma once

#include "session.hpp"

#include "protocol.hpp"

/**
 * @class ProtoSession тип сессии для обмена с клиентом данными по своему
 * протоколу
 */
class ProtoSession : public Session
{
public:
	/**
	 * Ctor
	 * @param invoker Интерфейс для выполнения задач
	 * @param socket Сокет соединения
	 */
	ProtoSession(IInvoker& invoker, asio::ip::tcp::socket&& socket);

	void run() override;

protected:
	void on_received() override;
	void on_sent() override;

private:
	enum State
	{
		kReceivingHeader,
		kReceivingPayload,
		kReceivingPayloadContinuation,
	};

	State                  m_state{kReceivingHeader};
	std::vector< uint8_t > m_recv_buffer;
	std::vector< uint8_t > m_send_buffer;

	proto::PayloadType m_type;

	union
	{
		proto::PrintTextPayload print_text_p;
	} m_payload{};

	TaskPtr m_task;

	void send_response(proto::Status status, const Image& image = {});

	void on_header_received();
	void on_request_received();
	void on_continuation_received();
	void on_complete();

	void process_print_text();
};