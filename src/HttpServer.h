#ifndef HTTPSERVER_H
#define HTTPSERVER_H


#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/config.hpp>
#include <boost/json.hpp>
#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <thread>

#include "AuthMng.h"
#include "ContentMng.h"

namespace beast = boost::beast;         // from <boost/beast.hpp>
namespace http = beast::http;           // from <boost/beast/http.hpp>
namespace net = boost::asio;            // from <boost/asio.hpp>
namespace ssl = boost::asio::ssl;       // from <boost/asio/ssl.hpp>
namespace json = boost::json;
using tcp = boost::asio::ip::tcp;       // from <boost/asio/ip/tcp.hpp>
using HttpRequest = http::request<http::string_body>;
using HttpResponse = http::response<http::string_body>;


// Return a reasonable mime type based on the extension of a file.
beast::string_view mime_type(beast::string_view path);

// Append an HTTP rel-path to a local filesystem path.
// The returned path is normalized for the platform.
std::string path_cat(beast::string_view base, beast::string_view path);

// Return a response for the given request.
//
// The concrete type of the response message (which depends on the
// request), is type-erased in message_generator.




http::message_generator sendReply(const HttpRequest & req, const std::string & msg, http::status status);


json::value parse_json_body(const HttpRequest & req);

//------------------------------------------------------------------------------

// Report a failure
void fail(beast::error_code ec, char const * what);

// Handles an HTTP server connection.
// This uses the Curiously Recurring Template Pattern so that
// the same code works with both SSL streams and regular sockets.
template<class Derived>
class session
{
  // Access the derived class, this is part of
  // the Curiously Recurring Template Pattern idiom.
  Derived & derived()
  {
    return static_cast<Derived&>(*this);
  }

  std::shared_ptr<std::string const> doc_root_;
  http::request<http::string_body> req_;

protected:
  beast::flat_buffer buffer_;

public:
  // Take ownership of the buffer
  session(beast::flat_buffer buffer, std::shared_ptr<std::string const> const& doc_root) :
    doc_root_(doc_root),
    buffer_(std::move(buffer))
  {
  }

  void onGetRequest()
  {
    // Build the path to the requested file
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

    send_response(std::move(res));
  }

  void onPostRequest()
  {
    if (req_.target() == "/login")
    {
      onLoginRequest();
      return;
    }

    if (req_.target() == "/register")
    {
      onRegisterRequest();
      return;
    }

    send_response(sendReply(req_, "Unknown HTTP-method", http::status::bad_request));
  }

  void do_read()
  {
    // Set the timeout.
    beast::get_lowest_layer(derived().stream()).expires_after(std::chrono::seconds(30));

    // Read a request
    http::async_read(derived().stream(), buffer_, req_, beast::bind_front_handler(&session::on_read, derived().shared_from_this()));
  }


  void on_read(beast::error_code ec, std::size_t bytes_transferred)
  {
    boost::ignore_unused(bytes_transferred);

    // This means they closed the connection
    if(ec == http::error::end_of_stream)
      return derived().do_eof();

    if(ec)
      return fail(ec, "read");

    // Request path must be absolute and not contain "..".
    if(req_.target().empty() || req_.target()[0] != '/' || req_.target().find("..") != beast::string_view::npos)
    {
      send_response(sendReply(req_, "Illegal request-target", http::status::bad_request));
      return;
    }

    switch (req_.method())
    {
      case http::verb::get:
        onGetRequest();
        break;
      case http::verb::post:
        onPostRequest();
        break;
      default:
        send_response(sendReply(req_, "Unknown HTTP-method", http::status::bad_request));
        break;
    }
  }

  void send_response(http::message_generator&& msg)
  {
    bool keep_alive = msg.keep_alive();

    // Write the response
    beast::async_write(derived().stream(), std::move(msg), beast::bind_front_handler(&session::on_write, derived().shared_from_this(), keep_alive));
  }

  void on_write(bool keep_alive, beast::error_code ec, std::size_t bytes_transferred)
  {
    boost::ignore_unused(bytes_transferred);

    if(ec)
      return fail(ec, "write");

    if(! keep_alive)
    {
      // This means we should close the connection, usually because
      // the response indicated the "Connection: close" semantic.
      return derived().do_eof();
    }

    // Read another request
    do_read();
  }



  void onLoginRequest()
  {
    try {
      auto body = parse_json_body(req_);

      std::string login = json::value_to<std::string>(body.at("login"));
      std::string password = json::value_to<std::string>(body.at("password"));

      HttpResponse res;
      res.set(http::field::set_cookie, "session_id=");
    } catch (const std::exception & err) {
      send_response(sendReply(req_, "User not found", http::status::not_found));
      return;
    }
  }

  void onRegisterRequest()
  {

  }
};

// Handles a plain HTTP connection
class plain_session : public session<plain_session>, public std::enable_shared_from_this<plain_session>
{
  beast::tcp_stream stream_;

public:
  // Create the session
  plain_session(tcp::socket&& socket, beast::flat_buffer buffer, std::shared_ptr<std::string const> const& doc_root) :
    session<plain_session>(std::move(buffer), doc_root),
    stream_(std::move(socket))
  {
  }

