#pragma once
#include <string>


struct ISender
{
	virtual ~ISender() = default;


	virtual void sendOK(const std::string &clientId) = 0;


	virtual void sendError(const std::string &clientId, const std::string &errDescription) = 0;


	virtual void sendResult(const std::string &clientId, const std::string &result) = 0;
};
