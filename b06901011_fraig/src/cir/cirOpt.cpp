/****************************************************************************
  FileName     [ cirSim.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir optimization functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <cassert>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Please keep "CirMgr::sweep()" and "CirMgr::optimize()" for cir cmd.
//       Feel free to define your own variables or functions

/*******************************/
/*   Global variable and enum  */
/******************************
enum OptCase
{
  False    = 0,
  CONST1_X = 1,           // ->1
  CONST0_X = 2,           // ->0
  X_X      = 3,           // ->X
  X_OX     = 4            // ->0
};
*/
/**************************************/
/*   Static varaibles and functions   */
/**************************************/

/**************************************************/
/*   Public member functions about optimization   */
/**************************************************/
// Remove unused gates
// DFS list should NOT be changed
// UNDEF, float and unused list may be changed
void
CirMgr::sweep(){
  for (size_t i = 0; i < _AIG.size(); i++){
    if(_AIG[i]->getIsInDFS() == false){
        CleanFanout(_AIG[i],_AIG[i]->getFanin0());
        CleanFanout(_AIG[i],_AIG[i]->getFanin1());
        _AIG.erase(_AIG.begin() + (i--));
    }
  }

  for (size_t i = 0 ; i < _totalGate.size(); i++){
		if ((_totalGate[i]->getTypeStr() == "AIG")||(_totalGate[i]->getTypeStr() == "UNDEF")){
      if (_totalGate[i]->getTypeStr() == "UNDEF"){
        if (_totalGate[i]->getFanout(0)->getIsInDFS() == false){
          cout<<"Sweeping: "<<_totalGate[i]->getTypeStr()<<"("<<_totalGate[i]->getID()<<") removed..."<<endl;
				  _totalGate[i]->setType(NON);
        }
      }
      else{
        if (_totalGate[i]->getIsInDFS() == false){
          cout<<"Sweeping: "<<_totalGate[i]->getTypeStr()<<"("<<_totalGate[i]->getID()<<") removed..."<<endl;
			  	_totalGate[i]->setType(NON);
        }
      }
    }
	}
}

// Recursively simplifying from POs;
// _dfsList needs to be reconstructed afterwards
// UNDEF gates may be delete if its fanout becomes empty...
void
CirMgr::optimize()
{
  for (size_t i = 0; i < _dfs.size(); i++){
    if (_dfs[i]->getTypeStr() == "AIG"){
      int case_num = OptCases(_dfs[i]);
      if (case_num == 1){
        if (_dfs[i]->getFanin0()->getTypeStr() == "CONST" && _dfs[i]->getInvert0() == true){
          CutAndConnect(_dfs[i], _dfs[i]->getFanin1(),_dfs[i]->getInvert1());
          Cout(_dfs[i]->getFanin1(),_dfs[i],_dfs[i]->getInvert1());
        }
        else {
          CutAndConnect(_dfs[i],_dfs[i]->getFanin0(),_dfs[i]->getInvert0());
          Cout(_dfs[i]->getFanin0(),_dfs[i],_dfs[i]->getInvert0());
        }
      }
      else if (case_num == 2){
        if (_dfs[i]->getFanin0()->getTypeStr() == "CONST" && _dfs[i]->getInvert0() == false){
          CutAndConnect(_dfs[i],_dfs[i]->getFanin0(),_dfs[i]->getInvert0());
          Cout(_dfs[i]->getFanin0(),_dfs[i],false);
        }
        else {
          CutAndConnect(_dfs[i],_dfs[i]->getFanin1(),_dfs[i]->getInvert1());
          Cout(_dfs[i]->getFanin1(),_dfs[i],false);
        }
      }
      else if (case_num == 3){
        CutAndConnect(_dfs[i],_dfs[i]->getFanin0(), _dfs[i]->getInvert0());
        Cout(_dfs[i]->getFanin0(),_dfs[i],_dfs[i]->getInvert0());
      }
      else if (case_num == 4){
        CutAndConnect(_dfs[i],_totalGate[0], false);
        Cout(_totalGate[0],_dfs[i],false);
      }
      if (case_num != 0){
        CleanFanout(_dfs[i],_dfs[i]->getFanin0());
        CleanFanout(_dfs[i],_dfs[i]->getFanin1());
        _dfs[i]->setType(NON);
      }
    }
  }

  for (size_t i = 0; i < _AIG.size(); i++){
    if (_AIG[i]->getTypeStr() == "NON") _AIG.erase(_AIG.begin()+(i--));
    
  } 
  //____________________________reset DFSList____________________________//
  for (size_t i = 0; i < _totalGate.size(); i++) _totalGate[i]->setIsInDFS(false);
  _dfs.clear();
  for (size_t i = 0; i < _poGate.size(); i++) DFS(_poGate[i]);
  for (size_t i = 0; i < _dfs.size(); i++)  _dfs[i]->setIsInDFS(true);
  for (size_t i = 0; i < _totalGate.size(); i++) _totalGate[i]->setMark(false);
}

void CirMgr::CutAndConnect(CirGate* gate, CirGate* NewIn, bool b){
  for (size_t i = 0; i < gate->fanout.size(); i++){
    if(gate->fanout[i]->getFanin0() == gate){
      bool g = !(gate->fanout[i]->getInvert0() == b);
      gate->fanout[i]->setFanin0(NewIn);
      gate->fanout[i]->setInvert0(g);
    } 
    else{
      bool g = !(gate->fanout[i]->getInvert1() == b);
      gate->fanout[i]->setFanin1(NewIn);
      gate->fanout[i]->setInvert1(g);
    } 
    NewIn->addFanout(gate->fanout[i]);
  }
}

void CirMgr::Cout(CirGate* first, CirGate* sec, bool b){
  cout << "Simplifying: " << first->getID() << " merging " << ((b) ? "!" : "" )<< sec->getID() << "..." << endl;
}

/***************************************************/
/*   Private member functions about optimization   */
/***************************************************/
void CirMgr::CleanFanout(CirGate* out, CirGate* in){
  for (size_t i = 0; i < in->getFanoutSize(); i++){
    if (in->getFanout(i) == out){
      (in->fanout).erase((in->fanout).begin() + i);
      break;
    } 
  }
}


int CirMgr::OptCases(CirGate* gate){
  if (gate->getFanin0()->getTypeStr() == "CONST" || gate->getFanin1()->getTypeStr() == "CONST"){
    if ((gate->getFanin0()->getTypeStr() == "CONST" && gate->getInvert0() == true) || (gate->getFanin1()->getTypeStr() == "CONST" && gate->getInvert1() == true)) return 1;
    if ((gate->getFanin0()->getTypeStr() == "CONST" && gate->getInvert0() == false) || (gate->getFanin1()->getTypeStr() == "CONST" && gate->getInvert1() == false)) return 2;
  }
  else if (gate->getFanin0() == gate->getFanin1()){
    if (gate->getInvert0() == gate->getInvert1()) return 3;
    else return 4;
  }
  else return 0;
}