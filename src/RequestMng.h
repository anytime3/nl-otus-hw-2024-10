#ifndef REQUESTMNG_H
#define REQUESTMNG_H
//------------------------------------------------------------------------------
#include "AuthMng.h"
#include "ContentMng.h"
#include <boost/beast/http.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
//------------------------------------------------------------------------------
namespace http = boost::beast::http;
namespace json = boost::json;
namespace beast = boost::beast;
using HttpRequest = http::request<http::string_body>;
using HttpResponse = http::response<http::string_body>;
using HttpFileResponse = http::response<http::file_body>;
//------------------------------------------------------------------------------
beast::string_view mime_type(beast::string_view path);
//------------------------------------------------------------------------------
void parseJsonBody(const HttpRequest & req, json::value & jsonValue);
//------------------------------------------------------------------------------
void generateSessionId(std::string & session_id);
//------------------------------------------------------------------------------
void getSessionIdFromReq(const HttpRequest & req, std::string & sessionId);
//------------------------------------------------------------------------------
beast::string_view mime_type(beast::string_view path);
//------------------------------------------------------------------------------
class RequestMng
{
private:
  loggerPtr                          _log;
  std::shared_ptr<std::string const> _doc_root;
  std::unique_ptr<AuthMng>           _authMng;
  std::unique_ptr<ContentMng>        _contentMng;
public:
  RequestMng() = default;
  RequestMng(std::shared_ptr<std::string const> & doc_root, loggerPtr & log);
  void onGetHttpRequest(const HttpRequest & req, HttpResponse & res, HttpFileResponse & fileRes) const;

  void onGetRequest(const HttpRequest & req, HttpResponse & res, HttpFileResponse & fileRes) const;
  void onGetHtml(const HttpRequest & req, HttpResponse & res, HttpFileResponse & fileRes) const;
  void onGetPosts(const HttpRequest & req, HttpResponse & res) const;

  void onPostRequest(const HttpRequest & req, HttpResponse & res) const;
  void onLogin(const HttpRequest & req, HttpResponse & res) const;
  void onLogout(const HttpRequest & req, HttpResponse & res) const;
  void onRegister(const HttpRequest & req, HttpResponse & res) const;
  void onAddPost(const HttpRequest & req, HttpResponse & res) const;

  void generateResponse(const HttpRequest & req, const std::string & bodyMsg, HttpResponse & res, http::status status) const;
  void generateResponseWithContentType(const HttpRequest & req, const std::string & bodyMsg, const std::string & contentType, HttpResponse & res, http::status status) const;
  void clearExpiredSessions() const;
  ~RequestMng() = default;
};
//------------------------------------------------------------------------------
#endif // REQUESTMNG_H
//------------------------------------------------------------------------------
