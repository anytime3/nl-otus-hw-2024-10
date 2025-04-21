#include <iostream>
#include <string>
#include <array>
#include "CommandProcessor.h"
#include <boost/asio.hpp>
#include <sstream>
#include <vector>

void splitString(const std::string & str, char delim, std::vector<std::string> & result) {
    std::stringstream ss (str);
    std::string item;

    while (getline (ss, item, delim)) {
        result.push_back(item);
    }
}


class Session : public std::enable_shared_from_this<Session>
{
public:
  Session(boost::asio::ip::tcp::socket socket, CommandProcessor & cmdProc) :
    _socket(std::move(socket)), _cmdProc(cmdProc)
  {
  }

  void start()
  {
    do_read();
  }

private:
  void do_read()
  {
    auto self(shared_from_this());
    _socket.async_read_some(boost::asio::buffer(_data.data(), _bufferLength),
        [this, self](boost::system::error_code ec, std::size_t length)
        {
          if (!ec)
          {
            std::string dataStr(_data.data(), length);
            std::vector<std::string> cmds;
            splitString(dataStr, '\n', cmds);
            for (const auto & cmd : cmds)
              _cmdProc.process_command(cmd);
          }
        });
  }

  boost::asio::ip::tcp::socket _socket;
  const size_t                 _bufferLength = 1024;
  std::array<char, 1024>       _data;
  CommandProcessor &           _cmdProc;
};


class Server
{
public:
  Server(boost::asio::io_context & io_context, int port, CommandProcessor & cmdProc) :
    _acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)),
    _cmdProc(cmdProc)
  {
    do_accept();
  }

private:
  void do_accept()
  {
    _acceptor.async_accept(
        [this](boost::system::error_code ec, boost::asio::ip::tcp::socket socket)
        {
          if (!ec)
          {
            std::make_shared<Session>(std::move(socket), _cmdProc)->start();
          }

          do_accept();
        });
  }

  boost::asio::ip::tcp::acceptor _acceptor;
  CommandProcessor & _cmdProc;
};



int main(int argc, char* argv[]) {
  if (argc != 3) {
    std::cerr << "Usage: <port> <block_size>" << std::endl;
    return 1;
  }

  int port;
  size_t block_size;
  try {
    port = std::stoi(argv[1]);
    block_size = std::stoul(argv[2]);
  } catch (const std::exception & e)
  {
    std::cerr << "Invalid port or block size" << std::endl;
    return 1;
  }
  CommandProcessor processor(block_size);

  boost::asio::io_context io_context;
  Server server(io_context, port, processor);
  io_context.run();
}
