#include "task/task.hpp"

void Task::execute() noexcept
{
	try
	{
		m_result = on_execute();
	}
	catch (...)
	{
		m_exception = std::current_exception();
	}

	m_handler();
}

const Image& Task::result() const
{
	if (m_exception)
	{
		std::rethrow_exception(m_exception);
	}

	return m_result;
}
