#pragma once

#include <stdexcept>
#include <iterator>
#include <mutex>
#include <algorithm>
#include <array>

#ifndef ENABLE_THREAD_SAFETY
#define ENABLE_THREAD_SAFETY true
#endif

namespace ESTL {
// Node structure for the list
template<typename T>
struct ListNode {
  T data;
  ListNode *prev;
  ListNode *next;
};

// Iterator for FixedList
template<typename T>
class FixedListIterator {
public:
  using iterator_category = std::bidirectional_iterator_tag;
  using value_type = T;
  using difference_type = std::ptrdiff_t;
  using pointer = T *;
  using reference = T &;

  explicit FixedListIterator(ListNode<T> *node) : m_node(node) {
  }

  reference operator*() const { return m_node->data; }
  pointer operator->() { return &m_node->data; }

  FixedListIterator &operator++() {
    m_node = m_node->next;
    return *this;
  }

  FixedListIterator operator++(int) {
    FixedListIterator temp = *this;
    m_node = m_node->next;
    return temp;
  }

  FixedListIterator &operator--() {
    m_node = m_node->prev;
    return *this;
  }

  FixedListIterator operator--(int) {
    FixedListIterator temp = *this;
    m_node = m_node->prev;
    return temp;
  }

  bool operator==(const FixedListIterator &other) const { return m_node == other.m_node; }
  bool operator!=(const FixedListIterator &other) const { return m_node != other.m_node; }

private:
  ListNode<T> *m_node;

  // For access to m_node
  template<typename U>
  friend class FixedList;
};

template<typename T>
class FixedList {
protected:
  ListNode<T> *m_storage; // Fixed-size array of nodes
  std::size_t m_capacity;
  std::size_t m_size;
  ListNode<T> *m_head;
  ListNode<T> *m_tail;
  ListNode<T> *m_freeList; // Points to the first free node
#if ENABLE_THREAD_SAFETY
  mutable std::mutex m_mutex;
#endif

public:
  using iterator = FixedListIterator<T>;
  using const_iterator = FixedListIterator<const T>;

  FixedList(ListNode<T> *buffer, std::size_t capacity) : m_storage(buffer), m_capacity(capacity), m_size(0),
                                                         m_head(nullptr), m_tail(nullptr), m_freeList(nullptr) {
    initFreeListPool();
  }

  // Helper to initialize the free bucket pool
  void initFreeListPool() {
    // Initialize free list with all nodes
    for (std::size_t i = 0; i < m_capacity - 1; ++i) {
      m_storage[i].next = &m_storage[i + 1]; // Link nodes in the free list
      m_storage[i].prev = nullptr; // Initialize prev pointers
    }
    m_storage[m_capacity - 1].next = nullptr; // Last node's next is null
    m_storage[m_capacity - 1].prev = nullptr; // Initialize prev pointer
    m_freeList = &m_storage[0]; // First free node
  }

  // Get a free node from the free list - O(1)
  ListNode<T> *getFreeNode() {
    if (!m_freeList) {
      throw std::out_of_range("FixedList is full");
    }
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    ListNode<T> *node = m_freeList;
    m_freeList = m_freeList->next; // Move free list pointer forward
    node->next = nullptr;
    node->prev = nullptr;
    return node;
  }

  // Return a node to the free list - O(1)
  void returnNode(ListNode<T> *node) {
    node->next = m_freeList;
    node->prev = nullptr;
    m_freeList = node;
  }

  // Size operations
  std::size_t size() const {
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return m_size;
  }

  std::size_t capacity() const { return m_capacity; }

  bool empty() const {
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return m_size == 0;
  }

  bool full() const {
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return m_size == m_capacity;
  }

  // Element access
  T &front() {
    if (!m_head) {
      throw std::out_of_range("FixedList is empty");
    }
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return m_head->data;
  }

  const T &front() const {
    if (!m_head) {
      throw std::out_of_range("FixedList is empty");
    }
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return m_head->data;
  }

  T &back() {
    if (!m_tail) {
      throw std::out_of_range("FixedList is empty");
    }
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return m_tail->data;
  }

  const T &back() const {
    if (!m_tail) {
      throw std::out_of_range("FixedList is empty");
    }
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return m_tail->data;
  }

  // Push to back - O(1)
  void push_back(const T &value) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedList is full");
    }

    ListNode<T> *newNode = getFreeNode();
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    newNode->data = value;
    newNode->next = nullptr;
    newNode->prev = m_tail;

