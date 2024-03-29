/****************************************************************************
  FileName     [ cirMgr.h ]
  PackageName  [ cir ]
  Synopsis     [ Define circuit manager ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_MGR_H
#define CIR_MGR_H

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include "cirGate.h"

using namespace std;

#include "cirDef.h"

extern CirMgr *cirMgr;

// TODO: Define your own data members and member functions
class CirMgr
{
public:
   CirMgr(){}
   ~CirMgr() {}

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { 
     for (size_t i = 0; i < _totalGate.size(); i++){
       if (_totalGate[i]!= NULL){
         if (_totalGate[i]->getID() == gid) return _totalGate[i];
       } 
     }
     return 0;
  }

   // Member functions about circuit construction
   bool readCircuit(const string&);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void writeAag(ostream&) const;

   void DFS(CirGate* gate){
     if(gate->getMark() == false){
       gate->setMark(true);
       if(gate->getTypeStr() != "PI" && gate->getTypeStr() != "CONST" && gate->getTypeStr() != "UNDEF"){
         DFS(gate->getFanin0());
         if (gate->getTypeStr() == "AIG") DFS(gate->getFanin1());
       }
       if(gate->getTypeStr() != "UNDEF")  _dfs.push_back(gate);
     }
   }
   void resetMark(){
     for (size_t i = 0; i < _totalGate.size(); i++) {
       if (_totalGate[i] != NULL){
        _totalGate[i]->setMark(false);
        _totalGate[i]->setReport(false);
       }
     }
   }


  

private:
  vector<CirGate*>      _piGate;
  vector<CirGate*>      _poGate;
  vector<CirGate*>      _AIG;
  vector<CirGate*>      _totalGate;
  vector<CirGate*>      _dfs;
  vector<int>           spec;
};

#endif // CIR_MGR_H
