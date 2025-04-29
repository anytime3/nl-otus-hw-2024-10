#include "RequestMng.h"
//------------------------------------------------------------------------------
namespace
{
std::string l_sessionExpired = "30";
}
//------------------------------------------------------------------------------
std::string path_cat(beast::string_view base, beast::string_view path)
{
  if(base.empty())
    return std::string(path);
  std::string result(base);

  char constexpr path_separator = '/';
  if(result.back() == path_separator)
    result.resize(result.size() - 1);
  result.append(path.data(), path.size());

  return result;
}
//------------------------------------------------------------------------------
RequestMng::RequestMng(std::shared_ptr<std::string const> & doc_root, loggerPtr & log) :
  _doc_root(doc_root),
  _log(log)
{
  _authMng = std::make_unique<AuthMng>("postgres", "postgres", _log);
}
//------------------------------------------------------------------------------
void RequestMng::onGetHttpRequest(const HttpRequest & req, HttpResponse & res, HttpFileResponse & fileRes) const
{
  // Request path must be absolute and not contain "..".
  if(req.target().empty() || req.target()[0] != '/' || req.target().find("..") != boost::beast::string_view::npos)
  {
    generateResponse(req, "Illegal request-target", res, http::status::bad_request);
    return;
  }

  switch (req.method())
  {
  case http::verb::get:
    onGetRequest(req, res, fileRes);
    break;
  case http::verb::post:
    onPostRequest(req, res);
    break;
  default:
    generateResponse(req, "Unknown HTTP-method", res, http::status::bad_request);
    break;
  }
}
//------------------------------------------------------------------------------
void RequestMng::onPostRequest(const HttpRequest &req, HttpResponse &res) const
{
  if (req.target() == "/login")
  {
    onLogin(req, res);
    return;
  }

  if (req.target() == "/register")
  {
    onRegister(req, res);
    return;
  }

  if (req.target() == "/logout")
  {
    onLogout(req, res);
    return;
  }

  generateResponse(req, "Unknown HTTP-method", res, http::status::bad_request);
}
//------------------------------------------------------------------------------
void RequestMng::onLogin(const HttpRequest &req, HttpResponse &res) const
{
  try
  {
    std::string session_id;
    getSessionIdFromReq(req, session_id);
    if (session_id.empty())
    {
      json::value jsonValue;
      parseJsonBody(req, jsonValue);
      std::string login = json::value_to<std::string>(jsonValue.at("login"));
      std::string password = json::value_to<std::string>(jsonValue.at("password"));

      if (_authMng->checkLogin(login) == false)
        throw std::runtime_error("User not found");

      generateSessionId(session_id);
      _authMng->setSessionIdByLogin(session_id, login);
      res.set(http::field::set_cookie, session_id);
    }
    generateResponse(req, "Login successful", res, http::status::ok);
  } catch (const std::exception & err)
  {
    generateResponse(req, "User not found", res, http::status::not_found);
    return;
  }
}
//------------------------------------------------------------------------------
void RequestMng::onLogout(const HttpRequest & req, HttpResponse & res) const
{
  try
  {
    std::string session_id;
    getSessionIdFromReq(req, session_id);
    if (session_id.empty())
    {
      json::value jsonValue;
      parseJsonBody(req, jsonValue);
      std::string login = json::value_to<std::string>(jsonValue.at("login"));

      if (_authMng->checkLogin(login) == false)
        throw std::runtime_error("User not found");

      _authMng->logoutByLogin(login);
    }
    else
      _authMng->logoutBySessionId(session_id);
    generateResponse(req, "Logout successful", res, http::status::ok);
  } catch (const std::exception & err)
  {
    generateResponse(req, "User not found", res, http::status::not_found);
    return;
  }
}
//------------------------------------------------------------------------------
void RequestMng::onRegister(const HttpRequest & req, HttpResponse & res) const
{
  try
  {
    json::value jsonValue;
    parseJsonBody(req, jsonValue);
    std::string login = json::value_to<std::string>(jsonValue.at("login"));
    std::string password = json::value_to<std::string>(jsonValue.at("password"));

    if (_authMng->registerUser(login, password) == false)
      throw std::runtime_error("User is already registered");

    std::string session_id;
    generateSessionId(session_id);
    _authMng->setSessionIdByLogin(session_id, login);
    res.set(http::field::set_cookie, session_id);
    generateResponse(req, "Registration successful", res, http::status::ok);
  } catch (const std::exception & err)
  {
    generateResponse(req, std::string("User didn't registered: " + std::string(err.what())), res, http::status::not_found);
    return;
  }
}
//------------------------------------------------------------------------------
void RequestMng::onGetRequest(const HttpRequest &req, HttpResponse &res, HttpFileResponse & fileRes) const
{
  onGetHtml(req, res, fileRes);
}
//------------------------------------------------------------------------------
void RequestMng::onGetHtml(const HttpRequest & req, HttpResponse & res, HttpFileResponse & fileRes) const
{
  // Build the path to the requested file
  std::string path = path_cat(*_doc_root, req.target());
  if(req.target().back() == '/')
    path.append("index.html");

  // Attempt to open the file
  beast::error_code ec;
  http::file_body::value_type body;
  body.open(path.c_str(), beast::file_mode::scan, ec);

  // Handle the case where the file doesn't exist
  if(ec == beast::errc::no_such_file_or_directory)
  {
    generateResponse(req, std::string("The resource '" + std::string(req.target()) + "' was not found."), res, http::status::not_found);
    return;
  }

  // Handle an unknown error
  if(ec)
  {
    generateResponse(req, std::string("An error occurred: '" + ec.message() + "'"), res, http::status::internal_server_error);
    return;
  }
  // Cache the size since we need it after the move
  auto const size = body.size();

  // Respond to GET request
  fileRes = HttpFileResponse {std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok,
                        req.version())};
  fileRes.set(http::field::content_type, mime_type(path));
  fileRes.content_length(size);
  fileRes.keep_alive(req.keep_alive());
}
//------------------------------------------------------------------------------
void RequestMng::generateResponse(const HttpRequest & req, const std::string & bodyMsg, HttpResponse & res, http::status status) const
{
  res.version(req.version());
  res.result(status);
  res.set(http::field::content_type, "text/html");
  res.keep_alive(req.keep_alive());
  res.body() = bodyMsg;
  res.prepare_payload();
}
//------------------------------------------------------------------------------
void RequestMng::clearExpiredSessions() const
{
  _authMng->deleteExpiredSessions();
}
//------------------------------------------------------------------------------
void parseJsonBody(const HttpRequest & req, json::value & jsonValue)
{
  // проверка что Content-Type: application/json
  if (req.find(http::field::content_type) == req.end() ||
      req.at(http::field::content_type) != "application/json")
  {
    throw std::runtime_error("content-type must be application/json");
  }

  boost::system::error_code ec;
  jsonValue = json::parse(req.body(), ec);

  if (ec)
    throw std::runtime_error("failed to parse json body");
}
//------------------------------------------------------------------------------
void generateSessionId(std::string & session_id)
{
  boost::uuids::random_generator generator;
  boost::uuids::uuid uuid = generator();
  session_id = boost::uuids::to_string(uuid);
}
//------------------------------------------------------------------------------
void getSessionIdFromReq(const HttpRequest & req, std::string & sessionId)
{
  auto it = req.find(http::field::cookie);
  if (it == req.end()) {
      return;
  }

  std::string cookies = it->value();
  std::string key = "session_id=";

  size_t start = cookies.find(key);
  if (start == std::string::npos) {
      return;
  }

  start += key.length();
  size_t end = cookies.find(';', start);
  sessionId = cookies.substr(start, end - start);
}
//------------------------------------------------------------------------------
