#include "validator.h"

ValidateResult Validator::parse(const std::vector<std::string> &tokens)
{
	if (tokens.size() == 0)
		return ValidateResult{ false, "no commands detected" };
	const std::string &command = tokens[0];
	if (command == "INSERT")
		return parseInsertCommand(tokens);
	else if (command == "TRUNCATE")
		return parseTruncateCommand(tokens);
	else if (command == "INTERSECTION")
		return parseIntersectionCommand(tokens);
	else if (command == "SYMMETRIC_DIFFERENCE")
		return parseSymmetricDifferenceCommand(tokens);
	else
		return ValidateResult{ false, "available commands: INSERT, TRUNCATE, INTERSECTION, SYMMETRIC_DIFFERENCE" };
}

bool Validator::validateTableName(const std::string &tableName)
{
	if (tableName != "A" && tableName != "B")
		return false;
	return true;
}

ValidateResult Validator::parseInsertCommand(const std::vector<std::string> &tokens)
{
	if (tokens.size() < 4)
		return ValidateResult{ false, "format: INSERT table id name, separator = one space exactly" };

	const std::string &tableName = tokens[1];
	if( ! validateTableName(tableName) )
		return ValidateResult{ false, "invalid table name, must be A or B" };

	int id;
	const std::string &idName = tokens[2];
	try { id = std::stoi(idName); }
	catch (...) { 
		return ValidateResult{ false, "invalid id:int format" };
	}

	const std::string &name = tokens[3];
	if (name.empty())
		return ValidateResult{ false, "invalid name:string parameter, check spacing (must be exactly 1 space)" };
	
	return ValidateResult{ true, "", CMD_ID::INSERT, std::vector{tableName, idName, name} };
}

ValidateResult Validator::parseTruncateCommand(const std::vector<std::string> &tokens)
{
	if (tokens.size() < 2)
		return ValidateResult{ false, "format: TRUNCATE table" };

	const std::string &tableName = tokens[1];
	if (!validateTableName(tableName))
		return ValidateResult{ false, "invalid table name, must be A or B" };

	return ValidateResult{ true, "", CMD_ID::TRUNCATE, std::vector{tableName} };
}

ValidateResult Validator::parseIntersectionCommand(const std::vector<std::string> &tokens)
{
	return ValidateResult{ true, "", CMD_ID::INTERSECT };
}

ValidateResult Validator::parseSymmetricDifferenceCommand(const std::vector<std::string> &tokens)
{
	return ValidateResult{ true, "", CMD_ID::SYMMETRIC_DIFFERENCE };
}