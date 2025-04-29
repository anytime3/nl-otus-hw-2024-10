#ifndef AUTHMNG_H
#define AUTHMNG_H
//------------------------------------------------------------------------------
#include "DbMng.h"
//------------------------------------------------------------------------------
class AuthMng : public DbMng
{
private:
  std::string _minExpires;
public:
  AuthMng() : DbMng() {}
  AuthMng(const std::string & user, const std::string & password, loggerPtr & log);
  bool checkLogin(const std::string & login) const;
  bool checkSession(const std::string & sessionId) const;
  bool logoutBySessionId(const std::string & sessionId);
  bool logoutByLogin(const std::string & login);
  bool registerUser(const std::string & login, const std::string & password) const;
  bool setSessionIdByLogin(const std::string & sessionId, const std::string & login) const;
  void deleteExpiredSessions() const;
  void getLoginBySessionId(const std::string & sessionId, std::string & login);
  ~AuthMng() override {}
};
//------------------------------------------------------------------------------
#endif // AUTHMNG_H
//------------------------------------------------------------------------------
