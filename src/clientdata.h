#pragma once
#include <boost/asio.hpp>
#include <string>
#include <memory>


struct ClientData
{
	std::shared_ptr<boost::asio::ip::tcp::socket> socket;
	std::string clientId;
};
