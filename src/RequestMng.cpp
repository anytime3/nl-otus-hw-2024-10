#include "RequestMng.h"
//------------------------------------------------------------------------------
namespace
{
const std::string l_sessionExpired = "30";
const std::string l_dbUser = "postgres";
const std::string l_dbPassword = "postgres";
}
//------------------------------------------------------------------------------
beast::string_view mime_type(beast::string_view path)
{
  using beast::iequals;
  auto const ext = [&path]
  {
    auto const pos = path.rfind(".");
    if(pos == beast::string_view::npos)
      return beast::string_view{};
    return path.substr(pos);
  }();
  if(iequals(ext, ".htm"))  return "text/html";
  if(iequals(ext, ".html")) return "text/html";
  if(iequals(ext, ".php"))  return "text/html";
  if(iequals(ext, ".css"))  return "text/css";
  if(iequals(ext, ".txt"))  return "text/plain";
  if(iequals(ext, ".js"))   return "application/javascript";
  if(iequals(ext, ".json")) return "application/json";
  if(iequals(ext, ".xml"))  return "application/xml";
  if(iequals(ext, ".swf"))  return "application/x-shockwave-flash";
  if(iequals(ext, ".flv"))  return "video/x-flv";
  if(iequals(ext, ".png"))  return "image/png";
  if(iequals(ext, ".jpe"))  return "image/jpeg";
  if(iequals(ext, ".jpeg")) return "image/jpeg";
  if(iequals(ext, ".jpg"))  return "image/jpeg";
  if(iequals(ext, ".gif"))  return "image/gif";
  if(iequals(ext, ".bmp"))  return "image/bmp";
  if(iequals(ext, ".ico"))  return "image/vnd.microsoft.icon";
  if(iequals(ext, ".tiff")) return "image/tiff";
  if(iequals(ext, ".tif"))  return "image/tiff";
  if(iequals(ext, ".svg"))  return "image/svg+xml";
  if(iequals(ext, ".svgz")) return "image/svg+xml";
  return "application/text";
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
  _authMng = std::make_unique<AuthMng>(l_dbUser, l_dbPassword, _log);
  _contentMng = std::make_unique<ContentMng>(l_dbUser, l_dbPassword, _log);
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

  if (req.target() == "/addpost")
  {
    onAddPost(req, res);
    return;
  }

  generateResponse(req, "Unknown HTTP-method", res, http::status::bad_request);
}
//------------------------------------------------------------------------------
void RequestMng::onLogin(const HttpRequest &req, HttpResponse &res) const
{
  try
  {
    std::string sessionId;
    getSessionIdFromReq(req, sessionId);
    if (sessionId.empty() || sessionId.empty() == false && _authMng->checkSession(sessionId) == false)
    {
      json::value jsonValue;
      parseJsonBody(req, jsonValue);
      std::string login = json::value_to<std::string>(jsonValue.at("login"));
      std::string password = json::value_to<std::string>(jsonValue.at("password"));

      if (_authMng->checkLogin(login) == false)
        throw std::runtime_error("User not found");

      generateSessionId(sessionId);
      _authMng->setSessionIdByLogin(sessionId, login);
      res.set(http::field::set_cookie, sessionId);
    }

    json::object user;
    std::string username;
    _authMng->getLoginBySessionId(sessionId, username);
    user["username"] = username;

    std::string jsonStr = json::serialize(user);
    generateResponseWithContentType(req, jsonStr, "application/json", res, http::status::ok);
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
    std::string sessionId;
    getSessionIdFromReq(req, sessionId);

    if (sessionId.empty())
    {
      json::value jsonValue;
      parseJsonBody(req, jsonValue);
      std::string login = json::value_to<std::string>(jsonValue.at("login"));

      if (_authMng->checkLogin(login) == false)
        throw std::runtime_error("User not found");

      _authMng->logoutByLogin(login);
    }
    else
      _authMng->logoutBySessionId(sessionId);
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

    std::string sessionId;
    generateSessionId(sessionId);
    _authMng->setSessionIdByLogin(sessionId, login);
    res.set(http::field::set_cookie, sessionId);

    json::object user;
    std::string username;
    _authMng->getLoginBySessionId(sessionId, username);
    user["username"] = username;

    std::string jsonStr = json::serialize(user);
    generateResponseWithContentType(req, jsonStr, "application/json", res, http::status::ok);
  } catch (const std::exception & err)
  {
    generateResponse(req, std::string("User didn't registered: " + std::string(err.what())), res, http::status::not_found);
    return;
  }
}
//------------------------------------------------------------------------------
void RequestMng::onAddPost(const HttpRequest &req, HttpResponse &res) const
{
  try
  {
    std::string sessionId;
    getSessionIdFromReq(req, sessionId);
    if (sessionId.empty())
      throw std::runtime_error("Unknown user");

    if(_authMng->checkSession(sessionId) == false)
      throw std::runtime_error("Session expired");

    json::value jsonValue;
    parseJsonBody(req, jsonValue);
    std::string theme = json::value_to<std::string>(jsonValue.at("theme"));
    std::string data = json::value_to<std::string>(jsonValue.at("data"));
    std::string user;

    _authMng->getLoginBySessionId(sessionId, user);
    _contentMng->addPost(user, theme, data);
    generateResponse(req, "Post added successful", res, http::status::ok);
  } catch (const std::exception & err)
  {
    generateResponse(req, std::string("Did not add post: " + std::string(err.what())), res, http::status::bad_request);
    return;
  }
}
//------------------------------------------------------------------------------
void RequestMng::generateResponseWithContentType(const HttpRequest & req, const std::string & bodyMsg, const std::string & contentType, HttpResponse & res, http::status status) const
{
  res.version(req.version());
  res.result(status);
  res.set(http::field::content_type, contentType);
  res.keep_alive(req.keep_alive());
  res.body() = bodyMsg;
  res.prepare_payload();
}
//------------------------------------------------------------------------------
void RequestMng::onGetRequest(const HttpRequest & req, HttpResponse & res, HttpFileResponse & fileRes) const
{
  if (req.target().starts_with("/getposts"))
  {
    onGetPosts(req, res);
    return;
  }

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
void RequestMng::onGetPosts(const HttpRequest & req, HttpResponse & res) const
{
  try
  {
    std::string sessionId;
    getSessionIdFromReq(req, sessionId);
    if (sessionId.empty())
      throw std::runtime_error("Unknown user");

    if(_authMng->checkSession(sessionId) == false)
      throw std::runtime_error("Session expired");

    json::object jsonPosts;
    json::array posts;
    _contentMng->getAllPosts(posts);
    jsonPosts["posts"] = posts;
    std::string jsonStr = json::serialize(jsonPosts);
    generateResponseWithContentType(req, jsonStr, "application/json", res, http::status::ok);
  } catch (const std::exception & err)
  {
    generateResponse(req, std::string(": " + std::string(err.what())), res, http::status::not_found);
    return;
  }
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
  sessionId = it->value();
}
//------------------------------------------------------------------------------
