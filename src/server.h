#pragma once
#include <cstdint>
#include <future>
#include <map>
#include <vector>
#include <string>
#include <boost/asio.hpp>
#include "isender.h"
#include "clientdatafactory.h"
#include "itables.h"


class Server : public ISender
{
public:

	Server(uint16_t port);


	void setClientDataFactory(std::unique_ptr<ClientDataFactory> clientDataFactory);


	void setTables(std::unique_ptr<ITables> tables);


	void start();


	void stop();

protected:
	uint16_t m_port;
	boost::asio::io_context m_ioContext;
	std::future<void> m_serverResult;
	void contextThread();

	std::unique_ptr<boost::asio::ip::tcp::acceptor> m_acceptor;
	void restartAsyncAccept();
	void onAccept(boost::system::error_code ec, std::shared_ptr<boost::asio::ip::tcp::socket> socket);

	std::unique_ptr<ClientDataFactory> m_clientDataFactory;
	std::map<std::string, std::unique_ptr<ClientData>> m_clients;

	void startRecv(std::shared_ptr<boost::asio::ip::tcp::socket> client);
	void onRecv(boost::system::error_code ec, size_t len, std::shared_ptr<boost::asio::ip::tcp::socket> socket);
	static const size_t MAX_RECV_BUFFER_SIZE = 1024;
	char m_recvBuffer[1024] = {};

	std::string trim(const std::string &s) const;
	std::vector<std::string> tokenize(const std::string &input) const;
	void processTokens(const std::vector<std::string> &tokens, const std::string &clientId);

	void send(const std::string &clientId, const std::string &data);
	void sendOK(const std::string &clientId) override;
	void sendError(const std::string &clientId, const std::string &errDescription) override;
	void sendResult(const std::string &clientId, const std::string &result) override;

	std::unique_ptr<ITables> m_tables;

};