  // Called by the base class
  beast::tcp_stream & stream()
  {
    return stream_;
  }

  // Start the asynchronous operation
  void
  run()
  {
      // We need to be executing within a strand to perform async operations
      // on the I/O objects in this session. Although not strictly necessary
      // for single-threaded contexts, this example code is written to be
      // thread-safe by default.
      net::dispatch(stream_.get_executor(), beast::bind_front_handler(&session::do_read, shared_from_this()));
  }

  void
  do_eof()
  {
      // Send a TCP shutdown
    beast::error_code ec;
    stream_.socket().shutdown(tcp::socket::shutdown_send, ec);

      // At this point the connection is closed gracefully
  }
};

// Handles an SSL HTTP connection
class ssl_session : public session<ssl_session>, public std::enable_shared_from_this<ssl_session>
{
  ssl::stream<beast::tcp_stream> stream_;

public:
  // Create the session
  ssl_session(tcp::socket&& socket, ssl::context& ctx, beast::flat_buffer buffer, std::shared_ptr<std::string const> const& doc_root) :
    session<ssl_session>(std::move(buffer), doc_root),
    stream_(std::move(socket), ctx)
  {
  }

  // Called by the base class
  ssl::stream<beast::tcp_stream> & stream()
  {
    return stream_;
  }

  // Start the asynchronous operation
  void run()
  {
    auto self = shared_from_this();
    // We need to be executing within a strand to perform async operations
    // on the I/O objects in this session.
    net::dispatch(stream_.get_executor(), [self]() {
      // Set the timeout.
      beast::get_lowest_layer(self->stream_).expires_after(std::chrono::seconds(30));

      // Perform the SSL handshake
      // Note, this is the buffered version of the handshake.
      self->stream_.async_handshake(ssl::stream_base::server, self->buffer_.data(), beast::bind_front_handler(&ssl_session::on_handshake,self));
    });
  }

  void on_handshake(
    beast::error_code ec,
    std::size_t bytes_used)
  {
    if(ec)
      return fail(ec, "handshake");

    // Consume the portion of the buffer used by the handshake
    buffer_.consume(bytes_used);

    do_read();
  }

  void do_eof()
  {
    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Perform the SSL shutdown
    stream_.async_shutdown(beast::bind_front_handler(&ssl_session::on_shutdown, shared_from_this()));
  }

  void on_shutdown(beast::error_code ec)
  {
    if(ec)
      return fail(ec, "shutdown");

    // At this point the connection is closed gracefully
  }
};

//------------------------------------------------------------------------------

// Detects SSL handshakes
class detect_session : public std::enable_shared_from_this<detect_session>
{
  beast::tcp_stream stream_;
  ssl::context& ctx_;
  std::shared_ptr<std::string const> doc_root_;
  beast::flat_buffer buffer_;

public:
  detect_session(tcp::socket&& socket, ssl::context& ctx, std::shared_ptr<std::string const> const& doc_root) :
    stream_(std::move(socket)),
    ctx_(ctx),
    doc_root_(doc_root)
  {
  }

  // Launch the detector
  void run()
  {
    // Set the timeout.
    beast::get_lowest_layer(stream_).expires_after(std::chrono::seconds(30));

    // Detect a TLS handshake
    async_detect_ssl(stream_, buffer_, beast::bind_front_handler(&detect_session::on_detect, shared_from_this()));
  }

  void on_detect(beast::error_code ec, bool result)
  {
    if(ec)
      return fail(ec, "detect");

    if(result)
    {
      // Launch SSL session
      std::make_shared<ssl_session>(stream_.release_socket(), ctx_, std::move(buffer_), doc_root_)->run();
      return;
    }
    // std::cout << "Warning! launching non-ssl session!!!\n";
    // Launch plain session
    //std::make_shared<plain_session>(stream_.release_socket(), std::move(buffer_), doc_root_)->run();
  }
};

// Accepts incoming connections and launches the sessions
class listener : public std::enable_shared_from_this<listener>
{
  net::io_context& ioc_;
  ssl::context& ctx_;
  tcp::acceptor acceptor_;
  std::shared_ptr<std::string const> doc_root_;

public:
  listener(net::io_context& ioc, ssl::context& ctx, tcp::endpoint endpoint, std::shared_ptr<std::string const> const & doc_root):
    ioc_(ioc),
    ctx_(ctx),
    acceptor_(net::make_strand(ioc)),
    doc_root_(doc_root)
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
  }

  // Start accepting incoming connections
  void run()
  {
    do_accept();
  }

private:
  void do_accept()
  {
    // The new connection gets its own strand
    acceptor_.async_accept(net::make_strand(ioc_), beast::bind_front_handler(&listener::on_accept, shared_from_this()));
  }

  void on_accept(beast::error_code ec, tcp::socket socket)
  {
    if(ec)
    {
      fail(ec, "accept");
    }
    else
    {
      // Create the detector session and run it
      std::make_shared<detect_session>(std::move(socket), ctx_, doc_root_)->run();
    }

    // Accept another connection
    do_accept();
  }
};
//------------------------------------------------------------------------------

#endif // HTTPSERVER_H
