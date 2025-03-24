#ifndef MATRIX_H
#define MATRIX_H

#include <unordered_map>
#include <tuple>
#include <iostream>

template<typename T, T defaultValue>
class IMatrix
{
protected:
  class MatrixRow
  {
  private:
    std::unordered_map<int, T> _data;
    T                          _defaultValue;

  public:
    MatrixRow() : _defaultValue(defaultValue) {}
    T & operator[](int x)
    {
      if (_data.find(x) == _data.end()) {
        _data[x] = _defaultValue;
      }
      return _data[x];
    }

    std::unordered_map<int, T> & getData() { return _data; }
    T & getDefaultValue() { return _defaultValue; }

    typename std::unordered_map<int, T>::const_iterator begin() const { return _data.begin(); }

    typename std::unordered_map<int, T>::const_iterator end() const { return _data.end(); }
  };

  T _defaultValue;
  std::unordered_map<int, MatrixRow> _data;

public:
  IMatrix() : _defaultValue(defaultValue) {}

  virtual MatrixRow & operator[](int x) = 0;
  virtual size_t size() = 0;
  virtual ~IMatrix() = default;

  class Iterator
  {
  public:
    Iterator(typename std::unordered_map<int, MatrixRow>::const_iterator it,
             typename std::unordered_map<int, MatrixRow>::const_iterator end) :
        _outerIt(it),
        _outerEnd(end)
    {
      if (_outerIt != _outerEnd)
        _innerIt = _outerIt->second.begin();
      skipDefaultValues();
    }

    std::tuple<int, int, T> operator*() const { return std::make_tuple(_outerIt->first, _innerIt->first, _innerIt->second); }

    Iterator& operator++() {
      if (_outerIt == _outerEnd)
        return *this;

      ++_innerIt;
      skipDefaultValues();

      return *this;
    }

    bool operator!=(const Iterator& other) const { return _outerIt != other._outerIt || (_outerIt != _outerEnd && _innerIt != other._innerIt); }
  private:
    typename std::unordered_map<int, MatrixRow>::const_iterator _outerIt;
    typename std::unordered_map<int, MatrixRow>::const_iterator _outerEnd;
    typename std::unordered_map<int, T>::const_iterator         _innerIt;

    void skipDefaultValues()
    {
      while (_outerIt != _outerEnd)
      {
        while (_innerIt != _outerIt->second.end())
        {
          if (_innerIt->second != defaultValue)
            return;
          ++_innerIt;
        }

        ++_outerIt;
        if (_outerIt != _outerEnd)
          _innerIt = _outerIt->second.begin();
      }
    }
  };

  virtual Iterator begin() const = 0;
  virtual Iterator end() const = 0;
};


template<typename T, T defaultValue>
class Matrix : public IMatrix<T, defaultValue>
{
public:
  using MatrixRow = typename IMatrix<T, defaultValue>::MatrixRow;
  using Iterator = typename IMatrix<T, defaultValue>::Iterator;

  Matrix() : IMatrix<T, defaultValue>() {}

  MatrixRow & operator[](int x) override { return this->_data[x]; }

  size_t size() override
  {
    size_t total = 0;
    for (auto & row : this->_data)
    {
      for (auto & cell : row.second.getData())
      {
        if (cell.second != defaultValue)
          total++;
      }
    }
    return total;
  }

  ~Matrix() override {}

  MatrixRow & at(int x)
  {
    if (this->_data.find(x) == this->_data.end())
      this->_data[x] = MatrixRow();

    return this->_data[x];
  }

  Iterator begin() const override { return Iterator(this->_data.begin(), this->_data.end()); }

  Iterator end() const override { return Iterator(this->_data.end(), this->_data.end()); }

};

template<typename T, T defaultValue>
class MatrixProxy : public IMatrix<T, defaultValue>
{
private:
  std::unique_ptr<Matrix<T, defaultValue>> _matrix;
public:
  using MatrixRow = typename IMatrix<T, defaultValue>::MatrixRow;
  using Iterator = typename IMatrix<T, defaultValue>::Iterator;

  MatrixProxy() : IMatrix<T, defaultValue>(), _matrix(std::make_unique<Matrix<T, defaultValue>>()) {}

  MatrixRow & operator[](int x) override { return _matrix->at(x); }
  size_t size() override {return _matrix->size();}
  ~MatrixProxy() override {}

  Iterator begin() const override { return _matrix->begin(); }

  Iterator end() const override { return _matrix->end(); }
};

#endif // MATRIX_H
