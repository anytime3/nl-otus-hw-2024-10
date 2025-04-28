#ifndef HTTPSERVER_H
#define HTTPSERVER_H
//------------------------------------------------------------------------------
#include <boost/beast/core.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <memory>
#include <string>
#include "RequestMng.h"
//------------------------------------------------------------------------------
namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
//------------------------------------------------------------------------------
beast::string_view mime_type(beast::string_view path);
//------------------------------------------------------------------------------
std::string path_cat(beast::string_view base, beast::string_view path);
//------------------------------------------------------------------------------
void fail(beast::error_code ec, char const * what);
//------------------------------------------------------------------------------
class session : public std::enable_shared_from_this<session>
{
  ssl::stream<beast::tcp_stream> stream_;
  beast::flat_buffer buffer_;
  std::shared_ptr<std::string const> doc_root_;
  http::request<http::string_body> req_;
  std::shared_ptr<RequestMng> & _reqMng;
  loggerPtr & _log;
public:
  session(tcp::socket&& socket, ssl::context & ctx, std::shared_ptr<std::string const> const & doc_root, loggerPtr & log, std::shared_ptr<RequestMng> & reqMng);
  void run();
  void on_run();
  void on_handshake(beast::error_code ec);
  void do_read();
  void on_read(beast::error_code ec, std::size_t bytes_transferred);
  void send_response(http::message_generator&& msg);
  void on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred);
  void do_close();
  void on_shutdown(beast::error_code ec);
};
//------------------------------------------------------------------------------
class listener : public std::enable_shared_from_this<listener>
{
  net::io_context & ioc_;
  ssl::context & ctx_;
  tcp::acceptor acceptor_;
  std::shared_ptr<std::string const> doc_root_;
  loggerPtr & _log;
  std::shared_ptr<RequestMng> _reqMng;
public:
  listener(net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, std::shared_ptr<std::string const> const & doc_root, loggerPtr & log);
  void run();
private:
  void do_accept();
  void on_accept(beast::error_code ec, tcp::socket socket);
};
//------------------------------------------------------------------------------
#endif // HTTPSERVER_H
