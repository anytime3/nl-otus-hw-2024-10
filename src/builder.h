#pragma once
#include <memory>
#include <cstdint>
#include "server.h"

/// \brief builder pattern
struct Builder
{
	/// \brief builds a server instance
	static std::unique_ptr<Server> build(uint16_t port);
};