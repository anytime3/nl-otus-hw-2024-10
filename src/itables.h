#pragma once
#include <string>

/// \brief interface to perform tables operations
struct ITables
{
	virtual ~ITables() = default;

	/// \brief clears specified table
	virtual void truncate(const std::string &tableId) = 0;

	/// \brief inserts data to the specified table
	virtual bool insert(const std::string &tableId, int id, const std::string &name) = 0;

	/// \brief performs intersection on the tables A and B
	virtual std::string intersection() const = 0;

	/// \brief performs symmetric difference operation on the tables A and B
	virtual std::string symmetricDifference() const = 0;
};