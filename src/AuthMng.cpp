#include "AuthMng.h"
//------------------------------------------------------------------------------
namespace
{
const std::string l_dbName = "http_server";
const std::string l_tableName = "users";
const std::string l_tableFields = "user_id serial PRIMARY KEY, login varchar(255) NOT NULL UNIQUE, password varchar(255) NOT NULL, session_id varchar(255) UNIQUE, is_admin BOOLEAN";
const std::string l_columns = "(login, password, session_id, is_admin)";
}
//------------------------------------------------------------------------------
AuthMng::AuthMng(const std::string & user, const std::string & password, loggerPtr & log) :
  DbMng(user, password, log)
{
  if (isDatabaseExist(l_dbName) == false)
    createDb(l_dbName);

  if (isTableExist(l_dbName, l_tableName) == false)
    createTable(l_dbName, l_tableName, l_tableFields);
  // addEntry(l_dbName, l_tableName, l_columns, "('nl', 'nl', NULL, false)");
  // editEntry(l_dbName, l_tableName, "session_id='6969'", "session_id IS NULL");
}
//------------------------------------------------------------------------------
