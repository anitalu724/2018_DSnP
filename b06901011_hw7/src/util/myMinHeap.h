/****************************************************************************
  FileName     [ myMinHeap.h ]
  PackageName  [ util ]
  Synopsis     [ Define MinHeap ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_MIN_HEAP_H
#define MY_MIN_HEAP_H

#include <algorithm>
#include <vector>

template <class Data>
class MinHeap
{
public:
   MinHeap(size_t s = 0) { if (s != 0) _data.reserve(s); }
   ~MinHeap() {}

   void clear() { _data.clear(); }

   // For the following member functions,
   // We don't respond for the case vector "_data" is empty!
   const Data& operator [] (size_t i) const { return _data[i]; }   
   Data& operator [] (size_t i) { return _data[i]; }

   size_t size() const { return _data.size(); }

   // TODO
   void sort_heap(size_t i){
      size_t index = i; size_t left = index*2; size_t right = index*2+1; size_t parent = index/2;
      if (parent > 0) {
         if (_data[index-1] < _data[parent-1]){
            swap(_data[index-1], _data[parent-1]);
            index = parent;
            sort_heap(index);
         }
      }
      

      if (left <= _data.size()){                         //至少有left
         if (right <= _data.size()){                     //有left and right
            if(_data[right-1] < _data[left-1]){
               if (_data[right-1] < _data[index-1]){
                  swap(_data[right-1], _data[index-1]);
                  index = right;
                  sort_heap(index);
               }
            }
            else {
               if (_data[left-1] < _data[index-1]){
                  swap(_data[left-1], _data[index-1]);
                  index = left;
                  sort_heap(index);
               }
            }
         }
         else{
            if (_data[left-1] < _data[index-1]){
               swap(_data[left-1], _data[index-1]);
               index = left;
               sort_heap(index);
            }
         }
      }
   }

   const Data& min() const { return _data[0]; }

   void insert(const Data& d) {
      _data.push_back(d);
		shape(size() - 1);
   }

   void shape(size_t i){
      size_t index = i;
		if(index == 0) return;
		size_t up;
		index % 2 == 0 ? (up = index/2 - 1) : (up = index/2);
		if(_data[index] < _data[up]){
			swap(_data[up], _data[index]);
			shape(up);
		}
	}


   void delMin() {
      if(_data.empty()) return;
      _data[0] = _data[_data.size()-1];
      _data.erase(--_data.end());
      //cerr << "size=" << size() << endl;
      size_t index = 1;
      sort_heap(index);
     // cerr << min() << endl;
   }



   void delData(size_t i){ 
      if (i == _data.size()-1){
         _data[i] = _data[_data.size()-1];
         _data.erase(--_data.end());
      } 
      else {
         _data[i] = _data[_data.size()-1];
         _data.erase(--_data.end());
         size_t index = i+1;
         sort_heap(index);
      }
   }

private:
   // DO NOT add or change data members
   vector<Data>   _data;
};

#endif // MY_MIN_HEAP_H
