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

// TODO: Feel free to define your own classes, variables, or functions.

#include "cirDef.h"


extern CirMgr *cirMgr;
class FECGroup{
  public:
    FECGroup( size_t i){ id = i;}
    ~FECGroup(){}
    void addFEC(CirGate* g, bool b){
      vector<pair<CirGate*,bool> >::iterator it = FECG.begin(), end = FECG.end(); 
      if (FECG.size() == 0){
        FECG.push_back(make_pair(g,b)); return;
      } 
      for (;it!= end;it++){
        if (g->getID() < it->first->getID()){
          FECG.insert(it,make_pair(g,b)); return;
        } 
        if (it == end -1){
          FECG.push_back(make_pair(g,b)); return;
        } 
      }
    }
    void sortEachGroup(){
      vector<pair<CirGate*,bool> >::iterator it = FECG.begin(), end = FECG.end(); 
      if (!it->second){
        for(;it != end; it++){
          if(it->second == false) it->second = true;
          else it->second = false;
        }
      }
    }
    vector<pair<CirGate*,bool>> getGroup(){ return FECG;}
    CirGate* getGate(size_t b) {
      vector<pair<CirGate*,bool> >::iterator it = FECG.begin(), end = FECG.end(); 
      return (it+b)->first;
    }

    bool getInvert(size_t b){
      vector<pair<CirGate*,bool> >::iterator it = FECG.begin(), end = FECG.end(); 
      return (it+b)->second;
    }

    void setGroup(vector<pair<CirGate*,bool>> tmp) { FECG = tmp;}
    void erase(size_t b){
      vector<pair<CirGate*,bool> >::iterator it = FECG.begin(), end = FECG.end(); 
      FECG.erase(it+b);
    }
    
    size_t getFirstMemId(){ return FECG.begin()->first->getID();}
    size_t getFECGroupSize(){ return FECG.size();}
    size_t getFECGroupId(){ return id;}
    size_t setFECGroupId(size_t d){ id = d;}
    void printFECGroup(){
      vector<pair<CirGate*,bool> >::iterator it = FECG.begin(), end = FECG.end(); 
      for(;it!= end;it++){
        cout << " " << ((it->second)?(""):("!")) << it->first->getID();
      }
      cout << endl;
    }
    string printOther(size_t id){
      vector<pair<CirGate*,bool> >::iterator it = FECG.begin();
      vector<pair<CirGate*,bool> >::iterator end = FECG.end(); 
      vector<pair<CirGate*,bool> >::iterator gate;
      string tmp;
      for(;it!= end;it++){
        if(it->first->getID() == id){
          gate = it;
          break;
        } 
      }
      for(it = FECG.begin();it!= end;it++){
        if(it->first->getID()!= id){
          if(tmp == "") {
            tmp += ((it->second == gate->second)?(""):("!"));
            tmp +=  to_string(it->first->getID());
          }
          else {
            tmp = tmp + " " + ((it->second == gate->second)?(""):("!")) + to_string(it->first->getID());
          }
        }
      }
      return tmp;
    }

    size_t id;
    vector<pair<CirGate*,bool>> FECG;
  private:  
    //vector<bool> sign;

};

class CirMgr
{
  friend class FECGroup;
public:
   CirMgr() {strashed = false;}
   ~CirMgr() {} 

   // Access functions
   // return '0' if "gid" corresponds to an undefined gate.
   CirGate* getGate(unsigned gid) const { 
     for (size_t i = 0; i < _totalGate.size(); i++){
      if (_totalGate[i]->getTypeStr() != "NON"){
        if (_totalGate[i]->getID() == gid) return _totalGate[i];
      } 
     }
     return 0;
    }

   // Member functions about circuit construction
   bool readCircuit(const string&);
   void addFECList(FECGroup* g){ _FECList.push_back(g); }
   void DFS(CirGate* gate){
     if(gate->getMark() == false){
       gate->setMark(true);
       if(gate->getTypeStr() != "PI" && gate->getTypeStr() != "CONST" && gate->getTypeStr() != "UNDEF" && gate->getTypeStr() != "NON"){
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

   // Member functions about circuit optimization
   void sweep();
   void optimize();
   void CleanFanout(CirGate*,CirGate*);
   int OptCases(CirGate*);
   void CutAndConnect(CirGate*,CirGate*,bool);
   void Cout(CirGate*,CirGate*, bool);

   // Member functions about simulation
   void randomSim();
   void fileSim(ifstream&);
   void AIGSimCal(CirGate*);
   void POSimCal(CirGate*);
   void CreateFECList();
   void ResetFECList();
   bool Equal(CirGate*,CirGate*);
   void setSimLog(ofstream *logFile) { _simLog = logFile; }
   string Invert(string);
   string ResultToValue(string);
   string GenerateInput();
   vector<FECGroup*> getFECList() {return _FECList;}


   
   void sortFECGroup();

   // Member functions about fraig
   void strash();
   void merge(CirGate*,CirGate*);
   void fraig();
   void CreateCNF(SatSolver*);
   void FraigMerge(CirGate*, CirGate*, bool);
   bool Prove(SatSolver*, CirGate*, FECGroup*);

   // Member functions about circuit reporting
   void printSummary() const;
   void printNetlist() const;
   void printPIs() const;
   void printPOs() const;
   void printFloatGates() const;
   void printFECPairs() const;
   void writeAag(ostream&) const;
   void writeGate(ostream&, CirGate*) const;

   
private:
  ofstream                *_simLog;
  bool                    strashed;
  vector<CirGate*>        _piGate, _poGate, _AIG,  _totalGate, _dfs;
  vector<FECGroup*>       _FECList;  
  vector<int>             spec;
};

#endif // CIR_MGR_H
