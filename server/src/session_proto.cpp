#include "session_proto.hpp"

#include <easylogging++.h>
#include <io_service.hpp>
#include <task/watermark_task.hpp>

#include "limits.hpp"

using namespace proto;

std::ostream& operator<<(std::ostream& os, const ProtoSession* session)
{
	os << "[Session:" << session->identity() << "] ";
	return os;
}

ProtoSession::ProtoSession(IInvoker& invoker, asio::ip::tcp::socket&& socket)
	: Session(invoker, std::move(socket))
{
}

void ProtoSession::run()
{
	m_recv_buffer.resize(sizeof(Header));
	receive(m_recv_buffer);
}

void ProtoSession::on_received()
{
	switch (m_state)
	{
	case kReceivingHeader:
		on_header_received();
		break;

	case kReceivingPayload:
		on_request_received();
		break;

	case kReceivingPayloadContinuation:
		on_continuation_received();
		break;
	}
}

void ProtoSession::on_sent()
{
	done();
}

void ProtoSession::on_header_received()
{
	auto header = reinterpret_cast< Header* >(m_recv_buffer.data());
	if (header->sign != Signature)
	{
		throw std::runtime_error("Malformed protocol header");
	}

	m_type = header->type;

	switch (header->type)
	{
	case PayloadType::kPrintTextRequest:
		m_recv_buffer.resize(sizeof(PrintTextPayload));
		m_state = kReceivingPayload;
		receive(m_recv_buffer);
		break;

	default:
		throw std::runtime_error("Invalid packet payload type");
	}
}

void ProtoSession::on_request_received()
{
	switch (m_type)
	{
	case PayloadType::kPrintTextRequest:
	{
		auto header =
			reinterpret_cast< PrintTextPayload* >(m_recv_buffer.data());

		if (header->text_size > Limit::kMaxTextLength
			|| header->image_size > Limit::kMaxImageSize)
		{
			LOG(WARNING) << this << "Limits exceeded";
			send_response(Status::kLimit);
			return;
		}

		m_payload.print_text_p = *header;

		m_state = kReceivingPayloadContinuation;
		m_recv_buffer.resize(header->text_size + header->image_size);
		receive(m_recv_buffer);
		break;
	}

	default:
		throw std::logic_error("invalid payload type");
	}
}

void ProtoSession::send_response(Status status, const Image& image)
{
	m_send_buffer.resize(sizeof(Header) + sizeof(ResponsePayload)
						 + image.size());

	auto header  = reinterpret_cast< Header* >(m_send_buffer.data());
	header->sign = Signature;
	header->size = sizeof(ResponsePayload) + image.size();
	header->type = PayloadType::kResponse;

	auto response    = reinterpret_cast< ResponsePayload* >(m_send_buffer.data()
                                                         + sizeof(Header));
	response->status = status;
	response->image_size = image.size();

	std::copy(image.begin(), image.end(),
			  m_send_buffer.begin() + sizeof(Header) + sizeof(ResponsePayload));

	send(m_send_buffer);
}

void ProtoSession::on_continuation_received()
{
	switch (m_type)
	{
	case PayloadType::kPrintTextRequest:
		process_print_text();
		break;

	default:
		throw std::logic_error("invalid payload type");
	}
}

void ProtoSession::process_print_text()
{
	std::string text(m_recv_buffer.begin(),
					 m_recv_buffer.begin() + m_payload.print_text_p.text_size);
	Image       image(m_recv_buffer.begin() + m_payload.print_text_p.text_size,
					  m_recv_buffer.end());

	LOG(DEBUG) << this << "Image received";

	auto completeHandler = [session = shared_from_this()]() noexcept
	{
		IoService::get().post([session] { session->on_complete(); });
	};

	m_task = std::make_shared< WatermarkTask >(
		std::move(completeHandler), std::move(image), std::move(text));

	if (!m_invoker.invoke(m_task))
	{
		LOG(WARNING) << this << "Request rejected, max jobs count reached";
		send_response(Status::kBusy);
	}
}

void ProtoSession::on_complete()
{
	try
	{
		const auto& image_buffer = m_task->result();
		send_response(Status::kOk, image_buffer);
	}
	catch (const std::exception& e)
	{
		LOG(ERROR) << "Process task failed: " << e.what();
		send_response(Status::kError);
	}
	LOG(INFO) << this << "Image job done";
}
