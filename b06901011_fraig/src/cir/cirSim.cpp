/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir simulation functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <fstream>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"
#include <string>
#include <vector>
#include <random>

using namespace std;

// TODO: Keep "CirMgr::randimSim()" and "CirMgr::fileSim()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/************************************************/
/*   Public member functions about Simulation   */
/************************************************/
void
CirMgr::randomSim()
{
  string result[_piGate.size()];
  string out[_poGate.size()];
  size_t MAX_FAIL;
  if(_piGate.size()<10) MAX_FAIL = 2;
  else if(_piGate.size()<50) MAX_FAIL = 10;
  else MAX_FAIL = 20;
   
  for (size_t m = 0; m < MAX_FAIL; m++){
    for(size_t i = 0; i < _piGate.size(); i++){
      result[i] = GenerateInput();
      _piGate[i]->setresult(result[i]);

    }
    string con;
    for (size_t i = 0; i < result[0].length(); i++) con += '0';
    _totalGate[0]->setresult(con);

    for (size_t i = 0; i < _dfs.size(); i++){
      if (_dfs[i]->getTypeStr() == "AIG") AIGSimCal(_dfs[i]);
      else if (_dfs[i] ->getTypeStr() == "PO") POSimCal(_dfs[i]);
    }
    CreateFECList();
    if (m != MAX_FAIL-1){
      for (size_t i = 0; i < _piGate.size(); i++) result[i] = "";
      for (size_t i = 0; i < _dfs.size(); i++) _dfs[i]->setresult("");
    }
    sortFECGroup();
    //cerr << m << endl;
  }
  for (size_t i = 0; i < _poGate.size(); i++) out[i] == _poGate[i]->getResult();
  cout << result[0].length()*MAX_FAIL << " patterns simulated." << endl;
  
  /**********************************************output logfile**********************************************/
  ostream* output = 0;
  if (_simLog){
    output = _simLog;
    for(size_t i = 0; i < result[0].length(); i++){
      for(size_t j = 0; j < _piGate.size(); j++) *output << result[j][i];
      *output << " ";
      for(size_t j = 0; j < _poGate.size(); j++) *output << out[j][i];
      *output << endl;
    }
  }
  else output = &cout;
  /**********************************************output logfile**********************************************/
  for (size_t i = 0; i < _dfs.size(); i++){
    _dfs[i]->setValue(ResultToValue(_dfs[i]->getResult()));
  }
  _totalGate[0]->setValue(ResultToValue(_totalGate[0]->getResult()));
}

void
CirMgr::fileSim(ifstream& patternFile)
{
  string pattern[_piGate.size()];
  string result[_piGate.size()];
  string out[_poGate.size()];
  string str;
  while(getline(patternFile, str)){
    for (size_t i = 0; i < str.length(); i++){
      if (str[i] == ' ') {
        str = str.substr(str.find_first_not_of(' '), i - str.find_first_not_of(' '));
        break;
      }
    }
    if (str.length() == 0) continue;

    /**********************************************testing pattern length**********************************************/
    if (str.length() != _piGate.size()){
      cerr <<  endl << "Error: Pattern(" << str << ") length(" << str.length() << ") does not match the number of inputs("
           << _piGate.size() << ") in a circuit!!\n\n0 patterns simulated."  << endl;
      return;
    }
    /**************************************************testing non-0/1**************************************************/
    for (size_t i = 0; i < str.length(); i++){
      if (str[i] != '0' && str[i] != '1'){
        cerr << endl << "Error: Pattern(" << str << ") contains a non-0/1 character('" << str[i] << "')." << endl << endl;
        cerr << "0 patterns simulated." << endl;
        return;
      }
    }
    for (size_t i = 0; i < _piGate.size(); i++){
      pattern[i] += str[i];
    } 
  }

  size_t times = 0;
  (pattern[0].length()%64 == 0)? (times = pattern[0].length()/64):(times = pattern[0].length()/64+1);
  for (size_t t = 0; t < times; t++){
    for (size_t i = 0; i < _piGate.size(); i++){
      if (t == times-1) {
        result[i] = pattern[i].substr(t*64, pattern[i].length()-t*64);
        while(result[i].length() < 64) result[i] += '0';
      }
      else result[i] += pattern[i].substr(t*64,64);
      _piGate[i]->setresult(result[i]);
    }
    string con;
    for (size_t i = 0; i < result[0].length(); i++) con += '0';
    _totalGate[0]->setresult(con);

    for (size_t i = 0; i < _dfs.size(); i++){
      if (_dfs[i]->getTypeStr() == "AIG") AIGSimCal(_dfs[i]);
      else if (_dfs[i] ->getTypeStr() == "PO") POSimCal(_dfs[i]);
    }
    CreateFECList();
    for (size_t i = 0; i < _dfs.size(); i++)  if (t != times-1) _dfs[i]->setresult("");
    if (t != times-1){
      for (size_t i = 0; i < _piGate.size(); i++) result[i] = "";
      _totalGate[0]->setresult("");
    }
    sortFECGroup();
    printf("Total #FEC Group = %d\n",_FECList.size());
    fflush(stdout);
  }
  for (size_t i = 0; i < _poGate.size(); i++) out[i] == _poGate[i]->getResult();
  cout << pattern[0].length() << " patterns simulated." << endl;

/**********************************************output logfile**********************************************/
  ostream* output = 0;
  if (_simLog){
    output = _simLog;
    for(size_t i = 0; i < pattern[0].length(); i++){
      for(size_t j = 0; j < _piGate.size(); j++) *output << pattern[j][i];
      *output << " ";
      for(size_t j = 0; j < _poGate.size(); j++) *output << out[j][i];
      *output << endl;
    }
  }
  else output = &cout;
/**********************************************output logfile**********************************************/
  
  for (size_t i = 0; i < _dfs.size(); i++){
    _dfs[i]->setValue(ResultToValue(_dfs[i]->getResult()));
  }
  _totalGate[0]->setValue(ResultToValue(_totalGate[0]->getResult()));
}
  

