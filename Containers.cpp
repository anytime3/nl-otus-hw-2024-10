#include <iostream>
#include <iterator>
#include <exception>
#include <memory>



template<class T, class Allocator = std::allocator<T>>
class CustomContainer
{
private:
  struct Node
  {
    T *                   _data;
    std::shared_ptr<Node> _next;

    Node() = delete;

    Node(T * obj) : _data(obj), _next(nullptr) {}
  };

  struct Iterator
  {
  private:
    std::shared_ptr<Node> _current;
  public:
    explicit Iterator(std::shared_ptr<Node> & node) : _current(node) {}

    T & operator*() { return *_current.get()->_data; }
    Iterator &operator++()
    {
      _current = _current.get()->_next;
      return *this;
    }

    bool operator!=(const Iterator &other) const { return _current != other._current; }
  };

  std::shared_ptr<Node> _begin;
  std::shared_ptr<Node> _end;
  Allocator _allocator;
  size_t _size;

public:
  CustomContainer() : _begin(nullptr), _end(nullptr), _size(0) {}
  ~CustomContainer()
  {
    clear();
  }

  void push_back(const T & value)
  {
    std::shared_ptr<Node> newNode = std::make_shared<Node>(_allocator.allocate(1));
    _allocator.construct(newNode.get()->_data, value);
    if (_begin == nullptr)
    {
      _begin = newNode; _end = _begin;
    }
    else
    {
      _end.get()->_next = newNode;
      _end = newNode;
    }
    ++_size;
  }
  
  void clear()
  {
    auto current = _begin;
    while (current)
    {
      auto next = current.get()->_next;
      _allocator.destroy(current.get()->_data);
      _allocator.deallocate(current.get()->_data, 1);
      current = next;
    }
    _begin = nullptr; _end = nullptr;
    _size = 0;
  }

  size_t size() const { return _size; }

  Iterator begin() { return Iterator(_begin); }
  Iterator end() { std::shared_ptr<Node> nlpt = nullptr; return Iterator(nlpt); }

  bool empty() const { return _size == 0 || _begin == nullptr; }

  T at(size_t n) const
  {
    if (n < 0 || n > _size)
      throw std::range_error("Out of range");
    auto current = _begin;
    for (size_t i = 0; i < n; i++)
    {
      current = current.get()->_next;
    }
    return *current.get()->_data;
  }

};