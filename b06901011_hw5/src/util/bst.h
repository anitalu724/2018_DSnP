/****************************************************************************
  FileName     [ bst.h ]
  PackageName  [ util ]
  Synopsis     [ Define binary search tree package ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2005-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef BST_H
#define BST_H

#include <cassert>
using namespace std;

template <class T> class BSTree;

// BSTreeNode is supposed to be a private class. User don't need to see it.
// Only BSTree and BSTree::iterator can access it.
//
// DO NOT add any public data member or function to this class!!
//
template <class T>
class BSTreeNode
{
	// TODO: design your own class!!
	friend class BSTree<T>;
	friend class BSTree<T>:: iterator;

	BSTreeNode(const T& d, BSTreeNode<T>* p = 0): _data(d), parent(p) {LSon = RSon = NULL;}

	T					_data;
	BSTreeNode<T>* parent;
	BSTreeNode<T>* LSon;
	BSTreeNode<T>* RSon;
};


template <class T>
class BSTree{
   // TODO: design your own class!!
public:
   BSTree() {
	  _size = 0;
      _root = _tail = new BSTreeNode<T>(T());
      _root->LSon = _root->RSon = _root->parent = NULL; // _root is a dummy node
   }
   ~BSTree() { clear(); }
   class iterator {
		friend class BSTree;
	
	public:
      iterator(BSTreeNode<T>* n= 0): _node(n) {}
      iterator(const iterator& i) : _node(i._node) {}
      ~iterator() {} // Should NOT delete _node

      // TODO: implement these overloaded operators
      const T& operator * () const { return _node->_data; }
      T& operator * () { return _node->_data; }
      iterator& operator ++ () {
			if (_node->RSon != 0){
				_node = _node->RSon;
				while(_node->LSon != 0) _node = _node->LSon;
			}
			else{
				if(_node->parent->LSon == _node) _node = _node->parent;
				else {
					_node = _node->parent;
					while(_node->parent->LSon != _node) _node = _node->parent;
					_node = _node->parent;
				}
			}
			return (*this);
		}
      iterator operator ++ (int){ 
			iterator tmp(_node);
			++(*this);
			return tmp; 
	}
      iterator& operator -- () { 
			if (_node->LSon != 0){
				_node = _node->LSon;
				while(_node->RSon != 0) _node = _node->RSon;
			}
			else{
				if(_node->parent->RSon == _node) _node = _node->parent;
				else{
					_node = _node->parent;
					while(_node->parent->RSon != _node) _node = _node->parent;
					_node = _node->parent;
				}
			}
			return (*this);
		}
      iterator operator -- (int){ 
			iterator tmp(_node);
			--(*this);
			return tmp; 
		}
		
      iterator& operator = (const iterator& i) { _node = i._node;	return *(this); }

      bool operator != (const iterator& i) const { return !(i._node == _node); }
      bool operator == (const iterator& i) const { return (i._node == _node); }

   private:
      BSTreeNode<T>* _node;	
	};
   
	// TODO: implement these functions
	bool empty() const { if (_size == 0) return true; return false; }
   size_t size() const { return  _size; }
   iterator begin() const { 
		if(empty()) return iterator(_tail);
		iterator it = _root;
		while(it._node->LSon != 0) it = it._node->LSon;
		return it;
	}
   iterator end() const { return iterator(_tail); }
	void pop_back(){
		iterator it = _root;
		while(it._node->RSon != 0 && it._node->RSon != _tail) it._node = it._node->RSon;
		erase(it);
	}
   void pop_front() { erase(begin());}
   bool erase(iterator pos) {
	   	iterator cpy = pos;
		if (empty() || pos == end()) return false;
		if (_size == 1){
			_tail->parent = 0;
			_root = _tail;
			delete pos._node;
			_size = 0;
			return true;
		}
		iterator a = 0; iterator b = 0;
		if (pos._node->LSon == 0 || pos._node->RSon == 0) b = pos;
		else {
			if (pos._node->RSon == _tail) pos--;
			else pos++;
			b = pos;
		}
		if (b._node->LSon != 0) a._node = b._node->LSon;
		else a._node = b._node->RSon;

		if (a._node != 0) a._node->parent = b._node->parent;
		if (b._node->parent == _tail){
			a._node->parent = _tail;
			b._node->parent = 0;
			b._node->RSon = 0;
			_root = a._node;
			delete b._node;
			b._node = 0;
			--_size;
			return true;
		} 
		else if (b._node == b._node->parent->LSon) b._node->parent->LSon = a._node;
		else b._node->parent->RSon = a._node;
		if (b != cpy) cpy._node->_data = b._node->_data;
		delete b._node;
		b._node = 0;
		--_size;
		return true;
	}
   bool erase(const T& x) {
		iterator del = find(x);
		if (del == end())	 return false;
		else return (erase(del));
	}
	iterator find(const T& x){
      for (iterator match = begin(); match != end(); match++){
         if (*match == x) return match;
      }
      return end();
   	}
   void insert(const T& x) {
		bool done = false; 
		//initialize
		if (empty()){
			BSTreeNode<T>* Initial = new BSTreeNode<T>(x,_tail);
			Initial->RSon = _tail;
			_tail->parent = Initial;
			_root = Initial;	
			done = true;
		}
		//insert next to root
		if (_size == 1){
			BSTreeNode<T>* tmp = new BSTreeNode<T>(x,_root);
			if (_root->_data > x) {
				_root->LSon = tmp;
				tmp->parent = _root;
			}
			else {
				_root->RSon = tmp;
				tmp->parent = _root;
				tmp->RSon = _tail;
				_tail->parent = tmp;
			}
			done = true;
		}	
		//recursive search
		if (!done){
			BSTreeNode<T>* a = 0;	BSTreeNode<T>* b = 0;
			BSTreeNode<T>* tmp = new BSTreeNode<T>(x,NULL);
			a = _root;
			while(a != 0 && a != _tail){
				b = a;
				if (x < a->_data)	a = a->LSon;
				else a = a->RSon;
			}
			if (a == _tail){
				b->RSon = tmp;
				tmp->RSon = _tail;
				tmp->parent = b;
				_tail->parent = tmp;
			}
			else {
				tmp->parent = b;
				if (b == NULL) this->_root = tmp;                    // 下面一組if-else, 把tmp接上BST
    			else if (x < b->_data) b->LSon = tmp;
    			else b->RSon = tmp;
			}
			done = true;
		}
		++_size;
	}

   void clear() { while (_size != 0) erase(begin()); }	// _root is a dummy node
	// delete all nodes except for the dummy node
	void sort() const{}
	void print() const {
		cerr << "_____________________verbose_____________________" << endl;
		for (iterator it = begin(); it != end(); it++){
			cerr << "Data:" << "\t" << it._node->_data << "\t" << "LS:" << "\t" ;
				  if (it._node->LSon == 0) cerr << "NULL" << "\t";
				  else cerr << it._node->LSon << "\t";
				  cerr << "Parent:" << "\t" ; 
				  if (it._node->parent == 0) cerr << "NULL" << "\t";
				  else cerr << it._node->parent->_data << "\t";
				  cerr << "RS:" << "\t" ;
				  if (it._node->RSon == 0) cerr << "NULL" << "\t";
				  else cerr << it._node->RSon->_data << "\t";
				  cerr << endl;
		}
	}
	
private:
   	BSTreeNode<T>*  	_root;  // = dummy node if list is empty
	BSTreeNode<T>*  	_tail;
	size_t				_size;
};

#endif // BST_H
