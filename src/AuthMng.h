#ifndef AUTHMNG_H
#define AUTHMNG_H

#include "DbMng.h"

class AuthMng : public DbMng
{
public:
  AuthMng(const std::string & user, const std::string & password, loggerPtr & log);
  bool checkLoginAndPassword(const std::string & login, const std::string & password);
  ~AuthMng() override {}
};

#endif // AUTHMNG_H
