#pragma once
#include "itables.h"
#include <deque>
#include <string>
#include <mutex>
#include <map>


class Tables : public ITables
{
public:
	void truncate(const std::string &tableId) override;
	bool insert(const std::string &tableId, int id, const std::string &name) override;
	std::string intersection() const override;
	std::string symmetricDifference() const override;

protected:
	std::map<int, std::string> m_tableA;
	std::map<int, std::string> m_tableB;

	// using two mutexes that protects their own tables
	mutable std::mutex m_protectA;
	mutable std::mutex m_protectB;
};
