#ifndef DBMNG_H
#define DBMNG_H
//------------------------------------------------------------------------------
#include <iostream>
#include <pqxx/pqxx>
#include <spdlog/spdlog.h>
//------------------------------------------------------------------------------
using loggerPtr = std::shared_ptr<spdlog::logger>;
//------------------------------------------------------------------------------
class DbMng
{
protected:
  std::string _user;
  std::string _password;
  loggerPtr _log;
public:
  DbMng(const std::string & user, const std::string & password, loggerPtr & log);

  bool isDatabaseExist(const std::string & dbName) const;
  bool createDb(const std::string & dbName) const;

  bool isTableExist(const std::string & dbName, const std::string & tableName) const;
  bool createTable(const std::string & dbName, const std::string & tableName, const std::string & fields) const;
  bool isExistInTable(const std::string & dbName, const std::string & tableName, const std::string & condition) const;

  bool addEntry(const std::string & dbName, const std::string & tableName, const std::string & columns, const std::string & values) const;
  bool deleteEntry(const std::string & dbName, const std::string & tableName, const std::string & condition) const;
  bool editEntry(const std::string & dbName, const std::string & tableName, const std::string & updateFields, const std::string & condition) const;
  void selectFrom(const std::string & dbName, const std::string & tableName, const std::string & select, const std::string & condition) const;
  virtual ~DbMng() {}
};
//------------------------------------------------------------------------------

#endif // DBMNG_H