void CirMgr::AIGSimCal(CirGate* gate){
  string tmp; string in0, in1;
  (gate->getInvert0())?(in0 = Invert(gate->getFanin0()->getResult())):(in0 = gate->getFanin0()->getResult());
  (gate->getInvert1())?(in1 = Invert(gate->getFanin1()->getResult())):(in1 = gate->getFanin1()->getResult());
  for (size_t i = 0; i < 64; i++) (in0[i] == '1' && in1[i] == '1')?(tmp += '1'):(tmp += '0');
  gate->setresult(tmp);
  //gate->setTotalResult(tmp);
}

void CirMgr::POSimCal(CirGate* gate){
  if (!gate->getInvert0()){
    gate->setresult(gate->getFanin0()->getResult());
    //gate->setTotalResult(gate->getFanin0()->getresult());
  } 
  else{
    gate->setresult(Invert(gate->getFanin0()->getResult()));
    //gate->setTotalResult(Invert(gate->getFanin0()->getresult()));
  } 
}

string CirMgr::Invert(string str){
  string tmp;
  for (size_t i = 0; i < str.length(); i++){
    (str[i] == '0' )?( tmp += '1'):(tmp += '0');
  }
  return tmp;
}

void CirMgr::CreateFECList(){
  //cerr << t << endl;
  bool const0;
  if(_FECList.empty()){
    size_t counter = 0;
    ResetFECList();
    for (size_t i = 0; i < _dfs.size(); i++) _dfs[i]->isInFECList = false;
    if (_totalGate[0]->isInFECList == false && _totalGate[0]->isInDFS == false){
      _dfs.insert(_dfs.begin(), _totalGate[0]);
      const0 = true;
    }  
    for(size_t i = 0; i < _dfs.size(); i++){
      if ((!_dfs[i]->isInFECList) && (_dfs[i]->getTypeStr() == "AIG" || _dfs[i]->getTypeStr() == "CONST")){
        bool first = true; size_t remind = counter;
        for (size_t j = i+1; j < _dfs.size(); j++){
          if (Equal(_dfs[i], _dfs[j]) && _dfs[j]->getTypeStr() != "PO" && _dfs[j]->getTypeStr() != "PI"){
            if (first){
              FECGroup* f = new FECGroup(counter);
              _dfs[i]->isInFECList = true;
              _dfs[i]->setFECid(counter);
              f->addFEC(_dfs[i],true);
              addFECList(f);
              first = false;
              counter++;
            }
            (_dfs[i]->getResult() == _dfs[j]->getResult())?(_FECList[counter-1]->addFEC(_dfs[j],true)):(_FECList[counter-1]->addFEC(_dfs[j],false));
            _dfs[j]->isInFECList = true;
		  			_dfs[j]->setFECid(counter-1);
          }
        }
        if(remind != counter) _FECList[remind]->sortEachGroup();
      }
    }
    //cerr << _FECList.size() << endl;
  }
  else{
    for(size_t i = 0; i < _FECList.size(); i++){
      vector<CirGate*> rest; rest.clear();
      for(size_t k = 1; k < _FECList[i]->getFECGroupSize();){
        if(!Equal(_FECList[i]->getGate(0), _FECList[i]->getGate(k))){
          rest.push_back(_FECList[i]->getGate(k));
          _FECList[i]->getGate(k)->isInFECList = false;
          _FECList[i]->erase(k);
        }
        else k++;
      }
      if(rest.size() >= 2){
        for(size_t j = 0; j < rest.size(); j++){
          if(!rest[j]->isInFECList){
            bool first = true; size_t remind = _FECList.size();
            for(size_t k = j+1; k < rest.size(); k++){
              if (Equal(rest[j], rest[k])){
                if(first){
                  FECGroup* f = new FECGroup(_FECList.size());
                  rest[j]->isInFECList = true;
                  rest[j]->setFECid(_FECList.size());
                  f->addFEC(rest[j],true);
                  addFECList(f);
                  first = false;
                }
                (rest[j]->getResult() == rest[k]->getResult())?(_FECList[_FECList.size()-1]->addFEC(rest[k],true)):(_FECList[_FECList.size()-1]->addFEC(rest[k],false));
                rest[k]->isInFECList = true;
		  			    rest[k]->setFECid(_FECList.size()-1);
              }
            }
        if(remind != _FECList.size()) _FECList[remind]->sortEachGroup();
          }
        
        }
      }
    }
  }
  for(size_t i = 0; i < _FECList.size(); i++) {
    if (_FECList[i]->getFECGroupSize() < 2){
      for (size_t j = 0; j < _FECList[_FECList.size()-1]->getFECGroupSize(); j++) 
        _FECList[_FECList.size()-1]->getGate(j)->setFECid(i);
      swap(_FECList[i], _FECList[_FECList.size()-1]);
      for (size_t j = 0; j < _FECList[_FECList.size()-1]->getFECGroupSize(); j++){
        _FECList[_FECList.size()-1]->getGate(j)->isInFECList = false;
        _FECList[_FECList.size()-1]->getGate(j)->setFECid(-1);
      } 
      _FECList.pop_back();
    }
  }
  if (const0) _dfs.erase(_dfs.begin());
  for(size_t i = 0; i < _totalGate.size(); i++) {
    if(_totalGate[i]->isInFECList == false) _totalGate[i]->setFECid(-1);
  }
}

