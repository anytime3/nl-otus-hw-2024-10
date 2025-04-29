#include "HttpServer.h"
//------------------------------------------------------------------------------
namespace
{
const int l_sessionExpired = 30;
}
//------------------------------------------------------------------------------
session::session(tcp::socket&& socket, ssl::context & ctx, std::shared_ptr<std::string const> const & doc_root, loggerPtr & log, std::shared_ptr<RequestMng> & reqMng) :
  _stream(std::move(socket), ctx),
  _doc_root(doc_root),
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
  net::dispatch(_stream.get_executor(), beast::bind_front_handler(&session::on_run, shared_from_this()));
}
//------------------------------------------------------------------------------
void session::on_run()
{
  // Set the timeout.
  beast::get_lowest_layer(_stream).expires_after(std::chrono::seconds(30));

  // Perform the SSL handshake
  _stream.async_handshake(ssl::stream_base::server, beast::bind_front_handler(&session::on_handshake, shared_from_this()));
}
//------------------------------------------------------------------------------
void session::on_handshake(beast::error_code ec)
{
  if(ec)
    return reportFail(ec, "handshake");

  do_read();
}
//------------------------------------------------------------------------------
void session::do_read()
{
  // Make the request empty before reading,
  // otherwise the operation behavior is undefined.
  _req = {};

  beast::get_lowest_layer(_stream).expires_after(std::chrono::seconds(30));

  http::async_read(_stream, _buffer, _req, beast::bind_front_handler(&session::on_read, shared_from_this()));
}
//------------------------------------------------------------------------------
void session::on_read(beast::error_code ec, std::size_t bytes_transferred)
{
  boost::ignore_unused(bytes_transferred);

  // This means they closed the connection
  if(ec == http::error::end_of_stream)
    return do_close();

  if(ec)
    return reportFail(ec, "read");
  HttpFileResponse fileRes;
  HttpResponse res;
  _reqMng->onGetHttpRequest(_req, res, fileRes);
  if (fileRes.has_content_length())
    send_response(std::move(fileRes));
  else
    send_response(std::move(res));
}
//------------------------------------------------------------------------------
void session::send_response(http::message_generator &&msg)
{
  bool keep_alive = msg.keep_alive();

  // Write the response
  beast::async_write(_stream, std::move(msg), beast::bind_front_handler(&session::on_write, this->shared_from_this(), keep_alive));
}
//------------------------------------------------------------------------------
void session::on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred)
{
  boost::ignore_unused(bytes_transferred);

  if(ec)
    return reportFail(ec, "write");

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
  beast::get_lowest_layer(_stream).expires_after(std::chrono::seconds(30));

  // Perform the SSL shutdown
  _stream.async_shutdown(beast::bind_front_handler(&session::on_shutdown, shared_from_this()));
}
//------------------------------------------------------------------------------
void session::on_shutdown(beast::error_code ec)
{
  if(ec)
    return reportFail(ec, "shutdown");

  // At this point the connection is closed gracefully
}
//------------------------------------------------------------------------------
void session::reportFail(beast::error_code ec, const std::string & what)
{
  if (ec != net::ssl::error::stream_truncated)
    _log->error(SPDLOG_FMT_STRING("{}: {}"), what, ec.message());
}
//------------------------------------------------------------------------------
listener::listener(boost::asio::io_context & ioc, ssl::context & ctx, tcp::endpoint endpoint, const std::shared_ptr<const std::string> &doc_root, loggerPtr & log) :
  _io_context(ioc),
  _sslContext(ctx),
  _acceptor(net::make_strand(ioc)),
  _doc_root(doc_root),
  _log(log),
  _timer(ioc, l_sessionExpired)
{
  beast::error_code ec;

  // Open the acceptor
  _acceptor.open(endpoint.protocol(), ec);
  if(ec)
  {
    reportFail(ec, "open");
    return;
  }

  // Allow address reuse
  _acceptor.set_option(net::socket_base::reuse_address(true), ec);
  if(ec)
  {
    reportFail(ec, "set_option");
    return;
  }

  // Bind to the server address
  _acceptor.bind(endpoint, ec);
  if(ec)
  {
    reportFail(ec, "bind");
    return;
  }

  // Start listening for connections
  _acceptor.listen(net::socket_base::max_listen_connections, ec);
  if(ec)
  {
    reportFail(ec, "listen");
    return;
  }
  _reqMng = std::make_shared<RequestMng>(_doc_root, _log);
  _timerHandler = [&](beast::error_code ec)
                  {
                    if (ec.failed())
                      return;
                    _timer.expires_after(std::chrono::seconds(l_sessionExpired));
                    _reqMng->clearExpiredSessions();
                    _timer.async_wait(_timerHandler);
                  };
  _timer.async_wait(_timerHandler);
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
  _acceptor.async_accept(net::make_strand(_io_context), beast::bind_front_handler(&listener::on_accept, shared_from_this()));
}
//------------------------------------------------------------------------------
void listener::on_accept(beast::error_code ec, tcp::socket socket)
{
  if(ec)
  {
    reportFail(ec, "accept");
  }
  else
  {
    _log->info("Openned new session");
    std::make_shared<session>(std::move(socket), _sslContext, _doc_root, _log, _reqMng)->run();
  }

  do_accept();
}
//------------------------------------------------------------------------------
void listener::reportFail(beast::error_code ec, const std::string & what)
{
  if (ec != net::ssl::error::stream_truncated)
    _log->error(SPDLOG_FMT_STRING("{}: {}"), what, ec.message());
}
//------------------------------------------------------------------------------
