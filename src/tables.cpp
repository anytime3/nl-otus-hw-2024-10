#include "tables.h"
#include <algorithm>
#include <sstream>

void Tables::truncate(const std::string &tableId)
{
	if (tableId == "A")
	{
		std::lock_guard l(m_protectA);
		m_tableA.clear();
	}
	else if (tableId == "B")
	{
		std::lock_guard l(m_protectB);
		m_tableB.clear();
	}
}

bool Tables::insert(const std::string &tableId, int id, const std::string &name)
{
	if (tableId == "A")
	{
		std::lock_guard l(m_protectA);
		return m_tableA.try_emplace(id, name).second;
	}

	std::lock_guard l(m_protectB);
	return m_tableB.try_emplace(id, name).second;
}

std::string Tables::intersection() const
{
	std::scoped_lock l(m_protectA, m_protectB);
	std::stringstream res;

	auto itA = m_tableA.begin();
	auto itB = m_tableB.begin();
	while (true)
	{
		if (itA == m_tableA.end())
			break;
		if (itB == m_tableB.end())
			break;

		// update iterator that have lesser value
		int a_id = itA->first;
		int b_id = itB->first;
		if (a_id < b_id)
		{
			++itA;
			continue;
		}
		if (b_id < a_id)
		{
			++itB;
			continue;
		}


		res << a_id << ',' << itA->second << ',' << itB->second << '\n';
		++itA;
		++itB;
	}

	return res.str();
}

std::string Tables::symmetricDifference() const
{
	std::scoped_lock l(m_protectA, m_protectB);
	std::stringstream res;

	auto itA = m_tableA.begin();
	auto itB = m_tableB.begin();
	while (true)
	{
		if (itA == m_tableA.end())
			break;
		if (itB == m_tableB.end())
			break;
		int a_id = itA->first;
		int b_id = itB->first;
		if (a_id == b_id)
		{
			++itA;
			++itB;
			continue;
		}
		else if (a_id < b_id)
		{
			res << a_id << ',' << itA->second << ',' << '\n';
			++itA;
		}
		else // b_id < a_id
		{
			res << b_id << ",," << itB->second << '\n';
			++itB;
		}
	}


	for(;itA != m_tableA.end(); ++itA)
	{
		int a_id = itA->first;
		res << a_id << ',' << itA->second << ',' << '\n';
	}
	for (;itB != m_tableB.end(); ++itB)
	{
		int b_id = itB->first;
		res << b_id << ",," << itB->second << '\n';
	}

	return res.str();
}
