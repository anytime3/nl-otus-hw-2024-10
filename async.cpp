#include "async.h"
#include "CommandProcessor.h"

#include <string>
#include <unordered_map>
#include <mutex>
#include <sstream>
#include <memory>
#include <vector>

namespace async {

struct Context {
  std::mutex mutex;
  std::unique_ptr<CommandProcessor> processor;
  std::string buffer;

  explicit Context(std::size_t bulk_size)
    : processor(std::make_unique<CommandProcessor>(bulk_size))
  {
  }
};

std::mutex g_contexts_mutex;
std::unordered_map<handle_t, std::shared_ptr<Context>> g_contexts;

handle_t connect(std::size_t bulk) {
  if (bulk == 0) return nullptr;

  auto context = std::make_shared<Context>(bulk);
  handle_t handle = static_cast<handle_t>(context.get());

  std::lock_guard<std::mutex> lock(g_contexts_mutex);
  g_contexts[handle] = std::move(context);

  return handle;
}

void receive(handle_t handle, const char *data, std::size_t size) {
  if (handle == nullptr || data == nullptr || size == 0)
    return;

  std::shared_ptr<Context> context;
  {
    std::lock_guard<std::mutex> lock(g_contexts_mutex);
    auto it = g_contexts.find(handle);
    if (it == g_contexts.end()) return;
    context = it->second;
  }

  std::lock_guard<std::mutex> lock(context->mutex);
  context->buffer.append(data, size);

  std::size_t pos = 0;
  while ((pos = context->buffer.find('\n')) != std::string::npos) {
    std::string command = context->buffer.substr(0, pos);
    context->processor->process_command(command);
    context->buffer.erase(0, pos + 1);
  }
}

void disconnect(handle_t handle) {
  if (handle == nullptr) return;

  std::shared_ptr<Context> context;
  {
    std::lock_guard<std::mutex> lock(g_contexts_mutex);
    auto it = g_contexts.find(handle);
    if (it == g_contexts.end()) return;
    context = it->second;
    g_contexts.erase(it);
  }

  std::lock_guard<std::mutex> lock(context->mutex);
  if (!context->buffer.empty()) {
    std::istringstream stream(context->buffer);
    std::string command;
    while (std::getline(stream, command))
      context->processor->process_command(command);
    context->buffer.clear();
  }

  context->processor.reset(); // вручную разрушить, чтобы сбросить остатки
}

} // namespace async
