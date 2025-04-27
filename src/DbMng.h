#ifndef DBMNG_H
#define DBMNG_H
//------------------------------------------------------------------------------
#include <iostream>
#include <libpq-fe.h>
//------------------------------------------------------------------------------
class DbMng
{
protected:
  std::string _user;
  std::string _password;
public:
  DbMng(const std::string & user, const std::string & password);

  bool isDatabaseExists(const std::string & dbName) const;
  void createDb(const std::string & dbName) const;

  bool isTableExists(const std::string & dbName, const std::string & tableName) const;
  void createTable(const std::string & dbName, const std::string & tableName, const std::string & fields) const;
  int isExistsInTable(const std::string & condition, const std::string & dbName, const std::string & tableName) const;

  virtual ~DbMng() = 0;
};
//------------------------------------------------------------------------------

#endif // DBMNG_H
