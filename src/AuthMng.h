#ifndef AUTHMNG_H
#define AUTHMNG_H

#include "DbMng.h"

class AuthMng : public DbMng
{
public:
  AuthMng(const std::string & user, const std::string & password);
};

#endif // AUTHMNG_H
