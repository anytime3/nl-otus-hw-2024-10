#include "HttpServer.h"
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
//------------------------------------------------------------------------------
session::session(tcp::socket&& socket, ssl::context & ctx, std::shared_ptr<std::string const> const & doc_root, loggerPtr & log, std::shared_ptr<RequestMng> & reqMng) :
  stream_(std::move(socket), ctx),
  doc_root_(doc_root),
  _log(log),
  _reqMng(reqMng)
{
}
//------------------------------------------------------------------------------
void session::run()
{
  // We need to be executing within a strand to perform async operations
  // on the I/O objects in this session. Although not strictly necessary
  // for single-threaded contexts, this example code is written to be
  // thread-safe by default.
  net::dispatch(stream_.get_executor(), beast::bind_front_handler(&session::on_run, shared_from_this()));
}
//------------------------------------------------------------------------------
void session::on_run()
{
  // Set the timeout.
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

  // Perform the SSL handshake
  stream_.async_handshake(ssl::stream_base::server, beast::bind_front_handler(&session::on_handshake, shared_from_this()));
}
//------------------------------------------------------------------------------
void session::on_handshake(beast::error_code ec)
{
  if(ec)
    return fail(ec, "handshake");

  do_read();
}
//------------------------------------------------------------------------------
void session::do_read()
{
  // Make the request empty before reading,
  // otherwise the operation behavior is undefined.
  req_ = {};

  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

  http::async_read(stream_, buffer_, req_, beast::bind_front_handler(&session::on_read, shared_from_this()));
}
//------------------------------------------------------------------------------
void session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
  boost::ignore_unused(bytes_transferred);

  // This means they closed the connection
  if(ec == http::error::end_of_stream)
    return do_close();

  if(ec)
    return fail(ec, "read");

  HttpResponse res;
  _reqMng->onGetHttpRequest(req_, res);
  send_response(std::move(res));
}
//------------------------------------------------------------------------------
void session::send_response(http::message_generator &&msg)
{
  bool keep_alive = msg.keep_alive();

  // Write the response
  beast::async_write(stream_, std::move(msg), beast::bind_front_handler(&session::on_write, this->shared_from_this(), keep_alive));
}
//------------------------------------------------------------------------------
void session::on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred)
{
  boost::ignore_unused(bytes_transferred);

  if(ec)
    return fail(ec, "write");

  if(! keep_alive)
  {
    // This means we should close the connection, usually because
    // the response indicated the "Connection: close" semantic.
    return do_close();
  }

  // Read another request
  do_read();
}
//------------------------------------------------------------------------------
void session::do_close()
{
  // Set the timeout.
  beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

  // Perform the SSL shutdown
  stream_.async_shutdown(beast::bind_front_handler(&session::on_shutdown, shared_from_this()));
}
//------------------------------------------------------------------------------
void session::on_shutdown(beast::error_code ec)
{
  if(ec)
    return fail(ec, "shutdown");

  // At this point the connection is closed gracefully
}
//------------------------------------------------------------------------------
listener::listener(boost::asio::io_context &ioc, ssl::context &ctx, tcp::endpoint endpoint, const std::shared_ptr<const std::string> &doc_root, loggerPtr & log) :
  ioc_(ioc),
  ctx_(ctx),
  acceptor_(net::make_strand(ioc)),
  doc_root_(doc_root),
  _log(log)
{
  beast::error_code ec;

  // Open the acceptor
  acceptor_.open(endpoint.protocol(), ec);
  if(ec)
  {
    fail(ec, "open");
    return;
  }

  // Allow address reuse
  acceptor_.set_option(net::socket_base::reuse_address(true), ec);
  if(ec)
  {
    fail(ec, "set_option");
    return;
  }

  // Bind to the server address
  acceptor_.bind(endpoint, ec);
  if(ec)
  {
    fail(ec, "bind");
    return;
  }

  // Start listening for connections
  acceptor_.listen(net::socket_base::max_listen_connections, ec);
  if(ec)
  {
    fail(ec, "listen");
    return;
  }
  _reqMng = std::make_shared<RequestMng>(_log);
}
//------------------------------------------------------------------------------
void listener::run()
{
  do_accept();
}
//------------------------------------------------------------------------------
void listener::do_accept()
{
  // The new connection gets its own strand
  acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}
//------------------------------------------------------------------------------
void listener::on_accept(beast::error_code ec, tcp::socket socket)
{
  if(ec)
  {
    fail(ec, "accept");
  }
  else
  {
    std::make_shared<session>(std::move(socket), ctx_, doc_root_, _log, _reqMng)->run();
  }

  do_accept();
}
//------------------------------------------------------------------------------
