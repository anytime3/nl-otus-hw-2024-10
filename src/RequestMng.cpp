#include "RequestMng.h"


RequestMng::RequestMng(loggerPtr & log) :
  _log(log)
{
  _authMng = std::make_unique<AuthMng>("postgres", "postgres", _log);
}

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

  generateResponse(req, "Unknown HTTP-method", res, http::status::bad_request);
}

void RequestMng::onLogin(const HttpRequest &req, HttpResponse &res) const
{
  try
  {
    json::value jsonValue;
    parseJsonBody(req, jsonValue);
    std::string login = json::value_to<std::string>(jsonValue.at("login"));
    std::string password = json::value_to<std::string>(jsonValue.at("password"));

    if (_authMng->checkLoginAndPassword(login, password) == false)
      throw std::runtime_error("User not found");

    // сгенерировать session_id, присвоить поле Set-Cookieыё
    generateResponse(req, "Login successful", res, http::status::ok);
  } catch (const std::exception & err)
  {
    generateResponse(req, "User not found", res, http::status::not_found);
    return;
  }
}

void RequestMng::onGetHttpRequest(const HttpRequest & req, HttpResponse & res) const
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
    onGetRequest(req, res);
    break;
  case http::verb::post:
    onPostRequest(req, res);
    break;
  default:
    generateResponse(req, "Unknown HTTP-method", res, http::status::bad_request);
    break;
  }
}

void RequestMng::onGetRequest(const HttpRequest &req, HttpResponse &res) const
{
  onGetHtml(req, res);
}

void RequestMng::onGetHtml(const HttpRequest &req, HttpResponse &res) const
{
  /*// Build the path to the requested file
  std::string path = path_cat(*doc_root_, req_.target());
  if(req_.target().back() == '/')
    path.append("index.html");

  // Attempt to open the file
  beast::error_code ec;
  http::file_body::value_type body;
  body.open(path.c_str(), beast::file_mode::scan, ec);

  // Handle the case where the file doesn't exist
  if(ec == beast::errc::no_such_file_or_directory)
    send_response(sendReply(req_, std::string("The resource '" + std::string(req_.target()) + "' was not found."), http::status::not_found));


  // Handle an unknown error
  if(ec)
    send_response(sendReply(req_, std::string("An error occurred: '" + ec.message() + "'"), http::status::internal_server_error));

  // Cache the size since we need it after the move
  auto const size = body.size();

  // // Respond to HEAD request
  // if(req.method() == http::verb::head)
  // {
  //   http::response<http::empty_body> res{http::status::ok, req.version()};
  //   res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  //   res.set(http::field::content_type, mime_type(path));
  //   res.content_length(size);
  //   res.keep_alive(req.keep_alive());
  //   return res;
  // }

  // Respond to GET request
  http::response<http::file_body> res{std::piecewise_construct,
        std::make_tuple(std::move(body)),
        std::make_tuple(http::status::ok,
                        req_.version())};
  res.set(http::field::server, BOOST_BEAST_VERSION_STRING);
  res.set(http::field::content_type, mime_type(path));
  res.content_length(size);
  res.keep_alive(req_.keep_alive());

  send_response(std::move(res));*/
}

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
void parseJsonBody(const HttpRequest & req, json::value & jsonValue)
{
  // проверка что Content-Type: application/json
  if (req.find(http::field::content_type) == req.end() ||
      req.at(http::field::content_type) != "application/json")
  {
    throw std::runtime_error("content-type must be application/json");
  }

  // пытаемся распарсить тело
  boost::system::error_code ec;
  jsonValue = json::parse(req.body(), ec);

  if (ec)
    throw std::runtime_error("failed to parse json body");
}
