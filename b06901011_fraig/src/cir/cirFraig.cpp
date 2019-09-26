/****************************************************************************
  FileName     [ cirFraig.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir FRAIG functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2012-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "sat.h"
#include "myHashMap.h"
#include "util.h"
#include <unordered_map>

using namespace std;

// TODO: Please keep "CirMgr::strash()" and "CirMgr::fraig()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/*******************************/

/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/*******************************************/
/*   Public member functions about fraig   */
/*******************************************/
// _floatList may be changed.
// _unusedList and _undefList won't be changed
void
CirMgr::strash()
{
  unordered_map< string, vector<CirGate*>> myMap;
  vector<CirGate*> match;
  if (strashed) cerr << "Error: circuit has been strashed!!" << endl;
  for (size_t i = 0; i < _dfs.size(); i++){
    if (_dfs[i]->getTypeStr() == "AIG"){
      HashKey k = HashKey(_dfs[i]->getFanin0()->getID(),_dfs[i]->getFanin1()->getID(),_dfs[i]->getInvert0(), _dfs[i]->getInvert1());
      if (myMap[k()].empty()){
        myMap[k()].push_back(_dfs[i]);
      } 
      else{
        cout << "Strashing: " << myMap[k()][0]->getID() << " merging " << _dfs[i]->getID() << "..." << endl;
        CleanFanout(_dfs[i],_dfs[i]->getFanin0());
        CleanFanout(_dfs[i],_dfs[i]->getFanin1());
        merge(_dfs[i],myMap[k()][0]);
        _dfs[i]->setType(NON);
      }
    }
  }
  for (size_t i = 0; i < _AIG.size(); i++){
    if (_AIG[i]->getTypeStr() == "NON") _AIG.erase(_AIG.begin()+(i--));
  } 
  for (size_t i = 0; i < _totalGate.size(); i++) _totalGate[i]->setIsInDFS(false);
  _dfs.clear();
  for (size_t i = 0; i < _poGate.size(); i++) DFS(_poGate[i]);
  for (size_t i = 0; i < _dfs.size(); i++)  _dfs[i]->setIsInDFS(true);
  for (size_t i = 0; i < _totalGate.size(); i++)  _totalGate[i]->setMark(false);
}


void
CirMgr::fraig()
{
  SatSolver ss; ss.initialize(); int SAT = 0; int UNSAT = 0;
  
  CreateCNF(&ss);
  /***************** SOLVE *********************/
  for(size_t i = 0; i < _dfs.size(); i++){
    if(_dfs[i]->isInFECList && _dfs[i]->getTypeStr()!= "NON" && _dfs[i]->getID() != 0){
        Prove(&ss,_dfs[i],_FECList[_dfs[i]->getFECid()])? SAT++ : UNSAT++;   
    }
  }
  /********************************************/
  //clean unused gate
	for (size_t i = 0 ; i < _AIG.size() ; i++){
		if (_AIG[i]->getTypeStr() == "NON") _AIG.erase(_AIG.begin() + (i--));
	}
	for (size_t i = 0 ; i < _totalGate.size() ; i++)  _totalGate[i]->isInDFS = false;

  //rebuild DFS
  _dfs.clear();
  for (size_t i = 0; i < _poGate.size(); i++)  DFS(_poGate[i]);
  for (size_t i = 0; i < _dfs.size(); i++)  _dfs[i]->setIsInDFS(true);
  for (size_t i = 0; i < _totalGate.size(); i++)  if(_totalGate[i]->getTypeStr() != "NON") _totalGate[i]->setMark(false);

  ResetFECList();
  if (UNSAT > 0 )cerr << "Updating by UNSAT... Total #FEC Group = " << _FECList.size() << endl;
  if (SAT > 0 )cerr << "Updating by SAT... Total #FEC Group = " << _FECList.size() << endl;

	strashed = false;
  strash();

}

