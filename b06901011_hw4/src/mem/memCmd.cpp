/****************************************************************************
  FileName     [ memCmd.cpp ]
  PackageName  [ mem ]
  Synopsis     [ Define memory test commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include "memCmd.h"
#include "memTest.h"
#include "cmdParser.h"
#include "util.h"

using namespace std;

extern MemTest mtest;  // defined in memTest.cpp

bool
initMemCmd()
{
   if (!(cmdMgr->regCmd("MTReset", 3, new MTResetCmd) &&
         cmdMgr->regCmd("MTNew", 3, new MTNewCmd) &&
         cmdMgr->regCmd("MTDelete", 3, new MTDeleteCmd) &&
         cmdMgr->regCmd("MTPrint", 3, new MTPrintCmd)
      )) {
      cerr << "Registering \"mem\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}


//----------------------------------------------------------------------
//    MTReset [(size_t blockSize)]
//----------------------------------------------------------------------
CmdExecStatus
MTResetCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token))
      return CMD_EXEC_ERROR;
   if (token.size()) {
      int b;
      if (!myStr2Int(token, b) || b < int(toSizeT(sizeof(MemTestObj)))) {
         cerr << "Illegal block size (" << token << ")!!" << endl;
         return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);
      }
      #ifdef MEM_MGR_H
      mtest.reset(toSizeT(b));
      #else
      mtest.reset();
      //cerr << "55"<< endl;
      #endif // MEM_MGR_H
   }
   else
      mtest.reset();
   return CMD_EXEC_DONE;
}

void
MTResetCmd::usage(ostream& os) const
{  
   os << "Usage: MTReset [(size_t blockSize)]" << endl;
}

void
MTResetCmd::help() const
{  
   cout << setw(15) << left << "MTReset: " 
        << "(memory test) reset memory manager" << endl;
}


//----------------------------------------------------------------------
//    MTNew <(size_t numObjects)> [-Array (size_t arraySize)]
//----------------------------------------------------------------------
CmdExecStatus
MTNewCmd::exec(const string& option)
{
   // TODO
   vector<string> options;
   if (!CmdExec::lexOptions(option,options)) return CMD_EXEC_ERROR;
//mtn XXX  
   if (options.size() == 1){
           if (!myStrNCmp("-Array", options[0], 2)){
                   cerr << "Error: Missing option after (" << options[0] << ")!!" << endl;
                   return CMD_EXEC_ERROR;
           }
           else {
                   int num = 0;
                   if (!myStr2Int(options[0],num)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
                   try{
                           if (num > 0) mtest.newObjs(num);
                   }
                   catch(std::bad_alloc){cerr << "catch" << endl;}
                   return CMD_EXEC_DONE;
           }
   }
//mtn XX SS
   else if (options.size() == 2){
           int num = 0;
           if (!myStrNCmp("-Array", options[0], 2)){
                   if (!myStr2Int(options[1],num)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else {
                           cerr << "Error: Missing option!!" << endl;
                           return CMD_EXEC_ERROR;
                   }
           }
           else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   }
//mtn XX SS WW
   else if (options.size() == 3){
           int size, num;
           int i = 0;
           //-a[4] * 3
           if (!myStrNCmp("-Array", options[0], 2)){
                   if (!myStr2Int(options[1],size)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   if (!myStr2Int(options[2],num)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   if (size <= 0) {cerr << "Error: Illegal option!! (" << size << ")" << endl; return CMD_EXEC_ERROR;}
                   if (num <= 0) {cerr << "Error: Illegal option!! (" << num << ")" << endl; return CMD_EXEC_ERROR;}
                   try{
                           if (size > 0  && num > 0)  {
                                   //cout << size << ' ' << num << '\n';
                                   mtest.newArrs(num,size); 
                           }
                   }
                   catch(std::bad_alloc){}
                   return CMD_EXEC_DONE;
           }   
           //3* -a[4]
           else if (!myStrNCmp("-Array", options[1], 2)){
                   if (!myStr2Int(options[0],num)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
                   if (!myStr2Int(options[2],size)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   if (size <= 0) {cerr << "Error: Illegal option!! (" << size << ")" << endl; return CMD_EXEC_ERROR;}
                   if (num <= 0) {cerr << "Error: Illegal option!! (" << num << ")" << endl; return CMD_EXEC_ERROR;}
                   try{
                           if (size > 0  && num > 0) mtest.newArrs(num,size);
                   }
                   catch(std::bad_alloc){}
                   return CMD_EXEC_DONE;
           }  
           else if (myStr2Int(options[0],i)) {
                   if (myStr2Int(options[1],i)) return CmdExec::errorOption(CMD_OPT_EXTRA,options[1]);
                   else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
           }
           else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   }
//mtn
   else if (options.size() == 0) {
           cerr << "Error: Missing option!!" << endl;
           return CMD_EXEC_ERROR;
   }
//mtn XX SS WW QQ
   else if (option.size() > 3){
           int i = 0;
           if (!myStrNCmp("-Array", options[0], 2)){
                   if (!myStr2Int(options[1],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   if (!myStr2Int(options[2],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   else return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
           }
           else if (myStr2Int(options[0],i)) {
                   if (!myStrNCmp("-Array", options[1], 2)){
                           if (!myStr2Int(options[2],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                           else return CmdExec::errorOption(CMD_OPT_EXTRA, options[3]);
                   }
                   else return CmdExec::errorOption(CMD_OPT_EXTRA,options[1]);
           }
   }
   else {
           cerr<<"Error: Illegal option!!"<<endl;
           return CMD_EXEC_ERROR;
   }
   // Use try-catch to catch the bad_alloc exception
}

void
MTNewCmd::usage(ostream& os) const
{  
   os << "Usage: MTNew <(size_t numObjects)> [-Array (size_t arraySize)]\n";
}

void
MTNewCmd::help() const
{  
   cout << setw(15) << left << "MTNew: " 
        << "(memory test) new objects" << endl;
}


//----------------------------------------------------------------------
//    MTDelete <-Index (size_t objId) | -Random (size_t numRandId)> [-Array]
//----------------------------------------------------------------------
CmdExecStatus
MTDeleteCmd::exec(const string& option)
{
   // TODO
   vector<string> options;
   if (!CmdExec::lexOptions(option,options)) return CMD_EXEC_ERROR;
//mtd
   if (options.size() == 0){
           cerr << "Error: Missing option!!" << endl;
           return CMD_EXEC_ERROR;
   }
//mtd XX
   else if (options.size() == 1){
           if (!myStrNCmp("-Index", options[0], 2)) return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
           else if (!myStrNCmp("-Random", options[0], 2)) return CmdExec::errorOption(CMD_OPT_MISSING, options[0]);
           else if (!myStrNCmp("-Array", options[0], 2)) {
                   cerr << "Error: Missing option!!" << endl;
                   return CMD_EXEC_ERROR;
           }
           else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   }
//mtd XX SS
   else if (options.size() == 2){
           if (!myStrNCmp("-Index", options[0], 2)){
                   int i = 0;
                   if (!myStr2Int(options[1],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else if (i <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else {
                           if (i >= mtest.getObjListSize()){
                                   cerr << "Size of object list (" << mtest.getObjListSize() << ") is <= " << i << "!!" << endl;
                                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                           }
                           else {
                                   mtest.deleteObj(i);
                                   return CMD_EXEC_DONE;
                           }
                   }
           }
           else if (!myStrNCmp("-Random", options[0], 2)){
                   int i = 0;
                   if (!myStr2Int(options[1],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else if (i <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else {
                           if (mtest.getObjListSize() == 0){
                                   cerr << "Size of object list is 0!!" << endl;
                                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
                           }
                           else {
                                   for (int j = 0; j < i; j++){
                                           int random = rnGen(mtest.getObjListSize());
                                           mtest.deleteObj(random);
                                   }
                                   return CMD_EXEC_DONE;
                           }
                   }

           }
           else if (!myStrNCmp("-Array", options[0], 2)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
           else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   }
//mtd XX SS WW
   else if (options.size() == 3){
        //mtd -i XX -a
           if (!myStrNCmp("-Index", options[0], 2) && !myStrNCmp("-Array", options[2], 2)){
                   int i = 0;
                   if (!myStr2Int(options[1],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else if (i <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else {
                           if (i >= mtest.getArrListSize()){
                                   cerr << "Size of array list (" << mtest.getArrListSize() << ") is <= " << i << "!!" << endl;
                                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                           }
                           else {
                                   mtest.deleteArr(i);
                                   return CMD_EXEC_DONE;
                           }
                   }
           }
        //mtd -a -i XX
           else if (!myStrNCmp("-Array", options[0], 2) && !myStrNCmp("-Index", options[1], 2)){
                   int i = 0;
                   if (!myStr2Int(options[2],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   else if (i <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   else {
                           if (i >= mtest.getArrListSize()){
                                   cerr << "Size of array list (" << mtest.getArrListSize() << ") is <= " << i << "!!" << endl;
                                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                           }
                           else {
                                   mtest.deleteArr(i);
                                   return CMD_EXEC_DONE;
                           }
                   }
           }
        //mtd -r XX -a 
           else if (!myStrNCmp("-Random", options[0], 2) && !myStrNCmp("-Array", options[2], 2)){
                   int i = 0;
                   if (!myStr2Int(options[1],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else if (i < 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else {
                           if (mtest.getArrListSize() == 0){
                                   cerr << "Size of array list is 0!!" << endl;
                                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
                           }
                           else {
                                   for (int j = 0; j < i; j++){
                                           int random = rnGen(mtest.getArrListSize());
                                           mtest.deleteArr(random);
                                   }
                                   return CMD_EXEC_DONE;
                           }
                   }
           }
        //mtd -a -r XX
           else if (!myStrNCmp("-Array", options[0], 2) && !myStrNCmp("-Random", options[1], 2)){
                   int i = 0;
                   if (!myStr2Int(options[2],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   else if (i <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   else {
                           if (mtest.getArrListSize() == 0){
                                   cerr << "Size of array list is 0!!" << endl;
                                   return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                           }
                           else {
                                   for (int j = 0; j < i; j++){
                                           int random = rnGen(mtest.getArrListSize());
                                           mtest.deleteArr(random);
                                   }
                                   return CMD_EXEC_DONE;
                           }
                   }
           }
           else if (!myStrNCmp("-Random", options[0], 2)){
                   int i = 0;
                   if (!myStr2Int(options[1],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else if (i <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else {
                        if (!myStrNCmp("-Random", options[2], 2) || !myStrNCmp("-Index", options[2], 2)) return CmdExec::errorOption(CMD_OPT_EXTRA, options[2]);
                        else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   }
           }
           else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
   }
//mtd XX SS WW QQ AA
   else if (options.size() > 3){
           if (!myStrNCmp("-Random", options[0], 2)){
                   int i = 0;
                   if (!myStr2Int(options[1],i)) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else if (i <= 0) return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[1]);
                   else {
                        if (!myStrNCmp("-Random", options[2], 2) || !myStrNCmp("-Index", options[2], 2)) return CmdExec::errorOption(CMD_OPT_EXTRA, options[2]);
                        else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[2]);
                   }
           }



   }
   else return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[0]);
}

void
MTDeleteCmd::usage(ostream& os) const
{  
   os << "Usage: MTDelete <-Index (size_t objId) | "
      << "-Random (size_t numRandId)> [-Array]" << endl;
}

void
MTDeleteCmd::help() const
{  
   cout << setw(15) << left << "MTDelete: " 
        << "(memory test) delete objects" << endl;
}


//----------------------------------------------------------------------
//    MTPrint
//----------------------------------------------------------------------
CmdExecStatus
MTPrintCmd::exec(const string& option)
{
   // check option
   if (option.size())
      return CmdExec::errorOption(CMD_OPT_EXTRA, option);
   mtest.print();

   return CMD_EXEC_DONE;
}

void
MTPrintCmd::usage(ostream& os) const
{  
   os << "Usage: MTPrint" << endl;
}

void
MTPrintCmd::help() const
{  
   cout << setw(15) << left << "MTPrint: " 
        << "(memory test) print memory manager info" << endl;
}


