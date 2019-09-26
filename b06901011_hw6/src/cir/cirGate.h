/****************************************************************************
  FileName     [ cirGate.h ]
  PackageName  [ cir ]
  Synopsis     [ Define basic gate data structures ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#ifndef CIR_GATE_H
#define CIR_GATE_H

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include "cirDef.h"
//#define COMPARATOR(code) [](auto && l, auto && r) -> bool { return code ; }

using namespace std;

class CirGate;

//------------------------------------------------------------------------
//   Define classes
//------------------------------------------------------------------------
// TODO: Define your own data members and member functions, or classes
class CirGate
{
public:
   CirGate(unsigned i = 0, unsigned l = 0, CirGate* f0 = 0, bool in0 = false, CirGate* f1 = 0, bool in1 = false,unsigned g = 0) {
     _id = i;
     _lineNo = l;
     fanin[0] = f0;
     fanin[1] = f1;
     invert0 = in0;
     invert1 = in1;
     isMarked = false;
     isFloating = false;
     _gatetype = g;
     isReported = false;
   }
   virtual ~CirGate() {}

   // Basic access methods
   string getTypeStr() const {
     if(_gatetype == 0) return "UNDEF";
     else if(_gatetype == 1) return "PI";
     else if(_gatetype == 2) return "PO";
     else if(_gatetype == 3) return "AIG";
     else if(_gatetype == 4) return "CONST";
     else return "";
  }
   unsigned getLineNo() const { return _lineNo; }
   unsigned getID() const { return _id; }
   bool getInvert0() const{ return invert0; }
   bool getInvert1() const{ return invert1; }
   string getName() const { return _name; }
   bool getMark() const { return isMarked; }
   CirGate* getFanin0() const { return fanin[0];}
   CirGate* getFanin1() const { return fanin[1];}
   CirGate* getFanout(size_t i) const { return fanout[i]; }
   size_t getFanoutSize() const {return fanout.size();}
   
   
   void addFanout(CirGate* tmp){ fanout.push_back(tmp); }
   
   void sortFanout(){
     if (fanout.size() >= 2) {
       for (size_t i = 0; i < fanout.size(); i++){
       for (size_t j = i+1; j < fanout.size(); j++){
         if (fanout[i]->getID() > fanout[j]->getID()) swap(fanout[i],fanout[j]);
       }
     } 
     }  
   }
   
   void setFanin0(CirGate* tmp){ fanin[0] = tmp; }
   void setFanin1(CirGate* tmp){ fanin[1] = tmp; }
   void setName(string tmp){ _name = tmp; }
   void setMark(bool b){ isMarked = b; }
   void setReport(bool b){ isReported = b; }

   // Printing functions
   virtual void printGate() const = 0;
   void reportGate() const;
   void reportFanin(int level) const;
   void reportFanout(int level) const;

   void Fanin(CirGate* gate, int level, int space, bool invert) const{
     for(int i = 0; i < space; i++) cout << "  ";
     if(gate->getTypeStr() == "PO"){
        cout << gate->getTypeStr() << " " << gate->getID() << endl; level--; space++; gate->setMark(true);
        if(level >= 0) Fanin(gate->getFanin0(), level, space, gate->getInvert0());
     }
     else if (gate->getTypeStr() == "AIG"){
        if (invert == true) cout << "!";
        cout << gate->getTypeStr() << " " << gate->getID(); level--; space++;
        if (gate->getMark() == true && level >= 0 && gate->isReported == true) {cout << " (*)"; level = -1;}
        gate->setMark(true);
        cout << endl;
        if (level >= 0){
          Fanin(gate->getFanin0(), level, space, gate->getInvert0());
          Fanin(gate->getFanin1(), level, space, gate->getInvert1());
          gate->isReported = true;
        }
     }
     else if (gate->getTypeStr() == "PI" ||gate->getTypeStr() == "UNDEF"){
       if (invert == true) cout << "!";
       cout << gate->getTypeStr() << " " << gate->getID() << endl; level=0; space++; gate->setMark(true);
     }
     else if (gate->getTypeStr() == "CONST"){
       if (invert == true) cout << "!";
       cout << gate->getTypeStr() << " " << gate->getID() << endl; level=0; space++; gate->setMark(true);
     }
   };

   void Fanout(CirGate* gate, int level, int space, bool invert) const{
     for(int i = 0; i < space; i++) cout << "  ";
     if(gate->getTypeStr() == "PI" || gate->getTypeStr() == "CONST" || gate->getTypeStr() == "UNDEF"){
        cout << gate->getTypeStr() << " " << gate->getID() << endl; level--; space++; gate->setMark(true);
        if(level >= 0) {
          for(size_t i = 0; i < gate->fanout.size(); i++){
            bool in;
            if (gate->fanout[i]->getFanin0() == gate) in = gate->fanout[i]->getInvert0();
            else in = gate->fanout[i]->getInvert1();
            Fanout(gate->fanout[i], level, space, in);
          }
        }
     }
     else if (gate->getTypeStr() == "AIG"){
       if (invert == true) cout << "!";
       cout << gate->getTypeStr() << " " << gate->getID(); level--; space++; 
       if (gate->getMark() == true && level >= 0 && gate->isReported == true) {cout << " (*)"; level = -1;}
       gate->setMark(true);
       cout << endl;
       if (level >= 0){
         for(size_t i = 0; i < gate->fanout.size(); i++){
            bool in;
            if (gate->fanout[i]->getFanin0() == gate) in = gate->fanout[i]->getInvert0();
            else in = gate->fanout[i]->getInvert1();
            Fanout(gate->fanout[i], level, space, in);
            gate->isReported = true;
        }
       }
     }
     else if (gate->getTypeStr() == "PO"){
       if (invert == true) cout << "!";
       cout << gate->getTypeStr() << " " << gate->getID() << endl; level--; space++; gate->setMark(true);
     }
   }

   bool checkFanoutMark(CirGate* gate) const{
     for (size_t i = 0; i < gate->fanout.size(); i++ ){
       if (gate->fanout[i]->getMark() == false) return false;
     } 
     return true;  
   }

   bool checkFaninMark(CirGate* gate) const{
       if (gate->fanin[0]->getMark() == false) return false;
       if (gate->fanin[1]->getMark() == false) return false;
     return true;  
   }

private:
  unsigned              _id;
  unsigned              _lineNo;
  bool                  invert0;
  bool                  invert1;
  bool                  isReported;
  CirGate*              fanin[2];
  vector<CirGate*>      fanout;
  unsigned              _gatetype;
  bool                  isMarked;
  bool                  isFloating;
  string                _name;
protected:

};
  

class PIGate : public CirGate{
  public:
    PIGate(unsigned i = 0, unsigned l = 0, GateType g = PI_GATE) : CirGate(i,l,NULL,false,NULL,false,g){};
    ~PIGate(){};
    void printGate() const{}
};
class POGate : public CirGate{
  public:
    POGate(unsigned i = 0, unsigned l = 0, bool in0 = 0, CirGate* f0 = 0, GateType g = PO_GATE) : CirGate(i,l,f0,in0,NULL,false,g){};
    ~POGate(){};
    void printGate() const{}
};
class AIG : public CirGate{
  public:
    AIG(unsigned i = 0, unsigned l = 0, CirGate* f0 = 0, bool in0 = 0, CirGate* f1 = 0, bool in1 = 0, GateType g = AIG_GATE) : CirGate(i,l,f0,in0,f1,in1,g){
    };
    ~AIG(){};
    void printGate() const{}
};

class CONST_0 : public CirGate{
  public:
    CONST_0(unsigned i = 0, unsigned l = 0, GateType g = CONST_GATE) : CirGate(i,l,NULL,false,NULL,false,g){};
    ~CONST_0(){};
    void printGate() const{}
};




#endif // CIR_GATE_H
