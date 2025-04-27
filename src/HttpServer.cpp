#include "HttpServer.h"



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

void fail(beast::error_code ec, char const * what)
{
  // ssl::error::stream_truncated, also known as an SSL "short read",
  // indicates the peer closed the connection without performing the
  // required closing handshake (for example, Google does this to
  // improve performance). Generally this can be a security issue,
  // but if your communication protocol is self-terminated (as
  // it is with both HTTP and WebSocket) then you may simply
  // ignore the lack of close_notify.
  //
  // https://github.com/boostorg/beast/issues/38
  //
  // https://security.stackexchange.com/questions/91435/how-to-handle-a-malicious-ssl-tls-shutdown
  //
  // When a short read would cut off the end of an HTTP message,
  // Beast returns the error beast::http::error::partial_message.
  // Therefore, if we see a short read here, it has occurred
  // after the message has been completed, so it is safe to ignore it.

  if(ec == net::ssl::error::stream_truncated)
    return;

  std::cerr << what << ": " << ec.message() << "\n";
}


json::value parse_json_body(const HttpRequest & req) {
    // проверка что метод POST
    if (req.method() != http::verb::post) {
        throw std::runtime_error("only POST method supported");
    }

    // проверка что Content-Type: application/json
    if (req.find(http::field::content_type) == req.end() ||
        req.at(http::field::content_type) != "application/json") {
        throw std::runtime_error("content-type must be application/json");
    }

    // пытаемся распарсить тело
    boost::system::error_code ec;
    auto parsed = json::parse(req.body(), ec);

    if (ec) {
        throw std::runtime_error("failed to parse json body");
    }

    return parsed;
}


http::message_generator sendReply(const HttpRequest & req, const std::string & msg, http::status status)
{
  http::response<http::string_body> res{status, req.version()};
  res.set(http::field::content_type, "text/html");
  res.keep_alive(req.keep_alive());
  res.body() = msg;
  res.prepare_payload();
  return res;
}
