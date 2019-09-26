/****************************************************************************
  FileName     [ cirMgr.cpp ]
  PackageName  [ cir ]
  Synopsis     [ Define cir manager functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2008-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/

#include <iostream>
#include <iomanip>
#include <cstdio>
#include <ctype.h>
#include <cassert>
#include <cstring>
#include "cirMgr.h"
#include "cirGate.h"
#include "util.h"

using namespace std;

// TODO: Implement memeber functions for class CirMgr

/*******************************/
/*   Global variable and enum  */
/*******************************/
CirMgr* cirMgr = 0;

enum CirParseError {
   EXTRA_SPACE,
   MISSING_SPACE,
   ILLEGAL_WSPACE,
   ILLEGAL_NUM,
   ILLEGAL_IDENTIFIER,
   ILLEGAL_SYMBOL_TYPE,
   ILLEGAL_SYMBOL_NAME,
   MISSING_NUM,
   MISSING_IDENTIFIER,
   MISSING_NEWLINE,
   MISSING_DEF,
   CANNOT_INVERTED,
   MAX_LIT_ID,
   REDEF_GATE,
   REDEF_SYMBOLIC_NAME,
   REDEF_CONST,
   NUM_TOO_SMALL,
   NUM_TOO_BIG,

   DUMMY_END
};

/**************************************/
/*   Static varaibles and functions   */
/**************************************/
static unsigned lineNo = 0;  // in printint, lineNo needs to ++
static unsigned colNo  = 0;  // in printing, colNo needs to ++
static char buf[1024];
static string errMsg;
static int errInt;
static CirGate *errGate;

vector<int> wff;
vector<int> noused;

