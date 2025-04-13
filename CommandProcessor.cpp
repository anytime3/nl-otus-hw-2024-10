#include "CommandProcessor.h"
#include <iostream>
namespace
{
std::string l_dynamicBlockStart = "{";
std::string l_dynamicBlockEnd = "}";
}
//-----------------------------------------------------------------------------
void ConsoleLogger::update(const std::vector<std::string> & commands, time_t time)
{
  std::cout << "bulk: ";
  for (size_t i = 0; i < commands.size(); ++i)
  {
    if (i != 0 && commands[i].size() > 0)
      std::cout << ", ";
    std::cout << commands[i];
  }
  std::cout << std::endl;
}
//-----------------------------------------------------------------------------
void FileLogger::update(const std::vector<std::string> & commands, time_t time)
{
  std::string filename = "bulk" + std::to_string(time) + ".log";
  std::ofstream file(filename);

  if (file.is_open())
  {
    for (size_t i = 0; i < commands.size(); i++)
    {
      if (i != 0 && commands[i].size() > 0)
        file << ", ";
      file << commands[i];
    }
    file << std::endl;
  }
  file.close();
}
//-----------------------------------------------------------------------------
CommandProcessor::CommandProcessor(size_t size) :
  _block_size(size),
  _first_command_time(0),
  _dynamic_block(false),
  _block_depth(0)
{
  _observers.push_back(std::make_shared<ConsoleLogger>());
  _observers.push_back(std::make_shared<FileLogger>());
}
//-----------------------------------------------------------------------------
void CommandProcessor::notify_observers()
{
  if (_commands.empty() == false)
  {
    for (const auto& observer : _observers)
      observer->update(_commands, _first_command_time);
    _commands.clear();
  }
}
//-----------------------------------------------------------------------------
void CommandProcessor::process_command(const std::string & cmd)
{
  if (_commands.empty())
    _first_command_time = std::chrono::system_clock::to_time_t(std::chrono::system_clock::now());

  if (cmd == l_dynamicBlockStart)
  {
    if (_dynamic_block == false)
    {
      notify_observers();
      _dynamic_block = true;
    } else
    {
      _block_depth++;
    }
    return;
  }

  if (cmd == l_dynamicBlockEnd)
  {
    if (_dynamic_block)
    {
      if (_block_depth == 0)
      {
        notify_observers();
        _dynamic_block = false;
      } else
      {
        _block_depth--;
      }
    }
    return;
  }

  _commands.push_back(cmd);
  if (_dynamic_block == false && _commands.size() >= _block_size)
    notify_observers();
}
//-----------------------------------------------------------------------------
CommandProcessor::~CommandProcessor()
{
  if (_dynamic_block == false)
    notify_observers();
}
//-----------------------------------------------------------------------------
