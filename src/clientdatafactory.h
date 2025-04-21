#pragma once
#include <memory>
#include <vector>
#include <deque>
#include "clientdata.h"

class ClientDataFactory
{
public:

	std::unique_ptr<ClientData> createClientData(std::shared_ptr<boost::asio::ip::tcp::socket> socket);


	static std::string asClientId(const std::shared_ptr<boost::asio::ip::tcp::socket> &socket);
};