static bool
parseError(CirParseError err)
{
   switch (err) {
      case EXTRA_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Extra space character is detected!!" << endl;
         break;
      case MISSING_SPACE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing space character!!" << endl;
         break;
      case ILLEGAL_WSPACE: // for non-space white space character
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal white space char(" << errInt
              << ") is detected!!" << endl;
         break;
      case ILLEGAL_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal "
              << errMsg << "!!" << endl;
         break;
      case ILLEGAL_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Illegal identifier \""
              << errMsg << "\"!!" << endl;
         break;
      case ILLEGAL_SYMBOL_TYPE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Illegal symbol type (" << errMsg << ")!!" << endl;
         break;
      case ILLEGAL_SYMBOL_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Symbolic name contains un-printable char(" << errInt
              << ")!!" << endl;
         break;
      case MISSING_NUM:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Missing " << errMsg << "!!" << endl;
         break;
      case MISSING_IDENTIFIER:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing \""
              << errMsg << "\"!!" << endl;
         break;
      case MISSING_NEWLINE:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": A new line is expected here!!" << endl;
         break;
      case MISSING_DEF:
         cerr << "[ERROR] Line " << lineNo+1 << ": Missing " << errMsg
              << " definition!!" << endl;
         break;
      case CANNOT_INVERTED:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": " << errMsg << " " << errInt << "(" << errInt/2
              << ") cannot be inverted!!" << endl;
         break;
      case MAX_LIT_ID:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Literal \"" << errInt << "\" exceeds maximum valid ID!!"
              << endl;
         break;
      case REDEF_GATE:
         cerr << "[ERROR] Line " << lineNo+1 << ": Literal \"" << errInt
              << "\" is redefined, previously defined as "
              << errGate->getTypeStr() << " in line " << errGate->getLineNo()
              << "!!" << endl;
         break;
      case REDEF_SYMBOLIC_NAME:
         cerr << "[ERROR] Line " << lineNo+1 << ": Symbolic name for \""
              << errMsg << errInt << "\" is redefined!!" << endl;
         break;
      case REDEF_CONST:
         cerr << "[ERROR] Line " << lineNo+1 << ", Col " << colNo+1
              << ": Cannot redefine constant (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_SMALL:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too small (" << errInt << ")!!" << endl;
         break;
      case NUM_TOO_BIG:
         cerr << "[ERROR] Line " << lineNo+1 << ": " << errMsg
              << " is too big (" << errInt << ")!!" << endl;
         break;
      default: break;
   }
   return false;
}

/**************************************************************/
/*   class CirMgr member functions for circuit construction   */
/**************************************************************/
bool spaceCheck(const string& str){
   size_t start = 0, end = 0; int count = 0;
   while (end < str.size()-2){
      start = str.find_first_not_of(" ",start);
      end = str.find_first_of(" ",start);
      if (end == -1) break; 
      count++;
      if(str[end-1] == char(9)) { lineNo = 0; colNo = end-1; parseError(MISSING_SPACE); return false;}
      if (count == 6) { lineNo = 0; colNo = end; parseError(MISSING_NEWLINE); return false;}
      start = end;
   }
   if (str[3] != ' ' && count != 5){ lineNo = 0; colNo = 3; parseError(MISSING_SPACE); return false;}
   if (count == 2) {lineNo = 0; colNo = end+1; errMsg = "number of PIs"; parseError(MISSING_NUM); return false;}
   size_t wspace = str.find_first_of(char(9),0);
   if (wspace != -1 && str[wspace-1] != ' ' && str[wspace+1] != ' ' && str[wspace-1] != char(9) && str[wspace+1] != char(9)){
      lineNo = 0; colNo = wspace; parseError(MISSING_SPACE); return false;
   }
   return true;
}

bool
CirMgr::readCircuit(const string& fileName)
{
   _piGate.clear(); _poGate.clear();_AIG.clear();_totalGate.clear();_dfs.clear(); lineNo = 0;
   //read file and open
   ifstream  _dofile;   string tmp;    vector<string> record;
   _dofile.open(fileName.c_str(), ifstream::in);
   if(!_dofile.is_open()){ cerr << "Cannot open design \"" << fileName << "\"!!" << endl; return false; }
   
   //getline
   while(getline(_dofile, tmp))  record.push_back(tmp);

   //first_line
   vector <string> content;
   size_t start = 0, end = 0;
   while (end < record[0].size()-1){
      string tmp_word = "";
      start = record[0].find_first_not_of(" ",start);
      end = record[0].find_first_of(" ",start);
      if(record[0].find_first_of(" ",end+1) == end+1){ colNo = end+1; parseError(EXTRA_SPACE); return false;}
      if(record[0].find_first_of(char(9),end+1) == end+1){ colNo = end+1; errInt = 9; parseError(ILLEGAL_WSPACE); return false;}
      if (end == -1) {
         end = record[0].size()-1;
         tmp_word = record[0].substr(start,end-start+1);
      }
      else tmp_word = record[0].substr(start,end-start);
      content.push_back(tmp_word);
      start = end;
   }

   if (!spaceCheck(record[0])) return false;
   if (content[0] != "aag"){ errMsg = content[0]; parseError(ILLEGAL_IDENTIFIER); return false; }
   int num= 0;
   for (int i = 1; i < 6; i ++){
      if (!myStr2Int(content[i], num)){
         if(i == 1) errMsg = "number of variables(" + content[1] + ")";
         if(i == 2) errMsg = "number of PIs(" + content[2] + ")";
         if(i == 3) errMsg = "number of latches(" + content[3] + ")";
         if(i == 4) errMsg = "number of POs(" + content[4] + ")";
         if(i == 5) errMsg = "number of AIGs(" + content[5] + ")";
         parseError(ILLEGAL_NUM);
         return false;
      }
      if(i == 3 && num != 0){ errMsg = "latches"; parseError(ILLEGAL_NUM); return false; }
      spec.push_back(num);
   }

   if(spec[0] < spec[1]+spec[4]) { errMsg = "Number of variables"; errInt = spec[0]; parseError(NUM_TOO_SMALL); return false; }
   lineNo++;

   //----------lineNo == 1----------//
   _totalGate.resize(spec[0]+spec[3]+1);
   int gateID[spec[0]+spec[3]+1];
   for (size_t i = 0; i < spec[0]+spec[3]+1; i++){
      gateID[i] = -1;
   }

   _totalGate[0] = new CONST_0(0,0,CONST_GATE);

   //create PIGate
   for (int i = 0; i < spec[1]; i++){
      int num = 0;
      myStr2Int(record[lineNo], num);
      //************************************************** error **************************************************//
      if (num%2 == 1){ errMsg = "PI";  errInt = num;  parseError(CANNOT_INVERTED); return false;}
      if (gateID[num/2] != -1){ errInt = num; errGate = _totalGate[num/2]; parseError(REDEF_GATE); return false;}
      //************************************************** error **************************************************//
      CirGate* tmp = new PIGate(num/2, lineNo+1, PI_GATE); 
      _piGate.push_back(tmp);
      _totalGate[num/2] = tmp;
      gateID[num/2] = lineNo;
      lineNo++;
   }

   //create POGate
   int POFanin[spec[3]];
   for (int i = 0; i < spec[3]; i++){
      int num = 0;   bool invert = false;
      myStr2Int(record[lineNo], num);
      if(num%2 == 1) invert = true;
      POFanin[i] = num/2;
      CirGate* tmp = new POGate(spec[0]+i+1, lineNo+1, invert, 0, PO_GATE);
      _poGate.push_back(tmp);
      _totalGate[spec[0]+i+1] = tmp;
      gateID[spec[0]+i+1] = lineNo;
      lineNo++;
   }

   //create AIG
   unsigned int array[spec[4]][2];
   for (int i = 0; i < spec[4]; i++){
      size_t start = record[lineNo].find_first_not_of(' ',0), end = 0; char del = ' ';
      vector <int> aig; bool inv0 = false; bool inv1 = false;
      while(end < record[lineNo].size()-1){
         start = record[lineNo].find_first_not_of(del,end);
         end = record[lineNo].find_first_of(del,start);
         int num = 0;
         myStr2Int(record[lineNo].substr(start, end-start) ,num);
         aig.push_back(num);
         start = end;
      }
      if(aig[1]%2 == 1) inv0 = true;
      if(aig[2]%2 == 1) inv1 = true;
      array[i][0] = aig[1]/2;
      array[i][1] = aig[2]/2;
      //****************************************************** error ******************************************************//
      if (gateID[aig[0]/2] != -1){ errInt = aig[0]; errGate = _totalGate[aig[0]/2]; parseError(REDEF_GATE); return false;}
      //****************************************************** error ******************************************************//
      CirGate* tmp = new AIG(aig[0]/2, lineNo+1, NULL, inv0, NULL, inv1, AIG_GATE);
      _AIG.push_back(tmp);
      _totalGate[aig[0]/2] = tmp;
      gateID[aig[0]/2] = lineNo;
   lineNo++;
   aig.clear();
   } 

   //connection
   bool match = false;
   for(size_t i = 0; i < _AIG.size(); i++){
      for (size_t j = 0; j < 2; j++){
         match = false;
         
         if (_totalGate[array[i][j]] != NULL ){
            (j == 0)?_AIG[i]->setFanin0(_totalGate[array[i][0]]):_AIG[i]->setFanin1(_totalGate[array[i][1]]);
            _totalGate[array[i][j]]->addFanout(_AIG[i]);
            match = true;
         }
         
         if (match == false){
            CirGate* tmp = new AIG(array[i][j], 0, NULL, false, NULL, false, UNDEF_GATE);
            (j == 0) ? _AIG[i]->setFanin0(tmp) : _AIG[i]->setFanin1(tmp);
            tmp->addFanout(_AIG[i]);
            _totalGate[array[i][j]] = tmp;
            match = true;
         }
      }
   }
   for (int i = 0; i < spec[3]; i++){
      _totalGate[POFanin[i]]->addFanout(_poGate[i]);
      _poGate[i]->setFanin0(_totalGate[POFanin[i]]);
   }

   //input output name
   int index_i = 0; int index_o = 0;   
   while(1){
      if (record.size() <= lineNo) break;
      else if (record[lineNo][0] == 'c' || record[lineNo][0] == EOF) break;
      else {
         tmp = record[lineNo].substr(record[lineNo].find_first_of(" ",0)+1,record[lineNo].size()-record[lineNo].find_first_of(" ",0)-1);
         if (record[lineNo][0] == 'i'){
               _piGate[index_i]->setName(tmp);
               index_i++; lineNo++;
            }
         else if (record[lineNo][0] == 'o'){
            _poGate[index_o]->setName(tmp);
            index_o++; lineNo++;
         }
         else return false;
      }
   }

   for (size_t i = 0; i < _totalGate.size(); i++){
      if(_totalGate[i]!= NULL  && _totalGate[i]->getTypeStr() != "PO") { _totalGate[i]->sortFanout();}
   }  

   for (size_t i = 0; i < _poGate.size(); i++) DFS(_poGate[i]);

   for (size_t i = 0; i < _totalGate.size(); i++){
       if(_totalGate[i]!= NULL ) _totalGate[i]->setMark(false);
   } 

   for (size_t i = 0; i < _totalGate.size(); i++){
      if(_totalGate[i]!= NULL &&_totalGate[i]->getTypeStr() != "PI" && _totalGate[i]->getTypeStr() != "CONST" && _totalGate[i]->getTypeStr() != "UNDEF"){
         if (_totalGate[i]->getTypeStr() == "PO"){
            if (_totalGate[i]->getFanin0()->getTypeStr() == "UNDEF") wff.push_back(_totalGate[i]->getID());
         }
         else if (_totalGate[i]->getTypeStr() == "AIG"){
            if (_totalGate[i]->getFanin0()->getTypeStr() == "UNDEF" || _totalGate[i]->getFanin1()->getTypeStr() == "UNDEF") wff.push_back(_totalGate[i]->getID());
         } 
      }
   }
 
   for (size_t i = 0; i < _totalGate.size(); i++)
      if (_totalGate[i]!= NULL && _totalGate[i]->getTypeStr() != "PO" && _totalGate[i]->getTypeStr() != "CONST" && _totalGate[i]->getFanoutSize() == 0) noused.push_back(_totalGate[i]->getID());
 
   for (size_t i = 0; i < noused.size(); i++){
      for (size_t j = i+1; j < noused.size(); j++){
         if(noused[i] > noused[j]) swap(noused[i],noused[j]);
      }
   }
   for (size_t i = 0; i < _totalGate.size(); i++){
      if(_totalGate[i]!= NULL)_totalGate[i]->setMark(false);
   } 
return true;
}



/**********************************************************/
/*   class CirMgr member functions for circuit printing   */
/**********************************************************/
/*********************
Circuit Statistics
==================
  PI          20
  PO          12
  AIG        130
------------------
  Total      162
*********************/
void
CirMgr::printSummary() const
{
   cout << endl;
   cout << "Circuit Statistics" << endl;
   cout << "==================" << endl;
   cout << "  " << "PI" << setw(12) << right << _piGate.size() << endl;
   cout << "  " << "PO" << setw(12) << right << _poGate.size() << endl;
   cout << "  " << "AIG" << setw(11) << right << _AIG.size() << endl;
   cout << "------------------" << endl;
   cout << "  " << "Total" << setw(9) << (spec[1] + spec[3] + spec[4]) << endl;
}

void
CirMgr::printNetlist() const
{
   cout << endl;
   for (size_t i = 0; i < _dfs.size(); i++){
      cout << "[" << i << "]";
      if (_dfs[i]->getTypeStr() == "CONST")cout << " CONST0";
      else cout << " " << _dfs[i]->getTypeStr();
      if(_dfs[i]->getTypeStr() != "CONST"){
         cout << " ";
         if(_dfs[i]->getTypeStr() != "AIG") cout << " ";
         cout << _dfs[i]->getID();
         if (_dfs[i]->getTypeStr() == "PI" && _dfs[i]->getName() != "") cout << " (" << _dfs[i]->getName() << ")";
         else if (_dfs[i]->getTypeStr() == "PO"){
            cout << " ";
            if (_dfs[i]->getFanin0()->getTypeStr() == "UNDEF") cout << "*";
            if (_dfs[i]->getInvert0() == true) cout << "!";
            cout << _dfs[i]->getFanin0()->getID();
            if (_dfs[i]->getName() != "") cout << " (" << _dfs[i]->getName() << ")";
         }
         else if (_dfs[i]->getTypeStr() == "AIG"){
            cout << " ";
            if (_dfs[i]->getFanin0()->getTypeStr() == "UNDEF") cout << "*";
            if (_dfs[i]->getInvert0() == true) cout << "!";
            cout << _dfs[i]->getFanin0()->getID() << " ";
            if (_dfs[i]->getFanin1()->getTypeStr() == "UNDEF") cout << "*";
            if (_dfs[i]->getInvert1() == true) cout << "!";
            cout << _dfs[i]->getFanin1()->getID();
         }
      }
      cout << endl;
   }
}

void
CirMgr::printPIs() const
{
   cout << "PIs of the circuit:";
   for (size_t i = 0; i < _piGate.size(); i++)  cerr << " " << _piGate[i]->getID();
   cout << endl;
}

void
CirMgr::printPOs() const
{
   cout << "POs of the circuit:";
   for (size_t i = 0; i < _poGate.size(); i++)  cerr << " " << _poGate[i]->getID();
   cout << endl;
}

void
CirMgr::printFloatGates() const
{
   if (wff.size() != 0){
      cout << "Gates with floating fanin(s):";
      for (size_t i = 0; i < wff.size(); i++) cout << " " << wff[i];
      cout << endl;
   }
   if (noused.size() != 0){
      cout << "Gates defined but not used  :";
      for (size_t i = 0; i < noused.size(); i++) cout << " " << noused[i];
      cout << endl;
   }
}

void
CirMgr::writeAag(ostream& outfile) const
{
   size_t a = 0;
   for(size_t i = 0; i < _dfs.size(); i++) { if(_dfs[i]->getTypeStr() == "AIG") a++; }
   outfile << "aag" << " " << spec[0] << " " << spec[1] << " " << spec[2] << " " << spec[3] << " " << a << endl;
   for (size_t i = 0; i < _piGate.size(); i++) outfile << (_piGate[i]->getID())*2 << endl;         //PI
   for (size_t i = 0; i < _poGate.size(); i++) {                                                   //PO
      if (_poGate[i]->getInvert0() == false) outfile << (_poGate[i]->getFanin0()->getID())*2 << endl;
      else outfile << (_poGate[i]->getFanin0()->getID())*2 + 1 << endl;
   }
   for (size_t i = 0; i < _dfs.size(); i++){                                                        //AIG
      if(_dfs[i]->getTypeStr() == "AIG"){
         outfile << (_dfs[i]->getID()*2) << " ";
         if(_dfs[i]->getInvert0() == false) outfile << (_dfs[i]->getFanin0()->getID())*2 << " ";
         else outfile << (_dfs[i]->getFanin0()->getID())*2 + 1 << " ";
         if(_dfs[i]->getInvert1() == false) outfile << (_dfs[i]->getFanin1()->getID())*2 ;
         else outfile << (_dfs[i]->getFanin1()->getID())*2 + 1 ;
         outfile << endl;
      }
   }
   for (size_t i = 0; i < _piGate.size(); i++){
      if (_piGate[i]->getName() != "") outfile << "i" << i << " " << _piGate[i]->getName() << endl;
   }
   for (size_t i = 0; i < _poGate.size(); i++){
      if (_poGate[i]->getName() != "") outfile << "o" << i << " " << _poGate[i]->getName() << endl;
   }
   outfile << "c" << endl;
   outfile << "AAG output by Chung-Yang (Ric) Huang" << endl;
}
