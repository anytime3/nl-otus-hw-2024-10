#pragma once
#include <memory>
#include <cstdint>
#include "server.h"


struct Builder
{

	static std::unique_ptr<Server> build(uint16_t port);
};
