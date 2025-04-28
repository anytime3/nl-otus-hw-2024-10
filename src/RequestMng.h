#ifndef REQUESTMNG_H
#define REQUESTMNG_H

#include "AuthMng.h"
#include "ContentMng.h"
#include <boost/beast/http.hpp>
#include <boost/json.hpp>

namespace http = boost::beast::http;
namespace json = boost::json;
using HttpRequest = http::request<http::string_body>;
using HttpResponse = http::response<http::string_body>;

void parseJsonBody(const HttpRequest & req, json::value & jsonValue);

class RequestMng
{
private:
  loggerPtr & _log;
  std::unique_ptr<AuthMng> _authMng;
  // std::unique_ptr _contentMng;
public:
  RequestMng(loggerPtr & log);
  void onGetHttpRequest(const HttpRequest & req, HttpResponse & res) const;

  void onGetRequest(const HttpRequest & req, HttpResponse & res) const;
  void onGetHtml(const HttpRequest & req, HttpResponse & res) const;

  void onPostRequest(const HttpRequest & req, HttpResponse & res) const;
  void onLogin(const HttpRequest & req, HttpResponse & res) const;
  void onRegister(const HttpRequest & req, HttpResponse & res) const;

  void generateResponse(const HttpRequest & req, const std::string & bodyMsg, HttpResponse & res, http::status status) const;
};

#endif // REQUESTMNG_H