void CirMgr::sortFECGroup(){
  for(size_t i = 0; i < _FECList.size(); i++){
    for(size_t j = i; j < _FECList.size(); j++){
      if(_FECList[i]->getFirstMemId() > _FECList[j]->getFirstMemId()){
        vector<pair<CirGate*,bool>> tmp = _FECList[i]->getGroup();
        //size_t tmp_id = i;
        for(size_t k = 0; k < _FECList[i]->getFECGroupSize(); k++) _FECList[i]->getGate(k)->setFECid(j);
        _FECList[i]->setGroup(_FECList[j]->getGroup());
        for(size_t k = 0; k < _FECList[j]->getFECGroupSize(); k++) _FECList[j]->getGate(k)->setFECid(i);
        _FECList[j]->setGroup(tmp);
      }
    }
  }
}



bool CirMgr::Equal(CirGate* first, CirGate* second){
  return (first->getResult() == second->getResult() || first->getResult() == Invert(second->getResult()));
}

string CirMgr::ResultToValue(string b){
  string v;
  reverse(b.begin(),b.end());
  for (size_t i = 0; i < 8; i++){
    v += b.substr(i*8,8);
    if (i != 7) v += '_';
  }
  return v;
}

string CirMgr::GenerateInput(){
  string tmp;
  random_device rd;
  for(size_t i = 0; i < 64; i++){
    mt19937_64 generator(rd());
    uniform_int_distribution<int> unif(0, 1);
    int x = unif(generator);
    tmp += to_string(x);
  }
  return tmp;
}


/*************************************************/
/*   Private member functions about Simulation   */
/*************************************************/
void CirMgr::ResetFECList(){ 
   if(!_FECList.empty()){
     for(size_t i = 0; i < _FECList.size(); i++){
       for(size_t j = 0; j < _FECList[i]->getFECGroupSize(); j++) _FECList[i]->getGate(j)->setFECid(-1);
     }
   }
   _FECList.clear();
}
