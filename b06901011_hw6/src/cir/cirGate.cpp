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

extern CirMgr *cirMgr;

// TODO: Implement memeber functions for class(es) in cirGate.h

/**************************************/
/*   class CirGate member functions   */
/**************************************/
void
CirGate::reportGate() const
{
	stringstream rg;
	cout << "==================================================" << endl;
	rg << "= " << getTypeStr() << "("<< getID() << ")";
   if (getName() != "") rg << "\"" << getName() << "\"";
   rg << ", line " << _lineNo;
	cout << setw(49)<< left << rg.str() << "=" << endl;
	cout << "==================================================" << endl;
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


