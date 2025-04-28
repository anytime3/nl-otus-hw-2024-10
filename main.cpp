#include "src/HttpServer.h"
#include <fstream>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ostr.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
//------------------------------------------------------------------------------
void load_server_certificate(const std::string & private_key_path, const std::string & certificate_path, boost::asio::ssl::context & ctx)
{
  ctx.set_options(
      boost::asio::ssl::context::default_workarounds |
      boost::asio::ssl::context::no_sslv2);
  ctx.use_certificate_chain_file(certificate_path);
  ctx.use_private_key_file(private_key_path, ssl::context::pem);
}
//------------------------------------------------------------------------------
std::shared_ptr<spdlog::logger> initLogger()
{
  auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  auto basic_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>("logs/http_server.log");
  std::vector<spdlog::sink_ptr> sinks{console_sink, basic_sink};
  auto logger = std::make_shared<spdlog::logger>("server", sinks.begin(), sinks.end());
  spdlog::register_logger(logger);
  logger->flush_on(spdlog::level::warn);
  spdlog::set_pattern("[%d-%m-%Y %H:%M:%S.%e] | [%^%l%$] | %v");
  return logger;
}
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
  if (argc != 6)
  {
    std::cerr << "Usage: http-server <address> <port> <doc_root> <private_key_path> <certificate_path>\n"
              << "Example:\n"
              << "    http_server 0.0.0.0 8080 . localhost-key.pem localhost.pem\n";
    return EXIT_FAILURE;
  }

  try
  {
    auto const address = net::ip::make_address(argv[1]);
    auto const port = static_cast<unsigned short>(std::atoi(argv[2]));
    auto const doc_root = std::make_shared<std::string>(argv[3]);
    auto const threads = 1;
    auto const private_key_path = std::string {argv[4]};
    auto const certificate_path = std::string {argv[5]};

    auto log = initLogger();
    // The io_context is required for all I/O
    net::io_context ioc{threads};

    // The SSL context is required, and holds certificates
    ssl::context ctx{ssl::context::tlsv12};

    // This holds the self-signed certificate used by the server
    load_server_certificate(private_key_path, certificate_path, ctx);

    // Create and launch a listening port
    std::make_shared<listener>(ioc, ctx, tcp::endpoint{address, port}, doc_root, log)->run();

    // Run the I/O service on the requested number of threads
    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for(auto i = threads - 1; i > 0; --i)
      v.emplace_back(
      [&ioc]
      {
        ioc.run();
      });
    ioc.run();
  } catch (const std::exception & err)
  {
    std::cout << "Exception: " << err.what() << std::endl;
    return EXIT_FAILURE;
  }
}
