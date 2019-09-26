/****************************************************************************
  FileName     [ dbCmd.cpp ]
  PackageName  [ db ]
  Synopsis     [ Define database commands ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2015-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <iostream>
#include <iomanip>
#include <cassert>
#include <cmath>
#include <ctype.h>
#include "util.h"
#include "dbCmd.h"
#include "dbJson.h"

// Global variable
DBJson dbjson;

bool
initDbCmd()
{
   // TODO...
   if (!(cmdMgr->regCmd("DBAPpend", 4, new DBAppendCmd) &&
         cmdMgr->regCmd("DBAVerage", 4, new DBAveCmd) &&
         cmdMgr->regCmd("DBCount", 3, new DBCountCmd) &&
         cmdMgr->regCmd("DBMAx", 4, new DBMaxCmd) &&
         cmdMgr->regCmd("DBMIn", 4, new DBMinCmd) &&
         cmdMgr->regCmd("DBPrint", 3, new DBPrintCmd) &&
         cmdMgr->regCmd("DBRead", 3, new DBReadCmd) &&
         cmdMgr->regCmd("DBSOrt", 4, new DBSortCmd) &&
         cmdMgr->regCmd("DBSUm", 4, new DBSumCmd)
      )) {
              cerr << "Registering \"init\" commands fails... exiting" << endl;
      return false;
   }
   return true;
}

//----------------------------------------------------------------------
//    DBAPpend <(string key)><(int value)>
//----------------------------------------------------------------------
CmdExecStatus
DBAppendCmd::exec(const string& option)
{
        
   // TODO...
   // check option
// mydb> dbappend_______________________Missing option
   if (option.empty()) {return CmdExec::errorOption(CMD_OPT_MISSING,option);}
   else {
        size_t key_begin = option.find_first_not_of(' ');
        size_t key_end = option.find_first_of(' ',key_begin);
// mydb> dbappend mary_______________________Missing option
        if (key_end == -1) {
                //cerr << "key_end == -1" << endl;
                return CmdExec::errorOption(CMD_OPT_MISSING,"");
        }
        string tmp_key; 
        tmp_key.append(option, key_begin, key_end - key_begin);
        size_t value_begin = option.find_first_not_of(' ',key_end);
        size_t value_end;
        string extra_str;
        if (option.find(' ',value_begin) == option.npos) value_end = option.size();
        else value_end = option.find_first_of(' ',value_begin);
        string tmp_value; 
        tmp_value.append(option, value_begin, value_end - value_begin);
        if (option.find_first_not_of(' ',value_end) != option.npos){
                string extra_str;
                extra_str.append(option,(option.find_first_not_of(' ',value_end)), (option.size()-option.find_first_not_of(' ',value_end)));
                return CmdExec::errorOption(CMD_OPT_EXTRA,extra_str);
        }
// mydb> dbappend @@@ ... _______________________Illegal option!!
        if (!isValidVarName(tmp_key)) {return CmdExec::errorOption(CMD_OPT_ILLEGAL,tmp_key);}
        else {
// mydb> dbappend mary @@@ _______________________Illegal option(@@@)!!
bool value_valid = true;
        if (tmp_value[0]!= '-'&&
            tmp_value[0]!= '0'&&
            tmp_value[0]!= '1'&&
            tmp_value[0]!= '2'&&
            tmp_value[0]!= '3'&&
            tmp_value[0]!= '4'&&
            tmp_value[0]!= '5'&&
            tmp_value[0]!= '6'&&
            tmp_value[0]!= '7'&&
            tmp_value[0]!= '8'&&
            tmp_value[0]!= '9') {
                value_valid = false;
            }
        for (size_t i = 1; i < tmp_value.size();i++){
                if (tmp_value[i]!= '0'&&
                    tmp_value[i]!= '1'&&
                    tmp_value[i]!= '2'&&
                    tmp_value[i]!= '3'&&
                    tmp_value[i]!= '4'&&
                    tmp_value[i]!= '5'&&
                    tmp_value[i]!= '6'&&
                    tmp_value[i]!= '7'&&
                    tmp_value[i]!= '8'&&
                    tmp_value[i]!= '9') {
                        value_valid = false;
                    }
        }
                if (!value_valid) return CmdExec::errorOption(CMD_OPT_ILLEGAL,tmp_value);
                for (size_t i = 0; i < dbjson.size(); i++){
// mydb> DBAPpend mary 4("mary" already exist)
                        if (tmp_key == dbjson[i].key()){
                        cerr << "Error: Element with key \"" << tmp_key << "\" already exists!!" << endl;
                        return CMD_EXEC_ERROR;
                }
                }
        //轉換tmp_value成int
                int value = 0;
                if(tmp_value[0] == '-' && tmp_value.size() == 1) {return CmdExec::errorOption(CMD_OPT_ILLEGAL,tmp_value);}
                //處理負數
                if(tmp_value[0] == '-'){
                        for (size_t i = 1; i < tmp_value.size(); i++){
                                value += (tmp_value[i]-48)*pow(10,tmp_value.size()-i-1);
                }
                value = -value;
                }
                else{
                        for (size_t i = 0; i < tmp_value.size(); i++){
                                value += (tmp_value[i]-48)*pow(10,tmp_value.size()-i-1);
                        }
                }
        DBJsonElem newelm(tmp_key, value);
        dbjson.add(newelm);
        return CMD_EXEC_DONE;
                }
        }
}

void
DBAppendCmd::usage(ostream& os) const
{
   os << "Usage: DBAPpend <(string key)><(int value)>" << endl;
}

void
DBAppendCmd::help() const
{
   cout << setw(15) << left << "DBAPpend: "
        << "append an JSON element (key-value pair) to the end of DB" << endl;
}


//----------------------------------------------------------------------
//    DBAVerage
//----------------------------------------------------------------------
CmdExecStatus
DBAveCmd::exec(const string& option)
{  
   // check option
   if (!CmdExec::lexNoOption(option))
      return CMD_EXEC_ERROR;

   float a = dbjson.ave();
   if (a == NAN) {
      cerr << "Error: The average of the DB is nan." << endl;
      return CMD_EXEC_ERROR;
   }
   ios_base::fmtflags origFlags = cout.flags();
   cout << "The average of the DB is " << fixed
        << setprecision(2) << a << ".\n";
   cout.flags(origFlags);

   return CMD_EXEC_DONE;
}

void
DBAveCmd::usage(ostream& os) const
{     
   os << "Usage: DBAVerage" << endl;
}

void
DBAveCmd::help() const
{
   cout << setw(15) << left << "DBAVerage: "
        << "compute the average of the DB" << endl;
}


//----------------------------------------------------------------------
//    DBCount
//----------------------------------------------------------------------
CmdExecStatus
DBCountCmd::exec(const string& option)
{  
   // check option
   if (!CmdExec::lexNoOption(option))
      return CMD_EXEC_ERROR;

   size_t n = dbjson.size();
   if (n > 1)
      cout << "There are " << n << " JSON elements in DB." << endl;
   else if (n == 1)
      cout << "There is 1 JSON element in DB." << endl;
   else
      cout << "There is no JSON element in DB." << endl;

   return CMD_EXEC_DONE;
}

void
DBCountCmd::usage(ostream& os) const
{     
   os << "Usage: DBCount" << endl;
}

void
DBCountCmd::help() const
{
   cout << setw(15) << left << "DBCount: "
        << "report the number of JSON elements in the DB" << endl;
}


//----------------------------------------------------------------------
//    DBMAx
//----------------------------------------------------------------------
CmdExecStatus
DBMaxCmd::exec(const string& option)
{  
   // check option
   if (!CmdExec::lexNoOption(option))
      return CMD_EXEC_ERROR;
   size_t maxI;
   int maxN = dbjson.max(maxI);
   if (maxN == INT_MIN) {
      cerr << "Error: The max JSON element is nan." << endl;
      return CMD_EXEC_ERROR;
   }
   cout << "The max JSON element is { " << dbjson[maxI] << " }." << endl;

   return CMD_EXEC_DONE;
}

void
DBMaxCmd::usage(ostream& os) const
{     
   os << "Usage: DBMAx" << endl;
}

void
DBMaxCmd::help() const
{
   cout << setw(15) << left << "DBMAx: "
        << "report the maximum JSON element" << endl;
}


//----------------------------------------------------------------------
//    DBMIn
//----------------------------------------------------------------------
CmdExecStatus
DBMinCmd::exec(const string& option)
{  
   // check option
   if (!CmdExec::lexNoOption(option))
      return CMD_EXEC_ERROR;

   size_t minI;
   int minN = dbjson.min(minI);
   if (minN == INT_MAX) {
      cerr << "Error: The min JSON element is nan." << endl;
      return CMD_EXEC_ERROR;
   }
   cout << "The min JSON element is { " << dbjson[minI] << " }." << endl;

   return CMD_EXEC_DONE;
}

void
DBMinCmd::usage(ostream& os) const
{     
   os << "Usage: DBMIn" << endl;
}

void
DBMinCmd::help() const
{
   cout << setw(15) << left << "DBMIn: "
        << "report the minimum JSON element" << endl;
}


//----------------------------------------------------------------------
//    DBPrint [(string key)]
//----------------------------------------------------------------------
CmdExecStatus
DBPrintCmd::exec(const string& option)
{  
   // TODO...
   //mydb> DBPrint

   if (dbjson.size() == 0) return CMD_EXEC_ERROR;

   if (option.size() == 0){
        cerr << "{" << endl;
        for (size_t i = 0; i < dbjson.size()-1; i++){
             cerr << "  " << dbjson[i] << "," << endl;
        }
        cerr << "  " << dbjson[dbjson.size()-1] << endl;
        cerr << "}" << endl;
        if (dbjson.size() == 1) cerr << "Total JSON element: 1"  << endl;
        else cerr << "Total JSON elements: " << dbjson.size() << endl;
        return CMD_EXEC_DONE;  
   }
   //mydb> DBPrint ...
   else {

           for (size_t i = 0;i < dbjson.size(); i++){
                   if (dbjson[i].key() == option) {
                           cerr << "{ " << dbjson[i] << " }" << endl;
                           return CMD_EXEC_DONE;  
                           }
           }
        cerr << "Error: No JSON element with key \""<< option << "\" is found." << endl;
        return CMD_EXEC_ERROR;
   }
}

void
DBPrintCmd::usage(ostream& os) const
{
   os << "DBPrint [(string key)]" << endl;
}

void
DBPrintCmd::help() const
{
   cout << setw(15) << left << "DBPrint: "
        << "print the JSON element(s) in the DB" << endl;
}


//----------------------------------------------------------------------
//    DBRead <(string jsonFile)> [-Replace]
//----------------------------------------------------------------------
CmdExecStatus
DBReadCmd::exec(const string& option)
{
   // check option
   vector<string> options;
   if (!CmdExec::lexOptions(option, options))
      return CMD_EXEC_ERROR;

   if (options.empty())
      return CmdExec::errorOption(CMD_OPT_MISSING, "");

   bool doReplace = false;
   string fileName;
   for (size_t i = 0, n = options.size(); i < n; ++i) {
      if (myStrNCmp("-Replace", options[i], 2) == 0) {
         if (doReplace) return CmdExec::errorOption(CMD_OPT_EXTRA,options[i]);
         doReplace = true;
      }
      else {
         if (fileName.size())
            return CmdExec::errorOption(CMD_OPT_ILLEGAL, options[i]);
         fileName = options[i];
      }
   }

   ifstream ifs(fileName.c_str());
   if (!ifs) {
      cerr << "Error: \"" << fileName << "\" does not exist!!" << endl;
      return CMD_EXEC_ERROR;
   }

   if (dbjson) {
      if (!doReplace) {
         cerr << "Error: DB exists. Use \"-Replace\" option for "
              << "replacement.\n";
         return CMD_EXEC_ERROR;
      }
      cout << "DB is replaced..." << endl;
      dbjson.reset();
   }
//   if (!(ifs >> dbtbl)) return CMD_EXEC_ERROR;
   ifs >> dbjson;
   cout << "\"" << fileName << "\" was read in successfully." << endl;

   return CMD_EXEC_DONE;
}

void
DBReadCmd::usage(ostream& os) const
{
   os << "Usage: DBRead <(string jsonFile)> [-Replace]" << endl;
}

void
DBReadCmd::help() const
{
   cout << setw(15) << left << "DBRead: "
        << "read data from .json file" << endl;
}


//----------------------------------------------------------------------
//    DBSOrt <-Key | -Value>
//----------------------------------------------------------------------
CmdExecStatus
DBSortCmd::exec(const string& option)
{
   // check option
   string token;
   if (!CmdExec::lexSingleOption(option, token, false))
      return CMD_EXEC_ERROR;

   if (myStrNCmp("-Key", token, 2) == 0) dbjson.sort(DBSortKey());
   else if (myStrNCmp("-Value", token, 2) == 0) dbjson.sort(DBSortValue());
   else return CmdExec::errorOption(CMD_OPT_ILLEGAL, token);

   return CMD_EXEC_DONE;
}

void
DBSortCmd::usage(ostream& os) const
{
   os << "Usage: DBSOrt <-Key | -Value>" << endl;
}

void
DBSortCmd::help() const
{
   cout << setw(15) << left << "DBSOrt: "
        << "sort the JSON object by key or value" << endl;
}


//----------------------------------------------------------------------
//    DBSUm
//----------------------------------------------------------------------
CmdExecStatus
DBSumCmd::exec(const string& option)
{  
   // check option
   if (!CmdExec::lexNoOption(option))
      return CMD_EXEC_ERROR;

   if (dbjson.empty()) {
      cerr << "Error: The sum of the DB is nan." << endl;
      return CMD_EXEC_ERROR;
   }
   cout << "The sum of the DB is " << dbjson.sum() << "." << endl;

   return CMD_EXEC_DONE;
}

void
DBSumCmd::usage(ostream& os) const
{     
   os << "Usage: DBSUm" << endl;
}

void
DBSumCmd::help() const
{
   cout << setw(15) << left << "DBSUm: "
        << "compute the summation of the DB" << endl;
}

