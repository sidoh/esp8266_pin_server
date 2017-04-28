#include <Arduino.h>

#ifndef _LINKEDLIST_H
#define _LINKEDLIST_H

template<typename T>
struct LinkedListNode {
  LinkedListNode(T value)
    : value(value),
      next(NULL)
  { }

  ~LinkedListNode() { }

  T value;
  volatile LinkedListNode* next;
};

template <typename T>
class LinkedList {
public:
  LinkedList()
    : _head(NULL),
      _tail(NULL),
      _size(0)
  { }

  ~LinkedList() {
    volatile LinkedListNode<T>* next;
    for (volatile LinkedListNode<T>* cur = first(); cur != NULL; cur = next) {
      next = cur->next;
      delete cur;
    }
  }

  void pushBack(T value) {
    LinkedListNode<T>* node = new LinkedListNode<T>(value);

    if (this->_head == NULL) {
      this->_head = node;
      this->_tail = node;
    } else {
      this->_tail->next = node;
      this->_tail = node;
    }

    this->_size++;
  }

  T pop() {
    volatile LinkedListNode<T>* popped = _head;

    if (popped != NULL) {
      T value = _head->value;
      this->_head = popped->next;
      this->_size--;
      delete _head;

      return value;
    }
  }

  volatile LinkedListNode<T>* first() {
    return _head;
  }

  LinkedListNode<T>* last() {
    return _tail;
  }

  size_t size() {
    return _size;
  }

protected:
  volatile LinkedListNode<T>* _head;
  volatile LinkedListNode<T>* _tail;
  volatile size_t _size;
};

#endif
