#include <iostream>
#include <cstring>
#include <memory>

template<typename T, std::size_t BufferSize>
struct FixedAllocator
{
private:
  T *    _buffer;
  T *    _curPos;
  T *    _end;
  size_t _bufferSize;
  std::shared_ptr<FixedAllocator<T, BufferSize>> _next;

  void reallocate()
  {
    _next = std::make_shared<FixedAllocator<T, BufferSize>>();
  }
public:
  using value_type = T;

  FixedAllocator() :
    _bufferSize(BufferSize),
    _next(nullptr)
  {
    _buffer = static_cast<T *>(::operator new(BufferSize * sizeof(T)));
    _curPos = _buffer;
    _end = _buffer + BufferSize;
  }
  ~FixedAllocator()
  {
    ::operator delete(_buffer);
  }

  template<typename U>
  struct rebind
  {
    using other = FixedAllocator<U, BufferSize>;
  };

  template<typename U>
  FixedAllocator(const FixedAllocator<U, BufferSize> &) {}

  T * allocate(size_t n)
  {
    if (_curPos + n > _end)
    {
      if (_next == nullptr)
        reallocate();
      return _next.get()->allocate(n);
    } else
    {
      T * result = _curPos;
      _curPos += 1;
      return result;
    }
  }

  void deallocate(T * p, size_t n)
  {
    if (_curPos - 1 <= _buffer)
      std::bad_alloc();
    _curPos -= 1;
  }

  template<typename U>
  void destroy(U * p)
  {
    p->~U();
  }

  template <typename U, typename... Args>
  void construct(U* p, Args&&... args) {
    new (p) U(std::forward<Args>(args)...);
  }

  bool operator==(const FixedAllocator& other) const {
    return _buffer == other._buffer;
  }

  bool operator!=(const FixedAllocator& other) const {
    return *this != other;
  }
};