    if (m_tail) m_tail->next = newNode;
    m_tail = newNode;
    if (!m_head) m_head = newNode;
    ++m_size;
  }

  // Push to front - O(1)
  void push_front(const T &value) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedList is full");
    }

    ListNode<T> *newNode = getFreeNode();
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    newNode->data = value;
    newNode->prev = nullptr;
    newNode->next = m_head;

    if (m_head) {
      m_head->prev = newNode;
    }
    m_head = newNode;
    if (!m_tail) {
      m_tail = newNode;
    }
    ++m_size;
  }

  template<typename... Args>
  iterator emplace(iterator pos, Args &&... args) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedList is full");
    }

    // Special cases for empty list or insertion at beginning/end
    if (!m_head || pos == begin()) {
      emplace_front(std::forward<Args>(args)...);
      return begin();
    }

    if (pos == end()) {
      emplace_back(std::forward<Args>(args)...);
      return iterator(m_tail);
    }

    // Regular case: insert between two nodes
    ListNode<T> *newNode = getFreeNode();
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    ListNode<T> *nextNode = pos.m_node;
    ListNode<T> *prevNode = nextNode->prev;

    newNode->data = T(std::forward<Args>(args)...);
    newNode->next = nextNode;
    newNode->prev = prevNode;

    prevNode->next = newNode;
    nextNode->prev = newNode;

    ++m_size;
    return iterator(newNode);
  }

  template<typename... Args>
  void emplace_front(Args &&... args) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedList is full");
    }

    ListNode<T> *newNode = getFreeNode();
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    newNode->data = T(std::forward<Args>(args)...);
    newNode->prev = nullptr;
    newNode->next = m_head;

    if (m_head) {
      m_head->prev = newNode;
    }
    m_head = newNode;
    if (!m_tail) {
      m_tail = newNode;
    }
    ++m_size;
  }

  template<typename... Args>
  void emplace_back(Args &&... args) {
    if (m_size >= m_capacity) {
      throw std::out_of_range("FixedList is full");
    }

    ListNode<T> *newNode = getFreeNode();
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    newNode->data = T(std::forward<Args>(args)...);
    newNode->next = nullptr;
    newNode->prev = m_tail;

    if (m_tail) m_tail->next = newNode;
    m_tail = newNode;
    if (!m_head) m_head = newNode;
    ++m_size;
  }

  // Pop from back - O(1)
  void pop_back() {
    if (!m_tail) {
      throw std::out_of_range("FixedList is empty");
    }
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    ListNode<T> *node = m_tail;
    m_tail = m_tail->prev;
    if (m_tail) {
      m_tail->next = nullptr;
    } else {
      m_head = nullptr;
    }
    returnNode(node);
    --m_size;
  }

  // Pop from front - O(1)
  void pop_front() {
    if (!m_head) {
      throw std::out_of_range("FixedList is empty");
    }
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    ListNode<T> *node = m_head;
    m_head = m_head->next;
    if (m_head) {
      m_head->prev = nullptr;
    } else {
      m_tail = nullptr;
    }
    returnNode(node);
    --m_size;
  }

  // Clear the list
  void clear() {
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    while (m_head) {
      ListNode<T> *node = m_head;
      m_head = m_head->next;
      returnNode(node);
    }
    m_tail = nullptr;
    m_size = 0;
  }

  // Iterator methods
  iterator begin() {
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return iterator(m_head);
  }

  iterator end() {
    return iterator(nullptr);
  }

  const_iterator begin() const {
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return const_iterator(m_head);
  }

  const_iterator end() const {
    return const_iterator(nullptr);
  }

  const_iterator cbegin() const {
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    return const_iterator(m_head);
  }

  const_iterator cend() const {
    return const_iterator(nullptr);
  }

  // Insert element at position
  iterator insert(iterator pos, const T &value) {
    if (m_size >= m_capacity) throw std::out_of_range("FixedList is full");

    // Special cases for empty list or insertion at beginning/end
    if (!m_head || pos == begin()) {
      push_front(value);
      return begin();
    }

    if (pos == end()) {
      push_back(value);
      return iterator(m_tail);
    }

    // Regular case: insert between two nodes
    ListNode<T> *newNode = getFreeNode();
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    ListNode<T> *nextNode = pos.m_node;
    ListNode<T> *prevNode = nextNode->prev;

    newNode->data = value;
    newNode->next = nextNode;
    newNode->prev = prevNode;

    prevNode->next = newNode;
    nextNode->prev = newNode;

    ++m_size;
    return iterator(newNode);
  }

  // Erase element at position
  iterator erase(iterator pos) {
    if (pos == end()) {
      throw std::out_of_range("Cannot erase end iterator");
    }

    ListNode<T> *node = pos.m_node;
    ListNode<T> *nextNode = node->next;

    // Special cases for erasing first or last element
    if (node == m_head) {
      pop_front();
      return iterator(m_head);
    }

    if (node == m_tail) {
      pop_back();
      return end();
    }
#if ENABLE_THREAD_SAFETY
    std::lock_guard<std::mutex> lock(m_mutex);
#endif
    // Regular case: remove from middle
    ListNode<T> *prevNode = node->prev;

    prevNode->next = nextNode;
    nextNode->prev = prevNode;

    returnNode(node);
    --m_size;

    return iterator(nextNode);
  }

  // Merge another list into this list
  void merge(FixedList &other) {
    if (this == &other) {
      return; // Avoid self-merge
    }

    iterator it1 = begin();
    iterator it2 = other.begin();

    while (it1 != end() && it2 != other.end()) {
      if (*it2 < *it1) {
        it1 = insert(it1, std::move(*it2));
        ++it2;
      }
      ++it1;
    }

    while (it2 != other.end()) {
      push_back(std::move(*it2));
      ++it2;
    }

    other.clear();
  }

  // Splice elements from another list into this list at the specified position
  void splice(iterator pos, FixedList &other) {
    if (this == &other) {
      return; // Avoid self-splice
    }

    while (!other.empty()) {
      insert(pos, std::move(other.front()));
      other.pop_front();
    }
  }

  // Remove all elements equal to the given value
  void remove(const T &value) {
    for (auto it = begin(); it != end();) {
      if (*it == value) {
        it = erase(it);
      } else {
        ++it;
      }
    }
  }

  // Remove all elements that satisfy the predicate
  template<typename Predicate>
  void remove_if(Predicate pred) {
    for (auto it = begin(); it != end();) {
      if (pred(*it)) {
        it = erase(it);
      } else {
        ++it;
      }
    }
  }

  // Remove consecutive duplicate elements
  void unique() {
    if (empty()) {
      return;
    }

    for (auto it = begin(); it != end();) {
      auto next = std::next(it);
      if (next != end() && *it == *next) {
        erase(next);
      } else {
        ++it;
      }
    }
  }
};

