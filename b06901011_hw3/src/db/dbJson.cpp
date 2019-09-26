/****************************************************************************
  FileName     [ dbJson.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database Json member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iomanip>
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cassert>
#include <climits>
#include <cmath>
#include <string>
#include <algorithm>
#include "dbJson.h"
#include "util.h"
#include <sstream>
#include <regex>



using namespace std;

/*****************************************/
/*          Global Functions             */
/*****************************************/

istream& operator >> (istream& is, DBJson& j)
{
  //cerr << "in istream& operator >> (istream& is, DBJson& j)" << endl;
   // TODO: to read in data from Json file and store them in a DB 
   // - You can assume the input file is with correct JSON file format
   // - NO NEED to handle error file format

   assert(j._obj.empty());

   int pos_start = 0, pos_end = 0, pos_mark = 0, pos_sub = 0;
   string tmp_new,tmp_key,tmp_value;
  
   while(getline(is, tmp_new)){
     if (tmp_new[0] == '{' || tmp_new[0] == '}') continue; 
     else{
    //find key
     pos_start = tmp_new.find("\"",0);
      if (pos_start == -1) continue;

     //cerr << "1:" << pos_start << endl;
     pos_end = tmp_new.find("\"",pos_start+1);
      if (pos_end == -1) continue;

     //cerr << "2:" << pos_end << endl;
     tmp_key.append(tmp_new, pos_start+1, pos_end - pos_start - 1);
     //cerr << tmp_key << endl;
    
    //find value
    pos_mark = tmp_new.find(":",pos_end);
    pos_sub = tmp_new.find("-",pos_mark);
    int value = 0;

    //正數
    if (pos_sub == -1){
      for (size_t i = pos_mark+1; i < tmp_new.size(); i++){
        if (tmp_new[i] == ' ' || tmp_new[i] == ',') continue;
        else tmp_value.append(tmp_new,i,1);
      }
     // cerr << "正數：" << tmp_value << "長度：" << tmp_value.size() << endl;
     for (size_t i = 0; i < tmp_value.size(); i++){
        value += (tmp_value[i]-48)*pow(10,tmp_value.size()-i-1);
     }
    }

    //負數
    else {
      for (size_t i = pos_sub+1; i < tmp_new.size(); i++){
        if (tmp_new[i] == ' ' || tmp_new[i] == ',') continue;
        else tmp_value.append(tmp_new,i,1);
      }
      //cerr << "負數:" << tmp_value << endl;

      for (size_t i = 0; i < tmp_value.size(); i++){
         value += (tmp_value[i]-48)*pow(10,tmp_value.size()-i-1);
      }
      value = 0 - value;
    }
     //store in DB 
     DBJsonElem a(tmp_key,value);
     j._obj.push_back(a);
     tmp_key = "";
     tmp_value = "";
     pos_start = pos_end = pos_mark = pos_sub = 0;
     }
   }
   /*
   cerr << "<< end!!!" << endl;
     for (int i = 0;i<j._obj.size();i++){
       cerr << j[i] << endl;
     }*/

     j.readed = true;

   return is; 
}

ostream&
operator << (ostream& os, const DBJsonElem& j)
{
  //cerr << "in ostream& operator << (ostream& os, const DBJsonElem& j)" << endl;
   os << "\"" << j._key << "\" : " << j._value;
   return os;
}

ostream& operator << (ostream& os, const DBJson& j)
{
   // TODO
   //cerr << "in ostream& operator << (ostream& os, const DBJson& j)" << endl;
   os << "{" <<endl;
   //os << j.size() << endl;
   if (j.size()!=0){
     for(int i = 0; i < (int)j.size()-1; i++){
     os << j._obj[i] << "," << endl;;
   }
   os << j._obj[j.size()-1] << endl;

   }

   
   os << "}" << endl;
   return os;
}

/**********************************************/
/*   Member Functions for class DBJsonElem    */
/**********************************************/
/*****************************************/
/*   Member Functions for class DBJson   */
/*****************************************/
void
DBJson::reset()
{
   // TODO
   //cerr << "in void DBJson::reset()" << endl;
   DBJson j;
   _obj.clear();
   j.readed = false;
}

int 
DBJson::keyrepeat(const DBJsonElem& elm)
{
  int position = -1;
  if ((int)_obj.size() == 0) return position;
  else {
    for (int i = 0; i < (int)_obj.size(); i++){
    if (_obj[i].key() == elm.key()) position = i;
  }
  return position;
  }
}

// return false if key is repeated
bool
DBJson::add(const DBJsonElem& elm)
{
   // TODO
   if (keyrepeat(elm) == -1){
     _obj.push_back(elm);
     return true;
   } 
   else return false;
}

// return NAN if DBJson is empty
float
DBJson::ave() const
{
   // TODO
   if (sum() == 0) return NAN;
   else {
     float ave = (float)sum()/_obj.size();
     return ave;
   }
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::max(size_t& idx) const
{
   // TODO
int maxN = INT_MIN;
    if (_obj.empty()){
        idx = _obj.size();
        return INT_MIN;
    }
    for (size_t i = 0; i < _obj.size(); i++){
        if (_obj[i].value() > maxN){
            maxN = _obj[i].value();
            idx = i;
        }
    }
    return maxN;
}

// If DBJson is empty, set idx to size() and return INT_MIN
int
DBJson::min(size_t& idx) const
{
   // TODO
   int minN = INT_MAX;
   if (_obj.empty()){
     idx = _obj.size();
     return INT_MAX;
   }
   for (size_t i = 0; i < _obj.size(); i++){
       if( minN > _obj[i].value()){
         minN = _obj[i].value();
         idx = i;
       }
     }
     return  minN;
}

void
DBJson::sort(const DBSortKey& s)
{
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

void
DBJson::sort(const DBSortValue& s)
{
   // Sort the data according to the order of columns in 's'
   ::sort(_obj.begin(), _obj.end(), s);
}

// return 0 if empty
int
DBJson::sum() const
{
   // TODO
   //cerr << "input sum" << endl;
   int s = 0;
   if(_obj.empty()) {
     //cerr << "empty" << endl;
    return 0;
   }
   else{
     //cerr << "not empty" << endl;
     for (int i = 0; i < (int)_obj.size(); i++){
       s += _obj[i].value();
     }
   }
   return s;
}
