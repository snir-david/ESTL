//
// Created by snirn on 3/12/25.
//
#pragma once

#include "ESTLUtils.hpp"
#include <algorithm>
#include <array>
#include <initializer_list>
#include <mutex>
#include <stdexcept>
#include <utility>

#ifndef ENABLE_THREAD_SAFETY
#define ENABLE_THREAD_SAFETY true
#endif

namespace ESTL {
// Base class for FixedVector
template <typename T> class FixedVector {
protected:
  T *m_data;
  std::size_t m_capacity;
  std::size_t m_size;
#if (ENABLE_THREAD_SAFETY)
  mutable std::mutex m_mutex;
#endif

public:
  using iterator = T *;
  using const_iterator = const T *;

  FixedVector(T *data, std::size_t capacity)
      : m_data(data), m_capacity(capacity), m_size(0) {}

  bool operator==(const FixedVector &other) const {
    return m_data == other.m_data && m_capacity == other.m_capacity &&
           m_size == other.m_size;
    ;
  }

  // Push element to back - O(1)
  void push_back(const T &value) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedVector overflow");
    }
#if (ENABLE_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    m_data[m_size++] = value;
  }

  // Emplace element at position - O(1)
  template <typename... Args> iterator emplace(iterator pos, Args &&...args) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedVector overflow");
    }
#if (ENABLE_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    std::move_backward(pos, end(), end() + 1);
    *pos = T(std::forward<Args>(args)...);
    ++m_size;
    return pos;
  }

  // Emplace element to back - O(1)
  template <typename... Args> void emplace_back(Args &&...args) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedVector overflow");
    }
#if (ENABLE_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    m_data[m_size++] = T(std::forward<Args>(args)...);
  }

  // Append range of elements - O(N)
  template <typename InputIt> void append_range(InputIt first, InputIt last) {
    while (first != last) {
      if (m_size >= m_capacity) {
        throw std::out_of_range("FixedVector overflow");
      }
#if (ENABLE_THREAD_SAFETY)
      std::lock_guard<std::mutex> lock(m_mutex);
#endif
      m_data[m_size++] = *first++;
    }
  }

  // Remove last element - O(1)
  void pop_back() {
    if (m_size == 0) {
      throw std::out_of_range("FixedVector underflow");
    }
#if (ENABLE_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    --m_size;
  }

  // Element access - O(1)
  T &operator[](std::size_t index) {
    if (index >= m_size) {
      throw std::out_of_range("Index out of bounds");
    }
    return m_data[index];
  }

  const T &operator[](std::size_t index) const {
    if (index >= m_size) {
      throw std::out_of_range("Index out of bounds");
    }
    return m_data[index];
  }

  // Bounds-checked access - O(1)
  T &at(std::size_t index) {
    if (index >= m_size) {
      throw std::out_of_range("Index out of bounds");
    }
    return m_data[index];
  }

  const T &at(std::size_t index) const {
    if (index >= m_size) {
      throw std::out_of_range("Index out of bounds");
    }
    return m_data[index];
  }

  // Access first and last element - O(1)
  T &front() { return m_data[0]; }
  T &back() { return m_data[m_size - 1]; }

  // Capacity methods - O(1)
  std::size_t size() const { return m_size; }
  std::size_t capacity() const { return m_capacity; }
  bool empty() const { return m_size == 0; }
  void clear() { m_size = 0; }

  // Swap two vectors - O(N)
  void swap(FixedVector &other) noexcept {
    std::swap(m_data, other.m_data);
    std::swap(m_size, other.m_size);
  }

  // Insert element at position - O(N)
  iterator insert(iterator pos, const T &value) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedVector overflow");
    }
