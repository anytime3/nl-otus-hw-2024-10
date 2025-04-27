#include "DbMng.h"
#include <exception>
//------------------------------------------------------------------------------
DbMng::DbMng(const std::string & user, const std::string & password) :
  _user(user),
  _password(password)
{

}
//------------------------------------------------------------------------------
bool DbMng::isDatabaseExists(const std::string & dbName) const
{
  PGconn *conn = nullptr;
  std::string conninfo = "host=127.0.0.1 user=" + _user + "password=" + _password;
  conn = PQconnectdb(conninfo.c_str());

  std::string query = "SELECT 1 FROM pg_database WHERE datname = '" + dbName + "'";
  PGresult* res = PQexec(conn, query.c_str());

  bool exists = (PQntuples(res) > 0);
  PQclear(res);
  PQfinish(conn);
  return exists;
}
//------------------------------------------------------------------------------
void DbMng::createDb(const std::string & dbName) const
{
  std::string conninfo = "host=127.0.0.1 user=" + _user + "password=" + _password;
  PGconn* conn = PQconnectdb(conninfo.c_str());

  if (PQstatus(conn) != CONNECTION_OK) {
    PQfinish(conn);
    throw std::runtime_error("Can not connect to DB");
  }

  std::string query = "CREATE DATABASE \"" + dbName + "\"";
  query += " WITH TEMPLATE = template1";
  query += " ENCODING = 'UTF8'";
  query += " OWNER = postgres";

  PGresult* res = PQexec(conn, query.c_str());

  bool success = true;
  if (PQresultStatus(res) != PGRES_COMMAND_OK) {
      std::cout << "Can not create DB " << dbName << std::endl;
      success = false;
  }

  PQclear(res);
  PQfinish(conn);
}
//------------------------------------------------------------------------------
bool DbMng::isTableExists(const std::string & dbName, const std::string & tableName) const
{
  PGconn *conn = nullptr;
  std::string schemaName = "public";
  std::string conninfo = "host=127.0.0.1 user=" + _user + "password=" + _password + " dbname=" + dbName;
  conn = PQconnectdb(conninfo.c_str());
  if(PQstatus(conn) != CONNECTION_OK)
  {
    PQfinish(conn);
    throw std::runtime_error("Can not connect to DB");
  }

   std::string query = "SELECT 1 FROM information_schema.tables "
                      "WHERE table_schema = 'public' AND table_name =" + tableName;

   PGresult* res = PQexec(conn, query.c_str());

   if (PQresultStatus(res) != PGRES_TUPLES_OK)
   {
     PQclear(res);
     throw std::runtime_error(std::string("Can not check is table exists " + tableName));
   }

   bool exists = (PQntuples(res) > 0);
   PQclear(res);
   return exists;
}
//------------------------------------------------------------------------------
void DbMng::createTable(const std::string & dbName, const std::string & tableName, const std::string & fields) const
{
  PGconn *conn = nullptr;
  std::string conninfo = "host=127.0.0.1 user=" + _user + "password=" + _password + " dbname=" + dbName;

  conn = PQconnectdb(conninfo.c_str());
  if(PQstatus(conn) != CONNECTION_OK)
  {
    PQfinish(conn);
    throw std::runtime_error("Can not connect to DB");
  }

  PGresult *res = nullptr;
  res = PQexec(conn, std::string("CREATE TABLE " + tableName + " (" + fields + ");").c_str());

  if(PQresultStatus(res) != PGRES_COMMAND_OK)
  {
    PQclear(res);
    PQfinish(conn);
    throw std::runtime_error(std::string("Can not CREATE TABLE " + tableName));
  }
}
//------------------------------------------------------------------------------
int DbMng::isExistsInTable(const std::string & condition, const std::string & dbName, const std::string & tableName) const
{
  PGconn *conn = nullptr;
  std::string conninfo = "host=127.0.0.1 user=" + _user + "password=" + _password + " dbname=" + dbName;
  conn = PQconnectdb(conninfo.c_str());
  if(PQstatus(conn) != CONNECTION_OK)
  {
    PQfinish(conn);
    throw std::runtime_error("Can not connect to DB");
  }

  PGresult *res = nullptr;
  res = PQexec(conn, std::string("SELECT * FROM " + tableName + " WHERE " + condition + ";").c_str());

  if(PQresultStatus(res) != PGRES_TUPLES_OK)
  {
    PQclear(res);
    PQfinish(conn);
    throw std::runtime_error(std::string("Can not SELECT " + condition));
  }

  int n_tuples = PQntuples(res);
  PQclear(res);
  PQfinish(conn);
  return n_tuples;
}
//------------------------------------------------------------------------------
