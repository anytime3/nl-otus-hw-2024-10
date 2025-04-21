#include "server.h"
#include <exception>
#include <iostream>
#include <sstream>
#include <chrono>
#include <cassert>
#include "validator.h"

using namespace std;
namespace bs = boost::system;
namespace ba = boost::asio;
using namespace std::placeholders;

Server::Server(uint16_t port) : m_port(port)
{
}

void Server::setClientDataFactory(std::unique_ptr<ClientDataFactory> clientDataFactory)
{
	m_clientDataFactory = std::move(clientDataFactory);
}

void Server::setTables(std::unique_ptr<ITables> tables)
{
	m_tables = std::move(tables);
}

void Server::start()
{
	using namespace boost::asio::ip;
	tcp::endpoint ep{ tcp::v4(), m_port };

	m_acceptor = make_unique<tcp::acceptor>(m_ioContext, ep);

	{
		boost::asio::socket_base::reuse_address option(true);
		m_acceptor->set_option(option); // allow fast server restarts
	}

	{
		boost::asio::socket_base::linger option(true, 0);
		m_acceptor->set_option(option);
	}

	restartAsyncAccept();

	m_serverResult = std::async(std::launch::async, &Server::contextThread, this);
}

void Server::restartAsyncAccept()
{
	std::shared_ptr<ba::ip::tcp::socket> socket = std::make_shared<ba::ip::tcp::socket>(m_ioContext);
	m_acceptor->async_accept(*socket, std::bind(&Server::onAccept, this, _1, socket));
}

void Server::stop()
{
	if (m_acceptor)
	{
		try {
			m_acceptor->cancel();
			m_ioContext.stop();
			m_serverResult.get();
		}
		catch (exception &e) {
			cerr << "Exception: " << e.what() << endl;
		}
		m_acceptor.reset();
	}

	m_clients.clear();
	m_clientDataFactory.reset();
}

void Server::contextThread()
{
	m_ioContext.run();
}

void Server::onAccept(bs::error_code ec, shared_ptr<ba::ip::tcp::socket> socket)
{
	if (ec)
	{
		if (!m_ioContext.stopped())
			cerr << "onAccept: " << ec.message() << endl;
		return;
	}

	//cout << "connection from " << socket->remote_endpoint().address() << ":" << socket->remote_endpoint().port() << endl; // TODO remove

	auto ep = socket->remote_endpoint();
	auto clientData = m_clientDataFactory->createClientData(socket); // тут важно сохранить куданть клиентский сокет, а то разорвёт соединение сразу
	m_clients.try_emplace(clientData->clientId, std::move(clientData));

	startRecv(socket);

	restartAsyncAccept();
}

void Server::startRecv(std::shared_ptr<boost::asio::ip::tcp::socket> client)
{
	client->async_read_some(ba::buffer(m_recvBuffer, std::size(m_recvBuffer)),
		std::bind(&Server::onRecv, this, _1, _2, client));
}

void Server::onRecv(boost::system::error_code ec, size_t len, shared_ptr<boost::asio::ip::tcp::socket> socket)
{
	std::string clientId = ClientDataFactory::asClientId(socket);
	if (ec)
	{
		if (auto it = m_clients.find(clientId); it != m_clients.end())
		{
			m_clients.erase(clientId);
			bs::error_code ec;
			socket->close(ec);
		}
		return;
	}

	if (auto it = m_clients.find(clientId); it != m_clients.end())
	{
		std::string s{ m_recvBuffer, len };

		unique_ptr<ClientData> &clientData = it->second;
		std::vector<std::string> tokens = tokenize(s);
		processTokens(tokens, clientId);
	}

	startRecv(socket);
}

inline std::string Server::trim(const std::string &s) const
{
	auto wsfront = std::find_if_not(s.begin(), s.end(), [](int c) {return std::isspace(c); });
	auto wsback = std::find_if_not(s.rbegin(), s.rend(), [](int c) {return std::isspace(c); }).base();
	return (wsback <= wsfront ? std::string() : std::string(wsfront, wsback));
}

std::vector<std::string> Server::tokenize(const std::string &input) const
{
	std::string trimmed = trim(input);
	std::stringstream ss{ trimmed };
	string token;
	std::vector<std::string> res;
	while (std::getline(ss, token, ' '))
		res.push_back(token);
	return res;
}

void Server::processTokens(const std::vector<std::string> &tokens, const std::string &clientId)
{
	ValidateResult vr = Validator::parse(tokens);
	if (!vr.ok)
	{
		sendError(clientId, vr.errorMessage);
		return;
	}

	switch (vr.command)
	{
	case CMD_ID::INSERT:
		if ( ! m_tables->insert(vr.parameters[0], std::stoi(vr.parameters[1]), vr.parameters[2]) )
		{
			sendError(clientId, "id must be unique");
			return;
		}
		break;
	case CMD_ID::INTERSECT:
		sendResult(clientId, m_tables->intersection());
		break;
	case CMD_ID::SYMMETRIC_DIFFERENCE:
		sendResult(clientId, m_tables->symmetricDifference());
		break;
	case CMD_ID::TRUNCATE:
		m_tables->truncate(vr.parameters[0]);
		break;
	default:
		assert(false);
		sendError(clientId, "no such command");
		return;
	}

	sendOK(clientId);
}

void Server::send(const std::string &clientId, const std::string &data)
{
	auto it = m_clients.find(clientId);
	if (it == m_clients.end())
		return;

	auto &clientData = it->second;
	clientData->socket->send(ba::buffer(data.c_str(), data.size()));
}

void Server::sendOK(const std::string &clientId)
{
	send(clientId, "OK\n");
}

void Server::sendError(const std::string &clientId, const std::string &errDescription)
{
	std::string err = "ERR " + errDescription + "\n";
	send(clientId, err);
}

void Server::sendResult(const std::string &clientId, const std::string &result)
{
	send(clientId, result);
}
