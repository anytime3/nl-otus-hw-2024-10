#ifndef CONTENTMNG_H
#define CONTENTMNG_H
//------------------------------------------------------------------------------
#include "DbMng.h"
#include <boost/json.hpp>
//------------------------------------------------------------------------------
class ContentMng : public DbMng
{
public:
  ContentMng() : DbMng() {}
  ContentMng(const std::string & user, const std::string & password, loggerPtr & log);
  bool addPost(const std::string & user, const std::string & theme, const std::string & data);
  void getAllPosts(boost::json::array & posts);
};
//------------------------------------------------------------------------------
#endif // CONTENTMNG_H
//------------------------------------------------------------------------------
