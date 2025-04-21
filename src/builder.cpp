#include "builder.h"
#include "clientdatafactory.h"
#include "tables.h"

std::unique_ptr<Server> Builder::build(uint16_t port)
{
	using namespace std;
	
	auto clientDataFactory = make_unique<ClientDataFactory>();
	auto tables = make_unique<Tables>();
	auto server = make_unique<Server>(port);

	server->setTables(std::move(tables));
	server->setClientDataFactory(std::move(clientDataFactory));

	return server;
}
