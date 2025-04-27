#include "AuthMng.h"

namespace
{
const std::string l_dbName = "http_server_users";
const std::string l_tableName = "users";
const std::string l_tableFields = "user_id int NOT NULL AUTO_INCREMENT, login varchar(255) NOT NULL UNIQUE, password varchar(255) NOT NULL, session_id varchar(255), is_admin  NOT NULL BOOLEAN, PRIMARY KEY (user_id)";
}
AuthMng::AuthMng(const std::string & user, const std::string & password)  :
  DbMng(user, password)
{
  if (isDatabaseExists(l_dbName) == false)
    createDb(l_dbName);

  if (isTableExists(l_dbName, l_tableName) == false)
    createTable(l_dbName, l_tableName, l_tableFields);
}
