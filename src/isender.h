#pragma once
#include <string>

/// \brief interface to class that do communications with clients
struct ISender
{
	virtual ~ISender() = default;

	/// \brief sends OK to specified client
	virtual void sendOK(const std::string &clientId) = 0;

	/// \brief sends error with description to specified client
	virtual void sendError(const std::string &clientId, const std::string &errDescription) = 0;

	/// \brief sends result to specified client
	virtual void sendResult(const std::string &clientId, const std::string &result) = 0;
};