/********************************************/
/*   Private member functions about fraig   */
/********************************************/
void CirMgr::merge(CirGate* gate, CirGate* NewIn){
  for (size_t i = 0; i < gate->getFanoutSize(); i++){
    CirGate* out = gate->getFanout(i);
    if (out->getFanin0() == gate) out->setFanin0(NewIn);
    else out->setFanin1(NewIn);
  }
}

void CirMgr::CreateCNF(SatSolver *ss){
  _totalGate[0]->setVar(0);
  unordered_map<Var, vector<CirGate*>> AIGvar;
  for(size_t i = 0; i < _dfs.size(); i++){
    if(_dfs[i]->getTypeStr() == "PI" || _dfs[i]->getTypeStr() == "AIG") _dfs[i]->setVar(ss->newVar());
    if(_dfs[i]->getTypeStr() == "AIG") AIGvar[_dfs[i]->getVar()].push_back(_dfs[i]);
  }
  for(size_t i = 0; i < _poGate.size(); i++) _poGate[i]->setVar(_poGate[i]->getFanin0()->getVar());
  //clause
  for(size_t i = 0; i < _AIG.size(); i++){
    if(_AIG[i]->isInDFS)
      ss->addAigCNF(_AIG[i]->getVar(),_AIG[i]->getFanin0()->getVar(),_AIG[i]->getInvert0(),_AIG[i]->getFanin1()->getVar(),_AIG[i]->getInvert1());
  }
  return;
}

void CirMgr::FraigMerge(CirGate* a, CirGate* b, bool c){
  cerr << "Fraig: " << a->getID()<<" merging "<<((a->getResult()!= b->getResult())?"!":"") << b->getID()<<"..."<<endl;
  b->setType(NON);
  CutAndConnect(b,a,!c);
  CleanFanout(b, b->getFanin0());
  CleanFanout(b, b->getFanin1());
}

bool CirMgr::Prove(SatSolver* ss, CirGate* gate, FECGroup* list){
  bool result; bool invert;
  if(list->getGate(0)->getID() == 0){               //const0 special case
    ss->assumeRelease();
    ss->assumeProperty(_totalGate[0]->getVar(), false);
    invert = gate->getResult() == Invert(_totalGate[0]->getResult()); 
    cout << "\nProving " << gate->getID() << " = " << (invert?"0":"1") << "...";
    ss->assumeProperty(gate->getVar(),!invert);
    result = ss->assumpSolve();
    cout << (result?"SAT!!":"UNSAT!!") << endl;
    if(result){                                         //SAT: means not FEC anymore    
    } 
    else{                                               //UNSAT: mergeing
      FraigMerge(_totalGate[0],gate,!invert);                                 
    } 
  }
  else{                                             //general cases
    size_t gate_id;
    for(size_t i = 0; i < list->getFECGroupSize();){
      for(size_t j = 0; j < list->getFECGroupSize(); j++){
        if(list->getGate(j)->getID() == gate->getID()) gate_id = j;
      }
      ss->assumeRelease();
      ss->assumeProperty(_totalGate[0]->getVar(), false);
      if(list->getGate(i)->getID() != gate->getID()){
        cout << "\nProving (" << (list->getInvert(gate_id)?"":"!") << gate->getID() << ", " ;
        cout << (list->getInvert(i)?"":"!") << list->getGate(i)->getID() << ")...";
        Var var = ss->newVar();
        invert = list->getInvert(gate_id) == list->getInvert(i);
        ss->addXorCNF(var,gate->getVar(),false, list->getGate(i)->getVar(), false);
        ss->assumeProperty(var,invert);
        result = ss->assumpSolve();
        cout << (result?"SAT!!":"UNSAT!!") << endl;
        if(result){                                         //SAT: means not FEC anymore
          gate->setFECid(-1);
          list->erase(gate_id);
          i = list->getFECGroupSize();
        } 
        else{                                               //UNSAT: mergeing
          list->getGate(i)->setFECid(-1);
          CirGate* tmp = list->getGate(i);
          list->erase(i);
          FraigMerge(gate,tmp,invert);                                 
        } 
      }
      else i++;
    }
  }
  return result;
}