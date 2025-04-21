#pragma once
#include <boost/asio.hpp>
#include <string>
#include <memory>

/// \brief client-related data to use in map <client_id -> clientData>
struct ClientData
{
	std::shared_ptr<boost::asio::ip::tcp::socket> socket;
	std::string clientId;
};