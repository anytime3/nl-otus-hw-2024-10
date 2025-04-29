#include "AuthMng.h"
//------------------------------------------------------------------------------
namespace
{
const std::string l_dbName = "http_server";

const std::string l_usersTableName = "users";
const std::string l_usersTableFields = "user_id serial PRIMARY KEY, login varchar(255) NOT NULL UNIQUE, password varchar(255) NOT NULL, is_admin BOOLEAN";
const std::string l_usersColumns = "(login, password, is_admin)";

const std::string l_sessionsTableName = "sessions";
const std::string l_sessionsTableFields = "session_id varchar(255) PRIMARY KEY, login varchar(255) NOT NULL UNIQUE, expires_at TIMESTAMP";
const std::string l_sessionsColumns = "(session_id, login, expires_at)";
}
//------------------------------------------------------------------------------
AuthMng::AuthMng(const std::string & user, const std::string & password, loggerPtr & log) :
  DbMng(user, password, log),
  _minExpires("30")
{
  if (isDatabaseExist(l_dbName) == false)
    createDb(l_dbName);

  if (isTableExist(l_dbName, l_usersTableName) == false)
    createTable(l_dbName, l_usersTableName, l_usersTableFields);

  if (isTableExist(l_dbName, l_sessionsTableName) == false)
    createTable(l_dbName, l_sessionsTableName, l_sessionsTableFields);
}
//------------------------------------------------------------------------------
bool AuthMng::checkLogin(const std::string & login) const
{
  std::string query = "login='" + login + "'";
  return isExistInTable(l_dbName, l_usersTableName, query);
}
//------------------------------------------------------------------------------
bool AuthMng::checkSession(const std::string &session_id) const
{
  std::string query = "session_id='" + session_id + "'";
  return isExistInTable(l_dbName, l_sessionsTableName, query);
}
//------------------------------------------------------------------------------
bool AuthMng::logoutBySessionId(const std::string & session_id)
{
  std::string condition = "session_id='" + session_id + "'";
  return deleteEntry(l_dbName, l_sessionsTableName, condition);
}
//------------------------------------------------------------------------------
bool AuthMng::logoutByLogin(const std::string & login)
{
  std::string condition = "login='" + login + "'";
  return deleteEntry(l_dbName, l_sessionsTableName, condition);
}
//------------------------------------------------------------------------------
bool AuthMng::registerUser(const std::string & login, const std::string & password) const
{
  if (checkLogin(login))
    return false;
  std::string values = "('" + login + "', '" + password + "', false)";
  return addEntry(l_dbName, l_usersTableName, l_usersColumns, values);
}
//------------------------------------------------------------------------------
bool AuthMng::setSessionIdByLogin(const std::string & session_id, const std::string & login) const
{
  std::string condition = "session_id='" + session_id + "'";
  if (isExistInTable(l_dbName, l_sessionsTableName, condition))
    deleteEntry(l_dbName, l_sessionsTableName, condition);

  condition = "('" + session_id + "', '" + login + "', now() + interval '" + _minExpires + " minutes')";
  return addEntry(l_dbName, l_sessionsTableName, l_sessionsColumns, condition);
}
//------------------------------------------------------------------------------
void AuthMng::deleteExpiredSessions() const
{
  std::string condition = "expires_at <= now()";
  deleteEntry(l_dbName, l_sessionsTableName, condition);
}
//------------------------------------------------------------------------------
