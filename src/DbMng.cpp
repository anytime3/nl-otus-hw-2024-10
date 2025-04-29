#include "DbMng.h"
//------------------------------------------------------------------------------
DbMng::DbMng(const std::string & user, const std::string & password, loggerPtr & log) :
  _user(user),
  _password(password),
  _log(log)
{

}
//------------------------------------------------------------------------------
bool DbMng::isDatabaseExist(const std::string & dbName) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error("Can not open connection to Postgres");
      return false;
    }

    pqxx::nontransaction txn(conn);
    std::string execStr = "SELECT * FROM pg_database WHERE datname = " + txn.quote(dbName);
    pqxx::result res = txn.exec(execStr);
    conn.close();

    return !res.empty();
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in isDatabaseExist! {} "), err.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool DbMng::createDb(const std::string & dbName) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
      return false;
    }

    pqxx::nontransaction txn(conn);
    std::string execStr = "CREATE DATABASE " + dbName;
    pqxx::result res = txn.exec(execStr);
    conn.close();

    _log->info(SPDLOG_FMT_STRING("Created db {}"), dbName);
    return true;
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in createDB! {} "), err.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool DbMng::isTableExist(const std::string & dbName, const std::string & tableName) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password + " dbname=" + dbName;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
      return false;
    }

    pqxx::nontransaction txn(conn);
    std::string execStr = "SELECT * FROM information_schema.tables WHERE table_schema = 'public' AND table_name =" + txn.quote(tableName);
    pqxx::result res = txn.exec(execStr);
    conn.close();

    return !res.empty();
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in isTableExist! {} "), err.what());
    return false;
  }
}

//------------------------------------------------------------------------------
bool DbMng::createTable(const std::string & dbName, const std::string & tableName, const std::string & fields) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password + " dbname=" + dbName;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
      return false;
    }

    pqxx::work txn(conn);
    std::string execStr = "CREATE TABLE " + tableName + " (" + fields + ")";
    pqxx::result res = txn.exec(execStr);
    txn.commit();
    conn.close();

    _log->info(SPDLOG_FMT_STRING("Created table {} in db {}"), tableName, dbName);
    return true;
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in createTable! {} "), err.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool DbMng::isExistInTable(const std::string & dbName, const std::string & tableName, const std::string & condition) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password + " dbname=" + dbName;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
      return false;
    }

    pqxx::nontransaction txn(conn);
    std::string execStr = "SELECT EXISTS (SELECT * FROM " + tableName + " WHERE " + condition + ")";
    pqxx::result res = txn.exec(execStr);
    conn.close();

    return res[0][0].as<bool>();
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in isTableExist! {} "), err.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool DbMng::addEntry(const std::string & dbName, const std::string & tableName, const std::string & columns, const std::string & values) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password + " dbname=" + dbName;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
      return false;
    }

    pqxx::work txn(conn);
    std::string execStr = "INSERT INTO " + tableName + " " + columns + " VALUES " + values;
    pqxx::result res = txn.exec(execStr);
    txn.commit();
    conn.close();

    _log->info(SPDLOG_FMT_STRING("Added values {} in table {} in db {}"), values, tableName, dbName);
    return true;
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in addEntry! {} "), err.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool DbMng::deleteEntry(const std::string &dbName, const std::string &tableName, const std::string &condition) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password + " dbname=" + dbName;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
      return false;
    }

    pqxx::work txn(conn);
    std::string execStr = "DELETE FROM " + tableName + " WHERE " + condition;
    pqxx::result res = txn.exec(execStr);
    txn.commit();
    conn.close();

    if (res.empty())
      return false;
    _log->info(SPDLOG_FMT_STRING("Deleted entry from table {} in db {} where {}. Count: {}"), tableName, dbName, condition, res.size());
    return true;
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in deleteEntry! {} "), err.what());
    return false;
  }
}
//------------------------------------------------------------------------------
bool DbMng::editEntry(const std::string & dbName, const std::string & tableName, const std::string & updateFields, const std::string & condition) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password + " dbname=" + dbName;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
      return false;
    }

    pqxx::work txn(conn);
    std::string execStr = "UPDATE " + tableName + " SET " + updateFields + " WHERE " + condition;
    pqxx::result res = txn.exec(execStr);
    txn.commit();
    conn.close();

    _log->info(SPDLOG_FMT_STRING("Updated fields {} in table {} in db {}"), updateFields, tableName, dbName);
    return true;
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in editEntry! {} "), err.what());
    return false;
  }
}
//------------------------------------------------------------------------------
void DbMng::selectFrom(const std::string & dbName, const std::string & tableName, const std::string & select, pqxx::result & res) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password + " dbname=" + dbName;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
    }

    pqxx::nontransaction txn(conn);
    std::string execStr = "SELECT " + select + " FROM " + tableName;
    res = txn.exec(execStr);
    conn.close();
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in selectFrom! {} "), err.what());
  }
}
//------------------------------------------------------------------------------
void DbMng::selectFromWhere(const std::string & dbName, const std::string & tableName, const std::string & select, const std::string & condition, pqxx::result & res) const
{
  try
  {
    std::string connStr = "host=127.0.0.1 port=5432 user=" + _user + " password=" + _password + " dbname=" + dbName;
    pqxx::connection conn(connStr);

    if (conn.is_open() == false)
    {
      _log->error(SPDLOG_FMT_STRING("Can not open connection to db {}"), dbName);
    }

    pqxx::nontransaction txn(conn);
    std::string execStr = "SELECT " + select + " FROM " + tableName + " WHERE " + condition;
    res = txn.exec(execStr);
    conn.close();
  } catch (const std::exception & err)
  {
    _log->error(SPDLOG_FMT_STRING("Exception in selectFromWhere! {} "), err.what());
  }
}
//------------------------------------------------------------------------------

