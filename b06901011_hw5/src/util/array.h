/****************************************************************************
  FileName     [ array.h ]
  PackageName  [ util ]
  Synopsis     [ Define dynamic array package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef ARRAY_H
#define ARRAY_H

#include <cassert>
#include <algorithm>

using namespace std;

// NO need to implement class ArrayNode
template <class T>
class Array
{
public:
   // TODO: decide the initial value for _isSorted
   Array() : _data(0), _size(0), _capacity(0) {}
   ~Array() { delete []_data; }

   // DO NOT add any more data member or function for class iterator
   class iterator
   {
      friend class Array;

   public:
      iterator(T* n= 0): _node(n) {}
      iterator(const iterator& i): _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return (*_node); }
      T& operator * () { return (*_node); }
      iterator& operator ++ () { _node++; return *this; }
      iterator operator ++ (int) {iterator tmp = *this; ++(*this); return tmp; }
      iterator& operator -- () { _node--; return *this; }
      iterator operator -- (int) {iterator tmp = *this; --(*this); return tmp; }

      iterator operator + (int i) const { iterator tmp = *this;  tmp += i; return tmp; }
      iterator& operator += (int i) { _node += i; return *this; }

      iterator& operator = (const iterator& i) { _node = i._node; return (*this); }

      bool operator != (const iterator& i) const { if(_node == i._node) return false; return true; }
      bool operator == (const iterator& i) const { if(_node == i._node) return true; return false; }

   private:
      T*    _node;
   };

   // TODO: implement these functions
   iterator begin() const { return _data; }
   iterator end() const { return _data + _size; }
   bool empty() const { if (_size == 0) return true; return false; }
   size_t size() const { return _size; }
   T& operator [] (size_t i) { return _data[i]; }
   const T& operator [] (size_t i) const { return _data[i]; }
   void push_back(const T& x) {
      if (_size == _capacity){
         if (_capacity == 0){
            _capacity++;
            _data = new T[_capacity];
         }
         else {
            _capacity *=2;
            T* tmp = new T[_capacity];
            for(size_t i = 0; i < _size; i++) tmp[i] = _data[i];
            delete[] _data;
            _data = tmp;
         }
      }
      _data[_size] = x;
      _size++;
   }
   void pop_front() { erase(begin()); }
   void pop_back() { erase(end()); }

   bool erase(iterator pos) {
      if (empty()) return false;
      size_t i = pos._node - _data;
      _data[i] = _data[_size-1];
      _size--;
      return true;
   }
   bool erase(const T& x) {
      if (find(x) == end()) return false;
      size_t i = find(x)._node - _data;
      _data[i] = _data[_size-1];
      _size--;
      return true;
   }

   iterator find(const T& x) {
      iterator it = end();
      for (size_t i = 0; i < _size; i++){
         if (_data[i] == x){
            it = _data + i;  
            return it;
         }  
      } 
      return it;
   }

   void clear() {
      for (size_t i = 0; i < _size; i++) erase(_data[i]);
      _size = 0;
   }

   // [Optional TODO] Feel free to change, but DO NOT change ::sort()
   void sort() const { if (!empty()) ::sort(_data, _data+_size); }

   // Nice to have, but not required in this homework...
   // void reserve(size_t n) { ... }
   // void resize(size_t n) { ... }

private:
   // [NOTE] DO NOT ADD or REMOVE any data member
   T*            _data;
   size_t        _size;       // number of valid elements
   size_t        _capacity;   // max number of elements
   mutable bool  _isSorted;   // (optionally) to indicate the array is sorted

   // [OPTIONAL TODO] Helper functions; called by public member functions
};

#endif // ARRAY_H
