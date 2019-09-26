/****************************************************************************
  FileName     [ myHashSet.h ]
  PackageName  [ util ]
  Synopsis     [ Define HashSet ADT ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2014-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef MY_HASH_SET_H
#define MY_HASH_SET_H

#include <vector>

using namespace std;

//---------------------
// Define HashSet class
//---------------------
// To use HashSet ADT,
// the class "Data" should at least overload the "()" and "==" operators.
//
// "operator ()" is to generate the hash key (size_t)
// that will be % by _numBuckets to get the bucket number.
// ==> See "bucketNum()"
//
// "operator ==" is to check whether there has already been
// an equivalent "Data" object in the HashSet.
// Note that HashSet does not allow equivalent nodes to be inserted
//
template <class Data>
class HashSet
{
public:
   HashSet(size_t b = 0) : _numBuckets(0), _buckets(0) { if (b != 0) init(b); }
   ~HashSet() { reset(); }

   // TODO: implement the HashSet<Data>::iterator
   // o An iterator should be able to go through all the valid Data
   //   in the Hash
   // o Functions to be implemented:
   //   - constructor(s), destructor
   //   - operator '*': return the HashNode
   //   - ++/--iterator, iterator++/--
   //   - operators '=', '==', !="
   //
   class iterator
   {
      friend class HashSet<Data>;
      public:
         iterator(vector<Data>* ptr, typename vector<Data>::iterator d, size_t i, size_t num): _buckets(ptr),_it(d),_index(i),_numBuckets(num){}
	      const Data& operator * () const { return *_it; }
	      iterator& operator ++ () {
            _it++;
            if(_it == _buckets[_index].end()){
	      		_index++;
	      		for(; _index < _numBuckets ; ++_index){
                  if(!_buckets[_index].empty()){
	      				_it = _buckets[_index].begin();
	      				break;
	      			}
               }
	      		if(_index == _numBuckets){
	      			_it = _buckets[_numBuckets-1].end();
	      			_index--;
	      		}
	      	}
	      	return *this;
	      }
	      iterator operator ++ (int){ 
	      	iterator tmp = *this;
	      	++*this;
	      	return tmp;
	      }
	      iterator& operator -- (){
	      	if(_it == _buckets[_index].begin() && _index != 0)  _it = _buckets[--_index].end();
	      	--_it;
	      	return *this;
	      }
	      iterator operator -- (int){
	      	iterator tmp = *this;
	      	--*this;
	      	return tmp;
	      }
	      bool operator == (const iterator& i) const { return _it == i._it; }
	      bool operator != (const iterator& i) const { return _it != i._it; }
	      iterator& operator = (const iterator& i){
	      	_buckets = i._bucket;   _it = i._it;   _index = i._index;   _numBuckets = i._numBuckets;
	      	return *this;
	      }
	private:
		vector<Data>*                          _buckets;
      typename vector<Data>::iterator        _it;
		size_t 					                  _index;
		const size_t 			                  _numBuckets;
   };

   void init(size_t b) { _numBuckets = b; _buckets = new vector<Data>[b]; }
   void reset() {
      _numBuckets = 0;
      if (_buckets) { delete [] _buckets; _buckets = 0; }
   }
   void clear() {
      for (size_t i = 0; i < _numBuckets; ++i) _buckets[i].clear();
   }
   size_t numBuckets() const { return _numBuckets; }

   vector<Data>& operator [] (size_t i) { return _buckets[i]; }
   const vector<Data>& operator [](size_t i) const { return _buckets[i]; }

   // TODO: implement these functions
   //
   // Point to the first valid data
   iterator begin() const {
      if (empty()) return end();
      for (size_t i = 0; i < _numBuckets; i++){
         if (_buckets[i].size() != 0)  return iterator(_buckets,_buckets[i].begin(),i,_numBuckets); 
      }
   }
   // Pass the end
   iterator end() const { return iterator(_buckets, _buckets[_numBuckets-1].end(), _numBuckets-1, _numBuckets); }
   // return true if no valid data
   bool empty() const { if (size() == 0) return true; return false; }
   // number of valid data
   size_t size() const {
      size_t s = 0;
      for (size_t i = 0; i < _numBuckets; i++)  s += _buckets[i].size();
      return s; 
   }

   // check if d is in the hash...
   // if yes, return true;
   // else return false;
   bool check(const Data& d) const {
      size_t index = bucketNum(d);
      for (size_t i = 0; i < _buckets[index].size(); i++){
         if (d == _buckets[index][i]) return true;
      }
      return false; 
   }

   // query if d is in the hash...
   // if yes, replace d with the data in the hash and return true;
   // else return false;
   bool query(Data& d) const {
      size_t index = bucketNum(d);
      for (size_t i = 0; i < _buckets[index].size(); i++){
         if (d == _buckets[index][i]){
            d = _buckets[index][i];
            return true;
         } 
      }
      return false; 
   }

   // update the entry in hash that is equal to d (i.e. == return true)
   // if found, update that entry with d and return true;
   // else insert d into hash as a new entry and return false;
   bool update(const Data& d) {
      size_t index = bucketNum(d);
      for (size_t i = 0; i < _buckets[index].size(); i++){
         if (d == _buckets[index][i]){
            _buckets[index][i] = d;
            return true;
         }
      }
      _buckets[index].push_back(d);
      return false; 
   }

   // return true if inserted successfully (i.e. d is not in the hash)
   // return false is d is already in the hash ==> will not insert
   bool insert(const Data& d) {
      if (check(d)) return false;
      _buckets[bucketNum(d)].push_back(d);
      return true;
   }

   // return true if removed successfully (i.e. d is in the hash)
   // return fasle otherwise (i.e. nothing is removed)
   bool remove(const Data& d) {
      if (check(d)){
         size_t index = bucketNum(d);
         typename vector<Data>::iterator it(_buckets[index].begin()) ;
         for (size_t i = 0; i < _buckets[index].size(); i++){
            if (d == *(it+i)){
               _buckets[index][i] = _buckets[index][_buckets[index].size()-1];
               _buckets[index].erase(it+_buckets[index].size()-1);
               return true;
            }
         }
      }
      else return false;
   }

private:
   // Do not add any extra data member
   size_t            _numBuckets;
   vector<Data>*     _buckets;

   size_t bucketNum(const Data& d) const {
      return (d() % _numBuckets); }
};

#endif // MY_HASH_SET_H
