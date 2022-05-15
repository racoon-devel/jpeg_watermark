#pragma once

#include "asio.hpp"

using DeadlineTimer = asio::basic_waitable_timer< std::chrono::steady_clock >;