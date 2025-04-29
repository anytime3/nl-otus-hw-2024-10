#ifndef AUTHMNG_H
#define AUTHMNG_H

#include "DbMng.h"

class AuthMng : public DbMng
{
private:
  std::string _minExpires;
public:
  AuthMng(const std::string & user, const std::string & password, loggerPtr & log);
  bool checkLogin(const std::string & login) const;
  bool checkSession(const std::string & session_id) const;
  bool logoutBySessionId(const std::string & session_id);
  bool logoutByLogin(const std::string & login);
  bool registerUser(const std::string & login, const std::string & password) const;
  bool setSessionIdByLogin(const std::string & session_id, const std::string & login) const;
  void deleteExpiredSessions() const;
  ~AuthMng() override {}
};

#endif // AUTHMNG_H
