#ifndef COMMANDPROCESSOR_H
#define COMMANDPROCESSOR_H
//-----------------------------------------------------------------------------
#include <vector>
#include <chrono>
#include <fstream>
#include <memory>
//-----------------------------------------------------------------------------
class IObserver
{
public:
  virtual ~IObserver() = default;
  virtual void update(const std::vector<std::string>& commands, time_t time) = 0;
};
//-----------------------------------------------------------------------------
class ConsoleLogger : public IObserver
{
public:
  void update(const std::vector<std::string> & commands, time_t time) override;
};
//-----------------------------------------------------------------------------
class FileLogger : public IObserver
{
public:
  void update(const std::vector<std::string> & commands, time_t time) override;
};
//-----------------------------------------------------------------------------
class CommandProcessor {
private:
  std::vector<std::shared_ptr<IObserver>> _observers;
  std::vector<std::string>                _commands;
  time_t                                  _first_command_time;
  bool                                    _dynamic_block;
  int                                     _block_depth;
  size_t                                  _block_size;

  void notify_observers();

public:
  CommandProcessor(size_t size);

  void process_command(const std::string & cmd);

  ~CommandProcessor();
};
//-----------------------------------------------------------------------------
#endif // COMMANDPROCESSOR_H