#if (ENABLE_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    std::move_backward(pos, end(), end() + 1);
    *pos = value;
    ++m_size;
    return pos;
  }

  // Erase element at position - O(N)
  iterator erase(iterator pos) {
    if (pos >= end()) {
      throw std::out_of_range("Invalid erase position");
    }
#if (ENABLE_THREAD_SAFETY)
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    std::move(pos + 1, end(), pos);
    --m_size;
    return pos;
  }

  // Iterators - O(1)
  iterator begin() {
    WARN_IF(m_size == 0,
            "Calling front on an empty container causes undefined behavior.");
    return m_data;
  }

  const_iterator begin() const {
    WARN_IF(m_size == 0,
            "Calling front on an empty container causes undefined behavior.");
    return m_data;
  }

  iterator end() { return m_data + m_size; }
  const_iterator end() const { return m_data + m_size; }
};

// Compile-time fixed vector
template <typename T, std::size_t N> class CTVector : public FixedVector<T> {
private:
  std::array<T, N> m_storage;

public:
  CTVector() : FixedVector<T>(m_storage.data(), N) {}

  ~CTVector() = default;

  CTVector(const CTVector &other) : FixedVector<T>(m_storage.data(), N) {
    if (this != &other) {
      std::copy(other.m_storage.begin(), other.m_storage.end(), m_storage);
      this->m_capacity = other.m_capacity;
      this->m_size = other.m_size;
    }
  }

  CTVector &operator=(const CTVector &other) {
    if (this != &other) {
      m_storage.swap(const_cast<std::array<T, N> &>(other.m_storage));
      this->m_capacity = other.m_capacity;
      this->m_size = other.m_size;
    }
    return *this;
  }

  CTVector(CTVector &&other) noexcept
      : m_storage(std::move(other.m_storage)),
        FixedVector<T>(m_storage.data(), N) {
    this->m_capacity = other.m_capacity;
    this->m_size = other.m_size;

    other.m_data = nullptr;
    other.m_size = other.m_capacity = 0;
  }

  CTVector &operator=(CTVector &&other) noexcept {
    if (this != &other) {
      m_storage = std::move(other.m_storage);
      this->m_capacity = other.m_capacity;
      this->m_size = other.m_size;

      other.m_data = nullptr;
      other.m_size = other.m_capacity = 0;
    }
    return *this;
  }

  CTVector(std::initializer_list<T> init)
      : FixedVector<T>(m_storage.data(), N) {
    if (init.size() > N) {
      throw std::out_of_range("Initializer list too large");
    }
    std::copy(init.begin(), init.end(), this->m_data);
    this->m_size = init.size();
  }
};

// Run-time fixed vector
template <typename T> class RTVector : public FixedVector<T> {
public:
  explicit RTVector(std::size_t capacity)
      : FixedVector<T>(new T[capacity], capacity) {}

  ~RTVector() { delete[] this->m_data; }

  RTVector(const RTVector &other)
      : FixedVector<T>(new T[other.m_capacity], other.m_capacity) {
    if (this != &other) {
      std::copy(other.m_data, other.m_data + other.m_size, this->m_data);
      this->m_size = other.m_size;
    }
  }

  RTVector &operator=(const RTVector &other) {
    if (this != &other) {
      delete[] this->m_data;
      this->m_data = new T[other.m_capacity];
      std::copy(other.m_data, other.m_data + other.m_size, this->m_data);
      this->m_capacity = other.m_capacity;
      this->m_size = other.m_size;
    }
    return *this;
  }

  RTVector(RTVector &&other) noexcept
      : FixedVector<T>(other.m_data, other.m_capacity) {
    other.m_data = nullptr;
    other.m_capacity = other.m_size = 0;
  }

  RTVector &operator=(RTVector &&other) noexcept {
    if (this != &other) {
      delete this->m_data;
      this->m_data = other.m_data;
      this->m_capacity = other.m_capacity;
      this->m_size = other.m_size;

      other.m_data = nullptr;
      other.m_capacity = other.m_size = 0;
    }
    return *this;
  }

  RTVector(std::initializer_list<T> init, std::size_t capacity = 0) : RTVector(capacity ? capacity : init.size()) {
    if (init.size() > capacity) {
      throw std::out_of_range("Initializer list too large");
    }
    std::copy(init.begin(), init.end(), this->m_data);
    this->m_size = init.size();
  }
};
} // namespace ESTL