// Compile-time fixed list
template<typename T, std::size_t N>
class CTList : public FixedList<T> {
  std::array<ListNode<T>, N> m_storage;

public:
  CTList() : FixedList<T>(m_storage.data(), N) {
    this->initFreeListPool();
  }

  CTList(std::initializer_list<T> init) : CTList() {
    if (init.size() > N) {
      throw std::out_of_range("Initializer list too large");
    }
    for (const auto &val: init) {
      this->push_back(val);
    }
  }
};

// Run-time fixed list
template<typename T>
class RTList : public FixedList<T> {
public:
  explicit RTList(std::size_t capacity) : FixedList<T>(new ListNode<T>[capacity], capacity) {
    this->initFreeListPool();
  }

  RTList(std::size_t capacity, std::initializer_list<T> init) : RTList(capacity) {
    if (init.size() > capacity) {
      throw std::out_of_range("Initializer list too large");
    }
    for (const auto &val: init) {
      this->push_back(val);
    }
  }

  ~RTList() {
    delete[] this->m_storage;
  }

  // Prevent copying to avoid double-delete issues
  RTList(const RTList &) = delete;

  RTList &operator=(const RTList &) = delete;

  // Allow moving
  RTList(RTList &&other) noexcept : FixedList<T>(other.m_storage, other.capacity()) {
    other.m_storage = nullptr;
    other.m_head = nullptr;
    other.m_tail = nullptr;
    other.m_freeList = nullptr;
    other.m_size = 0;
  }

  RTList &operator=(RTList &&other) noexcept {
    if (this != &other) {
      delete[] this->m_storage;
      // Re-initialize the base class
      this->m_storage = other.m_storage;
      this->m_capacity = other.capacity;
      this->m_size = other.size;
      this->m_head = other.m_head;
      this->m_tail = other.m_tail;
      this->m_freeList = other.m_freeList;

      other.m_storage = nullptr;
      other.m_head = nullptr;
      other.m_tail = nullptr;
      other.m_freeList = nullptr;
      other.m_size = 0;
    }
    return *this;
  }
};
} // namespace ESTL
