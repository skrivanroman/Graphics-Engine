#pragma once

#include <string>
#include <stdexcept>
#include <stdlib.h>
#include "Logger.hpp"

#define ASSERT_DEBUG_ON 1
#undef assert

static void assert(bool success, const char* message)
{
	if (!success)
		throw std::runtime_error(message);
}

static void assertDebug(bool success, const std::string& message)
{
	#if ASSERT_DEBUG_ON != 1
		return;
	#endif

	if (!success)
	{
		LOG_ERROR(message);
		exit(1);
	}
}
