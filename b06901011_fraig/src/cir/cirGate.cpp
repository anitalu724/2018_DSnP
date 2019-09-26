/****************************************************************************
  FileName     [ cirGate.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define class CirAigGate member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <sstream>
#include <stdarg.h>
#include <cassert>
#include "cirGate.h"
#include "cirMgr.h"
#include "util.h"

using namespace std;

// TODO: Keep "CirGate::reportGate()", "CirGate::reportFanin()" and
//       "CirGate::reportFanout()" for cir cmds. Feel free to define
//       your own variables and functions.

extern CirMgr *cirMgr;

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
   stringstream rg;
   stringstream fec;
   stringstream val;
	cout << "================================================================================" << endl;
	rg << "= " << getTypeStr() << "("<< getID() << ")";
   if (getName() != "") rg << "\"" << getName() << "\"";
   rg << ", line " << _lineNo;
   fec << "= FECs: ";
   if(getFECid()!= -1){
      fec <<((cirMgr->getFECList())[getFECid()])->printOther(getID());
   }
   val << "= " << "Value: " << getValue();
   cout << setw(49)<< left << rg.str() << endl;
   cout << setw(49)<< left << fec.str() << endl;
   cout << setw(49)<< left << val.str() << endl;
	cout << "================================================================================" << endl;
	cout << right;
}

void
CirGate::reportFanin(int level) const
{
   assert (level >= 0);
   Fanin(const_cast<CirGate*>(this) ,level,0, false);
   cirMgr->resetMark();
}

void
CirGate::reportFanout(int level) const
{
   assert (level >= 0);
   Fanout(const_cast<CirGate*>(this) ,level,0, false);
   cirMgr->resetMark();
}

