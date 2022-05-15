#pragma once

#include "asio.hpp"

/**
 * @class IoService синглтон для удобства доступа к единственному
 * io_service. Больше чем 1 инстанс нам тут и не потребуется
 */
class IoService
{
public:
	IoService() = delete;

	static asio::io_service& get()
	{
		static asio::io_service io;
		return io;
	}
};