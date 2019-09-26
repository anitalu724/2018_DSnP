/****************************************************************************
  FileName     [ p2Json.h]
  PackageName  [ p2 ]
  Synopsis     [ Header file for class Json JsonElem ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2018-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#ifndef P2_JSON_H
#define P2_JSON_H

#include <vector>
#include <string>
#include <unordered_set>

using namespace std;

class JsonElem
{
public:
   // TODO: define constructor & member functions on your own
   JsonElem() {}
   JsonElem(const string& k, int v): _key(k), _value(v) {}
   friend ostream& operator << (ostream&, const JsonElem&);
   friend bool operator >> (string& in ,JsonElem&);
   friend class Json;

private:
   string  _key;   // DO NOT change this definition. Use it to store key.
   int     _value; // DO NOT change this definition. Use it to store value.

   int getvalue() const{ return _value; }
};

class Json
{
public:
   Json(){}
   // TODO: define constructor & member functions on your owntr
   bool read(const string&);
   bool is_empty();
   void judge_command(string command);

   void print();
   void sum();
   void ave();
   JsonElem max();
   JsonElem min();
   void add(const string&, const int&);
private:
   vector<JsonElem>       _obj;  // DO NOT change this definition.
                                 // Use it to store JSON elements.
   vector<JsonElem>:: iterator it;
};

#endif // P2_TABLE_H
