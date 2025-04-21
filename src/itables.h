#pragma once
#include <string>


struct ITables
{
	virtual ~ITables() = default;


	virtual void truncate(const std::string &tableId) = 0;


	virtual bool insert(const std::string &tableId, int id, const std::string &name) = 0;


	virtual std::string intersection() const = 0;


	virtual std::string symmetricDifference() const = 0;
};
