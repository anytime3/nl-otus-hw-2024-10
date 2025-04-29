#include "ContentMng.h"
//------------------------------------------------------------------------------
namespace
{
const std::string l_dbName = "http_server";

const std::string l_contentTableName = "content";
const std::string l_contentTableFields = "post_id serial PRIMARY KEY, username varchar(255) NOT NULL, theme varchar(255) NOT NULL, data varchar(2048) NOT NULL";
const std::string l_contentColumns = "(username, theme, data)";
}
//------------------------------------------------------------------------------
ContentMng::ContentMng(const std::string & user, const std::string & password, loggerPtr & log) :
  DbMng(user, password, log)
{
  if (isDatabaseExist(l_dbName) == false)
    createDb(l_dbName);

  if (isTableExist(l_dbName, l_contentTableName) == false)
    createTable(l_dbName, l_contentTableName, l_contentTableFields);
}

bool ContentMng::addPost(const std::string & user, const std::string & theme, const std::string & data)
{
  std::string values = "('" + user + "', '" + theme + "', '" + data + "')";
  return addEntry(l_dbName, l_contentTableName, l_contentColumns, values);
}
//------------------------------------------------------------------------------
void ContentMng::getAllPosts(boost::json::array & posts)
{
  std::string select = "username, theme, data";
  pqxx::result res;
  selectFrom(l_dbName, l_contentTableName, select, res);
  for (const auto & row : res)
  {
    boost::json::object post;
    post["username"] = row["username"].c_str();
    post["theme"] = row["theme"].c_str();
    post["data"] = row["data"].c_str();
    posts.push_back(post);
  }
}
//------------------------------------------------------------------------------
