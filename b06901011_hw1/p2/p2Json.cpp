/****************************************************************************
  FileName     [ p2Json.cpp ]
  PackageName  [ p2 ]
  Synopsis     [ Define member functions of class Json and JsonElem ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2018-present DVLab, GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <string>
#include "p2Json.h"
#include <regex>
#include <fstream>
#include <iomanip>
#include <sstream>
#define ss stringstream


using namespace std;

ostream& operator << (ostream& os, const JsonElem& j){
   return (os << "\"" << j._key << "\" : " << j._value);
}


// Implement member functions of class Row and Table here
bool Json::read(const string& jsonFile){
  // TODO
  ifstream infile(jsonFile);
  if (!infile.is_open()) return 0;
  else{
    string tmp_new;
    while (getline(infile,tmp_new)) {
      JsonElem tmp_ori;
      if(tmp_new >> tmp_ori) _obj.push_back(tmp_ori);
    }
    infile.close();
    return true; 
  } 
}

bool Json::is_empty(){
  if (_obj.empty()){
    cout << "Error: No element found!!" << endl;
    return 1;
  }
  else return 0;
}

void Json::judge_command(string command){
  if(command == "PRINT") Json::print();
  else if (command == "SUM") Json::sum();
  else if (command == "AVE") Json::ave();
  else if (command == "MAX") {
    if (Json::is_empty());
        else{
          cout << "The maximum element is: { " << Json::max() << " }." << endl;
      }
  }
  else if (command == "MIN") {
        if (Json::is_empty());
        else{
          cout << "The minimum element is: { " << Json::min() << " }." << endl;
      }
  }
 
  else if (command == "ADD") {
        regex pattern_key("^[A-Za-z0-9]+$");
        regex pattern_value("^[0-9]+$");
        string key, value;
        cin >> key >> value;
        if (regex_match(key, pattern_key) && regex_match(value, pattern_value)){
          Json::add(key,stoi(value));
        }
      }
  }


void Json::print(){
  it = _obj.begin();
  cout << "{" << endl;
  if (!_obj.empty()){
    while (it != _obj.end()-1) {
      cout << " " << *(it) << "," << endl; 
      it++;
  }
  cout << " " << *it << endl; 
  }
  cout << "}" << endl;
}

void Json::sum(){
  if (Json::is_empty());
  else {
    it = _obj.begin();
    int sum = 0;
    while (it != _obj.end()){
      sum = sum + (*it).getvalue();
      it++;
  }
  cout << "The summation of the values is: "<< sum << "." << endl;
  }
}

void Json::ave(){
  if (Json::is_empty());
  else {
    it = _obj.begin();
    int sum = 0;
    while (it != _obj.end()){
      sum = sum + (*it).getvalue();
      it++;
    }
  double ave = double(sum)/_obj.size();
  cout << "The average of the values is: " << fixed <<  setprecision(1) << ave << "." << endl;
  }
}

JsonElem Json::max(){
  it = _obj.begin();
    int max = -2147483648;
    int count = 0;
    int number = 0;

  while (it != _obj.end()){
    if ((*it).getvalue() > max){
      max = (*it).getvalue();
      number = count;
    }
    count++;
    it++;
  }
  return _obj.at(number);
}

JsonElem Json::min(){
  it = _obj.begin();
  int min = 2147483647;
  int count = 0;
  int number = 0;
  while (it != _obj.end()){
    if ((*it).getvalue() < min){
      min = (*it).getvalue();
      number = count;
    }
    count++;
    it++;
  }
  return _obj.at(number);
}

void Json::add(const string& key_new, const int& value_new){
  JsonElem add(key_new,value_new);
  _obj.push_back(add);
}

bool operator >> (string& in,JsonElem& j){
  smatch smatch;
  if(regex_match(in,smatch,regex("[ \t]*\"([A-Za-z_0-9]*)\"[ \t]*:[ \t]*(-[0-9]*|[0-9]*).*"))){
    j._key = smatch[1];
    ss tmp;
    tmp << smatch[2].str();
    tmp >> j._value;
    return 1;
  }
  else return 0;
}




