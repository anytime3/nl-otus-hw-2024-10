#include "clientdatafactory.h"

using namespace std;

unique_ptr<ClientData> ClientDataFactory::createClientData(shared_ptr<boost::asio::ip::tcp::socket> socket)
{
	unique_ptr<ClientData> cd = make_unique<ClientData>();

	cd->clientId = asClientId(socket);
	cd->socket = socket;

	return std::move(cd);
}

string ClientDataFactory::asClientId(const shared_ptr<boost::asio::ip::tcp::socket> &socket)
{
	const auto &ep = socket->remote_endpoint();
	return ep.address().to_string() + ":" + to_string(ep.port());
}