/****************************************************************************
  FileName     [ cmdParser.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command parsing member functions for class CmdParser ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-present LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <iostream>
#include <iomanip>
#include <cstdlib>
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include "util.h"
#include "cmdParser.h"


using namespace std; 

//----------------------------------------------------------------------
//    External funcitons
//----------------------------------------------------------------------
void mybeep();
//----------------------------------------------------------------------
//    Member Function for class cmdParser
//----------------------------------------------------------------------
// return false if file cannot be opened
// Please refer to the comments in "DofileCmd::exec", cmdCommon.cpp
bool
CmdParser::openDofile(const string& dof)
{
   // TODO...
   if (_dofileStack.size() > 1024) return false;
   else {
     _dofile = new ifstream(dof.c_str());
     if ((*_dofile).is_open()){
       _dofileStack.push(_dofile);
       return _dofile;
     }
     else { 
       delete _dofile;
       if ((int)_dofileStack.size() != 0){
         _dofile = _dofileStack.top();
         return false;
       }
       else {
         _dofile = 0;
         return false;
       }
     }
   }
}

// Must make sure _dofile != 0
void
CmdParser::closeDofile()
{
   assert(_dofile != 0);
   // TODO...
   delete _dofile;
   _dofileStack.pop();
   if ((int)_dofileStack.size() ==0) _dofile = 0;
   else _dofile = _dofileStack.top();
}

// Return false if registration fails
bool
CmdParser::regCmd(const string& cmd, unsigned nCmp, CmdExec* e)
{
   // Make sure cmd hasn't been registered and won't cause ambiguity
   string str = cmd;
   unsigned s = str.size();
   if (s < nCmp) return false;
   while (true) {
      if (getCmd(str)) return false;
      if (s == nCmp) break;
      str.resize(--s);
   }

   // Change the first nCmp characters to upper case to facilitate
   //    case-insensitive comparison later.
   // The strings stored in _cmdMap are all upper case
   //
   assert(str.size() == nCmp);  // str is now mandCmd
   string& mandCmd = str;
   for (unsigned i = 0; i < nCmp; ++i)
      mandCmd[i] = toupper(mandCmd[i]);
   string optCmd = cmd.substr(nCmp);
   assert(e != 0);
   e->setOptCmd(optCmd);

   // insert (mandCmd, e) to _cmdMap; return false if insertion fails.
   return (_cmdMap.insert(CmdRegPair(mandCmd, e))).second;
}

// Return false on "quit" or if excetion happens
CmdExecStatus
CmdParser::execOneCmd()
{
   bool newCmd = false;
   if (_dofile != 0)
      newCmd = readCmd(*_dofile);
   else
      newCmd = readCmd(cin);

   // execute the command
   if (newCmd) {
     /*////////////////////
     for (CmdMap::iterator it = _cmdMap.begin(); it != _cmdMap.end(); it++){
       cerr << (*it).first << endl;

     }
     */////////////////////
      string option;
      CmdExec* e = parseCmd(option);
      if (e != 0)
         return e->exec(option);
   }
   return CMD_EXEC_NOP;
}

// For each CmdExec* in _cmdMap, call its "help()" to print out the help msg.
// Print an endl at the end.
void
CmdParser::printHelps() const
{
   // TODO...
   for (CmdMap::const_iterator it = _cmdMap.begin(); it != _cmdMap.end();it++){
     (*it).second->help();
   }
   cerr << endl;
}

void
CmdParser::printHistory(int nPrint) const
{
   assert(_tempCmdStored == false);
   if (_history.empty()) {
      cout << "Empty command history!!" << endl;
      return;
   }
   int s = _history.size();
   if ((nPrint < 0) || (nPrint > s))
      nPrint = s;
   for (int i = s - nPrint; i < s; ++i)
      cout << "   " << i << ": " << _history[i] << endl;
}


//
// Parse the command from _history.back();
// Let string str = _history.back();
//
// 1. Read the command string (may contain multiple words) from the leading
//    part of str (i.e. the first word) and retrieve the corresponding
//    CmdExec* from _cmdMap
//    ==> If command not found, print to cerr the following message:
//        Illegal command!! "(string cmdName)"
//    ==> return it at the end.
// 2. Call getCmd(cmd) to retrieve command from _cmdMap.
//    "cmd" is the first word of "str".
// 3. Get the command options from the trailing part of str (i.e. `second
//    words and beyond) and store them in "option"
//
CmdExec*
CmdParser::parseCmd(string& option)
{
   assert(_tempCmdStored == false);
   assert(!_history.empty());
   string str = _history.back();

   // TODO...
   assert(str[0] != 0 && str[0] != ' ');
//command + option
   if (str.find_first_of(' ') != str.npos){
     string cmd = str.substr(0, str.find_first_of(' '));
     if (getCmd(cmd) == NULL){
       cerr << "Illegal command!! (" << cmd  << ")" << endl;
       return getCmd(cmd);
     }
     else {
       option = str.substr(str.find_first_of(' ')+1, str.size() - str.find_first_of(' '));
       return getCmd(cmd);
   }
  }
  //command only
  else {
    string cmd = str;
    if (getCmd(cmd) == NULL){
      cerr << "Illegal command!! (" << cmd  << ")" << endl;
      return NULL;
    }
    else {
      return getCmd(cmd);
    }
  }
}

// Remove this function for TODO...
//
// This function is called by pressing 'Tab'.
// It is to list the partially matched commands.
// "str" is the partial string before current cursor position. It can be 
// a null string, or begin with ' '. The beginning ' ' will be ignored.
//
// Several possibilities after pressing 'Tab'
// (Let $ be the cursor position)
// 1. LIST ALL COMMANDS
//    --- 1.1 ---
//    [Before] Null cmd
//    cmd> $
//    --- 1.2 ---
//    [Before] Cmd with ' ' only
//    cmd>     $
//    [After Tab]
//    ==> List all the commands, each command is printed out by:
//           cout << setw(12) << left << cmd;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location (including ' ')
//
// 2. LIST ALL PARTIALLY MATCHED COMMANDS
//    --- 2.1 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$                   // partially matched
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$                   // and then re-print the partial command
//    --- 2.2 ---
//    [Before] partially matched (multiple matches)
//    cmd> h$llo                // partially matched with trailing characters
//    [After Tab]
//    HELp        HIStory       // List all the parially matched commands
//    cmd> h$llo                // and then re-print the partial command
//
// 3. LIST THE SINGLY MATCHED COMMAND
//    ==> In either of the following cases, print out cmd + ' '
//    ==> and reset _tabPressCount to 0
//    --- 3.1 ---
//    [Before] partially matched (single match)
//    cmd> he$
//    [After Tab]
//    cmd> heLp $               // auto completed with a space inserted
//    --- 3.2 ---
//    [Before] partially matched with trailing characters (single match)
//    cmd> he$ahah
//    [After Tab]
//    cmd> heLp $ahaha
//    ==> Automatically complete on the same line
//    ==> The auto-expanded part follow the strings stored in cmd map and
//        cmd->_optCmd. Insert a space after "heLp"
//    --- 3.3 ---
//    [Before] fully matched (cursor right behind cmd)
//    cmd> hElP$sdf
//    [After Tab]
//    cmd> hElP $sdf            // a space character is inserted
//
// 4. NO MATCH IN FITST WORD
//    --- 4.1 ---
//    [Before] No match
//    cmd> hek$
//    [After Tab]
//    ==> Beep and stay in the same location
//
// 5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
//    --- 5.1 ---
//    [Before] Already matched on first tab pressing
//    cmd> help asd$gh
//    [After] Print out the usage for the already matched command
//    Usage: HELp [(string cmd)]
//    cmd> help asd$gh
//
// 6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
//    ==> Note: command usage has been printed under first tab press
//    ==> Check the word the cursor is at; get the prefix before the cursor
//    ==> So, this is to list the file names under current directory that
//        match the prefix
//    ==> List all the matched file names alphabetically by:
//           cout << setw(16) << left << fileName;
//    ==> Print a new line for every 5 commands
//    ==> After printing, re-print the prompt and place the cursor back to
//        original location
//    --- 6.1 ---
//    Considering the following cases in which prefix is empty:
//    --- 6.1.1 ---
//    [Before] if prefix is empty, and in this directory there are multiple
//             files and they do not have a common prefix,
//    cmd> help $sdfgh
//    [After] print all the file names
//    .               ..              Homework_3.docx Homework_3.pdf  Makefile
//    MustExist.txt   MustRemove.txt  bin             dofiles         include
//    lib             mydb            ref             src             testdb
//    cmd> help $sdfgh
//    --- 6.1.2 ---
//    [Before] if prefix is empty, and in this directory there are multiple
//             files and all of them have a common prefix,
//    cmd> help $orld
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help mydb-$orld
//    --- 6.1.3 ---
//    [Before] if is empty, and only one file in the current directory
//    cmd> help $ydb
//    [After] print out the single file name followed by a ' '
//    cmd> help mydb $
//    --- 6.2 ---
//    [Before] with a prefix and with mutiple matched files
//    cmd> help M$Donald
//    [After]
//    Makefile        MustExist.txt   MustRemove.txt
//    cmd> help M$Donald
//    --- 6.3 ---
//    [Before] with a prefix and with mutiple matched files,
//             and these matched files have a common prefix
//    cmd> help Mu$k
//    [After]
//    ==> auto insert the common prefix and make a beep sound
//    ==> DO NOT print the matched files
//    cmd> help Must$k
//    --- 6.4 ---
//    [Before] with a prefix and with a singly matched file
//    cmd> help MustE$aa
//    [After] insert the remaining of the matched file name followed by a ' '
//    cmd> help MustExist.txt $aa
//    --- 6.5 ---
//    [Before] with a prefix and NO matched file
//    cmd> help Ye$kk
//    [After] beep and stay in the same location
//    cmd> help Ye$kk
//
//    [Note] The counting of tab press is reset after "newline" is entered.
//
// 7. FIRST WORD NO MATCH
//    --- 7.1 ---
//    [Before] Cursor NOT on the first word and NOT matched command
//    cmd> he haha$kk
//    [After Tab]
//    ==> Beep and stay in the same location

void
CmdParser::listCmd(const string& str)
{
   // TODO...
//1. NULL CMD or Cmd with ' ' only
  //cerr << _tabPressCount << endl;
   if (str.find_first_not_of(' ') == str.npos){
     string tmp[13];
     int pos = 0;
     for (CmdMap::iterator it = _cmdMap.begin(); it !=_cmdMap.end(); it++){
       tmp[pos] = it->first + it->second->getOptCmd(); pos++;
     }
     cerr << endl;
     for (int i = 0; i < 13; i++){
       cout << setw(12) << left << tmp[i];
       if (i%5 == 4) cerr << endl;
     }
     reprintCmd();
   }
   else {
//----------------------------------------------------------------------
//    判斷cmd和prefix(tmp是全大寫的cursor前字串; )
//----------------------------------------------------------------------
     string tmp; string ori_str = str;
     for (size_t i = 0; i < str.size(); i++){
       tmp += toupper(str[i]);
     }
     tmp = tmp.substr(tmp.find_first_not_of(' '), tmp.size() - tmp.find_first_not_of(' '));
     ori_str = ori_str.substr(ori_str.find_first_not_of(' '), ori_str.size() - ori_str.find_first_not_of(' '));

     string tmp_first; string prefix;
     size_t first_space; size_t last_space;
     first_space = tmp.find(' ',tmp.find_first_not_of(' ')); 

     last_space = ori_str.find(' ',ori_str.find_last_of(' '));
     if (first_space != tmp.npos) tmp_first = tmp.substr(tmp.find_first_not_of(' '), first_space + 1);
     else tmp_first = "";

     if (last_space != ori_str.npos) prefix = ori_str.substr(last_space+1,ori_str.size()-last_space-1);
     else prefix = "";
//cerr << "(" << tmp_first << "," << prefix << ")" << endl;
      bool DBAP_val = tmp_first == "DBAPPEND " || tmp_first == "DBAPPEN " || tmp_first == "DBAPPE " || tmp_first == "DBAPP " || tmp_first == "DBAP ";
      bool DBAV_val = (tmp_first == "DBAVERAGE " || tmp_first == "DBAVERAG " || tmp_first == "DBAVERA " || tmp_first == "DBAVER " || tmp_first == "DBAVE " || tmp_first == "DBAV ");
      bool DBC_val = (tmp_first == "DBCOUNT " || tmp_first == "DBCOUN " || tmp_first == "DBCOU " || tmp_first == "DBCO " || tmp_first == "DBC ");
      bool DBMA_val = (tmp_first == "DBMAX " || tmp_first == "DBMA ");
      bool DBMI_val = (tmp_first == "DBMIN " || tmp_first == "DBMI ");
      bool DBP_val = (tmp_first == "DBPRINT " || tmp_first == "DBPRIN " || tmp_first == "DBPRI " || tmp_first == "DBPR " || tmp_first == "DBP ");
      bool DBR_val = (tmp_first == "DBREAD " || tmp_first == "DBREA " || tmp_first == "DBRE " || tmp_first == "DBR ");
      bool DBSO_val = (tmp_first == "DBSORT " || tmp_first == "DBSOR " || tmp_first == "DBSO ");
      bool DBSU_val = (tmp_first == "DBSUM " || tmp_first == "DBSU ");
      bool DO_val = (tmp_first == "DOFILE " || tmp_first == "DOFIL " || tmp_first == "DOFI " || tmp_first == "DOF " || tmp_first == "DO ");
      bool HEL_val = (tmp_first == "HELP " || tmp_first == "HEL ");
      bool HIS_val = (tmp_first == "HISTORY " || tmp_first == "HISTOR " || tmp_first == "HISTO " || tmp_first == "HIST " || tmp_first == "HIS ");
      bool Q_val = (tmp_first == "QUIT " || tmp_first == "QUI " || tmp_first == "QU " || tmp_first == "Q ");
      bool all_val = DBAP_val || DBAV_val || DBC_val || DBMA_val || DBMI_val || DBP_val || DBR_val || DBSO_val || DBSU_val || DO_val || HEL_val  || HIS_val || Q_val;
  //  讀檔案夾裡的檔案名稱
    string dir = string(".");//資料夾路徑(絕對位址or相對位址)
    vector<string> files = vector<string>();
    listDir(files,prefix,dir);
    //getdir(dir, files);

//2. partially matched
      if (tmp == "D"){
       cerr << endl;
       cout << setw(12) << left << "DBAPpend"<< setw(12) << left << "DBAVerage" << setw(12) << left << "DBCount" << setw(12) << left << "DBMAx" << setw(12) << left << "DBMIn" << endl;
       cout << setw(12) << left << "DBPrint"<< setw(12) << left << "DBRead" << setw(12) << left << "DBSOrt" << setw(12) << left << "DBSUm" << setw(12) << left << "DOfile";
       reprintCmd();
      }
      else if (tmp == "DB"){
       cerr << endl;
       cout << setw(12) << left << "DBAPpend"<< setw(12) << left << "DBAVerage" << setw(12) << left << "DBCount" << setw(12) << left << "DBMAx" << setw(12) << left << "DBMIn" << endl;
       cout << setw(12) << left << "DBPrint"<< setw(12) << left << "DBRead" << setw(12) << left << "DBSOrt" << setw(12) << left << "DBSUm";
       reprintCmd();
      }
      else if (tmp == "DBA"){
       cerr << endl;
       cout << setw(12) << left << "DBAPpend"<< setw(12) << left << "DBAVerage";
       reprintCmd();
      }
      else if (tmp == "DBM"){
       cerr << endl;
       cout << setw(12) << left << "DBMAx"<< setw(12) << left << "DBMIn";
       reprintCmd();
      }
      else if (tmp == "DBS"){
       cerr << endl;
       cout << setw(12) << left << "DBSOrt"<< setw(12) << left << "DBSUm";
       reprintCmd();
      }
      else if (tmp == "H"){
        cerr << endl;
        cout << setw(12) << left << "HELp"<< setw(12) << left << "HIStory";
        reprintCmd();
      }
//3. LIST THE SINGLY MATCHED COMMANDs
  ///////DBAPpend
      else if (tmp == "DBAPPEND"){insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAPPEN"){insertChar('d');insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAPPE"){insertChar('n');insertChar('d');insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAPP"){insertChar('e');insertChar('n');insertChar('d');insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAP"){insertChar('p');insertChar('e');insertChar('n');insertChar('d');insertChar(' '); _tabPressCount = 0;} 
  ///////DBAVerage 
      else if (tmp == "DBAVERAGE"){insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAVERAG"){insertChar('e');insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAVERA"){insertChar('g');insertChar('e');insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAVER"){insertChar('a');insertChar('g');insertChar('e');insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAVE"){insertChar('r');insertChar('a');insertChar('g');insertChar('e');insertChar(' '); _tabPressCount = 0;} 
      else if (tmp == "DBAV"){insertChar('e');insertChar('r');insertChar('a');insertChar('g');insertChar('e');insertChar(' '); _tabPressCount = 0;} 
  ///////DBCount
      else if (tmp == "DBC"){insertChar('o');insertChar('u');insertChar('n');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBCO"){insertChar('u');insertChar('n');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBCOU"){insertChar('n');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBCOUN"){insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBCOUNT"){insertChar(' '); _tabPressCount = 0;}
  ///////DBMAx
      else if (tmp == "DEMA"){insertChar('x');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DEMAX"){insertChar(' '); _tabPressCount = 0;}
  ///////DBMIn 
      else if (tmp == "DEMI"){insertChar('n');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DEMIN"){insertChar(' '); _tabPressCount = 0;} 
  ///////DBPrint
      else if (tmp == "DBP"){insertChar('r');insertChar('i');insertChar('n');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBPR"){insertChar('i');insertChar('n');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBPRI"){insertChar('n');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBPRIN"){insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBPRINT"){insertChar(' '); _tabPressCount = 0;}
  ///////DBRead
      else if (tmp == "DBR"){insertChar('e');insertChar('a');insertChar('d');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBRE"){insertChar('a');insertChar('d');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBREA"){insertChar('d');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBREAD"){insertChar(' '); _tabPressCount = 0;}
  ///////DBSOrt
      else if (tmp == "DBSO"){insertChar('r');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBSOR"){insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBSORT"){insertChar(' '); _tabPressCount = 0;}
  ///////DBSUm
      else if (tmp == "DBSU"){insertChar('m');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DBSUM"){insertChar(' '); _tabPressCount = 0;}
  ///////DOfile
      else if (tmp == "DO"){insertChar('f');insertChar('i');insertChar('l');insertChar('e');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DOF"){insertChar('i');insertChar('l');insertChar('e');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DOFI"){insertChar('l');insertChar('e');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DOFIL"){insertChar('e');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "DOFILE"){insertChar(' '); _tabPressCount = 0;}
  ///////HELp
      else if (tmp == "HE"){insertChar('L');insertChar('p');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "HEL"){insertChar('p');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "HELP"){insertChar(' '); _tabPressCount = 0;}
  ///////HIStory
      else if (tmp == "HI"){insertChar('S');insertChar('t');insertChar('o');insertChar('r');insertChar('y');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "HIS"){insertChar('t');insertChar('o');insertChar('r');insertChar('y');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "HIST"){insertChar('o');insertChar('r');insertChar('y');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "HISTO"){insertChar('r');insertChar('y');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "HISTOR"){insertChar('y');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "HISTORY"){insertChar(' '); _tabPressCount = 0;}
  ///////Quit
      else if (tmp == "Q"){insertChar('u');insertChar('i');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "QU"){insertChar('i');insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "QUI"){insertChar('t');insertChar(' '); _tabPressCount = 0;}
      else if (tmp == "QUIT"){insertChar(' '); _tabPressCount = 0;}

//5. FIRST WORD ALREADY MATCHED ON FIRST TAB PRESSING
      else if (DBAP_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DBAPpend <(string key)><(int value)>" << endl; reprintCmd();
      }
      else if (DBAV_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DBAVerage" << endl; reprintCmd();
      }
      else if (DBC_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DBCount" << endl; reprintCmd();
      }
      else if (DBMA_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DBMAx" << endl; reprintCmd();
      }
      else if (DBMI_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DBMIn" << endl; reprintCmd();
      }
      else if (DBP_val && _tabPressCount == 1){
         cerr << endl; cerr << "DBPrint [(string key)]" << endl; reprintCmd();
      }
      else if (DBR_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DBRead <(string jsonFile)> [-Replace]" << endl; reprintCmd();
      }
      else if (DBSO_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DBSOrt <-Key | -Value>" << endl; reprintCmd();
      }
      else if (DBSU_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DBSUm" << endl; reprintCmd();
      }
      else if (DO_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: DOfile <(string file)>" << endl; reprintCmd();
      }
      else if (HEL_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: HELp [(string cmd)]" << endl; reprintCmd();
      }
      else if (HIS_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: HIStory [(int nPrint)]" << endl; reprintCmd();
      }
      else if (Q_val && _tabPressCount == 1){
         cerr << endl; cerr << "Usage: Quit [-Force]" << endl; reprintCmd();
      }
      else if (_tabPressCount > 1){
//----------------------------------------------------------------------
//    判斷所有檔名的共同開頭(得到的共同檔名開頭：string tmp_com)
//----------------------------------------------------------------------
  size_t tmp_size; int files_pos;
  if (!files.empty()){
    tmp_size = files[0].size(); files_pos = 0;
  }
    string tmp_com; string tmp2_com = "";
    if (files.size() > 1){
      for (size_t i = 1; i < files.size(); i++){
        if (files[i].size() < tmp_size) {
          tmp_size = files[i].size();
          files_pos = i;
        }
      }
      tmp_com = files[files_pos];
      for (size_t i = 0; i < files.size(); i++){
        for (size_t j = 0;j < tmp_com.size(); j++){
          if (files[i][j] == tmp_com[j]){
            tmp2_com.append(tmp_com,j,1);
          }
          else break;
        }
        tmp_com = tmp2_com;
        tmp2_com.clear();
        if (tmp_com.empty()) break;
      }      
    }
//----------------------------------------------------------------------
//    判斷有多少檔案match prefix(共有count個match)
//----------------------------------------------------------------------
          vector <string> match_files;
          size_t j = prefix.size();
          int count = 0; 
          for (size_t i = 0; i < files.size(); i++){
            if (prefix == files[i].substr(0,j)){
              count++; match_files.push_back(files[i]);
            }
          } 
//----------------------------------------------------------------------
//    判斷所有的match file裡面都有common_prefix (共同項 string mf_com)
//----------------------------------------------------------------------
  size_t mf_size = 0;int mf_pos;
  if (!match_files.empty()) {mf_size = match_files[0].size(); mf_pos = 0;}
          string mf_com; string mf2_com = "";
          if (match_files.size() > 1){
            for (size_t i = 1; i < match_files.size(); i++){
              if (match_files[i].size() < mf_size) {
                mf_size = match_files[i].size();
                mf_pos = i;
              }
            }
            mf_com = match_files[mf_pos]; 
            for (size_t i = 0; i < match_files.size(); i++){
              for (size_t j = 0; j < mf_com.size(); j++){
                if (match_files[i][j] == mf_com[j]){
                  mf2_com.append(mf_com,j,1);
                }
                else break;
              }
              mf_com = mf2_com;
              mf2_com.clear();
              if (mf_com.empty()) break;
            }      
          }
//6. FIRST WORD ALREADY MATCHED ON SECOND AND LATER TAB PRESSING
  //--- 6.1 ---
        if (prefix.empty()){
    //--- 6.1.1 ---
          if (files.size() > 1 && tmp_com.size() == 0 && all_val){
            cerr << endl;
            for(size_t i=0; i<files.size(); i++){
                cout << setw(16) << left << files[i];
                if (i%5 ==4) cerr << endl;
            }
            reprintCmd();
          }
    //--- 6.1.2 ---
          else if (files.size() > 1 && tmp_com.size() != 0 && all_val){
            cerr << tmp_com;
            mybeep();
          }  
    //--- 6.1.3 ---
           else if(files.size() == 1 && all_val){
               cout << files[0] << " " << endl;
               reprintCmd();
           }
           else mybeep();
        }
        else {
  //--- 6.2 --- 
            if ((all_val && count > 1) && (mf_com.size() == 0 || mf_com == prefix)){
              cerr << endl;
              for (size_t i = 0; i < match_files.size(); i++){
                cerr << setw(16) << left << match_files[i];
                if (i%5 == 4) cerr << endl;
              }
              reprintCmd();
            }
  //--- 6.3 ---
            else if (all_val && count > 1 && mf_com.size() >=1){
              //cerr << "in" << endl;
              string print = mf_com.substr(prefix.size(),(mf_com.size()-prefix.size()));
              for (size_t i = 0; i < print.size(); i++){insertChar(print[i]);}
              mybeep(); 
            }
  //--- 6.4 ---
            else if (all_val && count == 1){
              string print = match_files[0].substr(prefix.size(),(match_files[0].size()-prefix.size()));
              for (size_t i = 0; i < print.size(); i++){insertChar(print[i]);}
              insertChar(' ');
            }
  //--- 6.5 ---
            else if (all_val && count == 0) { mybeep();}
            else { mybeep();}
          }
      }
//4. NO MATCH IN FITST WORD
//7. FIRST WORD NO MATCH
      else {mybeep();}
   }
}


// cmd is a copy of the original input
//
// return the corresponding CmdExec* if "cmd" matches any command in _cmdMap
// return 0 if not found.
//
// Please note:
// ------------
// 1. The mandatory part of the command string (stored in _cmdMap) must match
// 2. The optional part can be partially omitted.
// 3. All string comparison are "case-insensitive".
//
CmdExec*
CmdParser::getCmd(string cmd)
{
   CmdExec* e = 0;
   for (CmdMap::iterator it = _cmdMap.begin(); it != _cmdMap.end(); it++){
     string tmp = (*it).first + (*it).second->getOptCmd();
     if (cmd.size() > tmp.size()) continue;
     if (0 == myStrNCmp(tmp,cmd,(*it).first.size())) return e = (*it).second;
   }
   // TODO...
   return e;
}


//----------------------------------------------------------------------
//    Member Function for class CmdExec
//----------------------------------------------------------------------
// return false if option contains an token
bool
CmdExec::lexNoOption(const string& option) const
{
   string err;
   myStrGetTok(option, err);
   if (err.size()) {
      errorOption(CMD_OPT_EXTRA, err);
      return false;
   }
   return true;
}

// Return false if error options found
// "optional" = true if the option is optional XD
// "optional": default = true
//
bool
CmdExec::lexSingleOption
(const string& option, string& token, bool optional) const
{
   size_t n = myStrGetTok(option, token);
   if (!optional) {
      if (token.size() == 0) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
   }
   if (n != string::npos) {
      errorOption(CMD_OPT_EXTRA, option.substr(n));
      return false;
   }
   return true;
}

// if nOpts is specified (!= 0), the number of tokens must be exactly = nOpts
// Otherwise, return false.
//
bool
CmdExec::lexOptions
(const string& option, vector<string>& tokens, size_t nOpts) const
{
   string token;
   size_t n = myStrGetTok(option, token);
   while (token.size()) {
      tokens.push_back(token);
      n = myStrGetTok(option, token, n);
   }
   if (nOpts != 0) {
      if (tokens.size() < nOpts) {
         errorOption(CMD_OPT_MISSING, "");
         return false;
      }
      if (tokens.size() > nOpts) {
         errorOption(CMD_OPT_EXTRA, tokens[nOpts]);
         return false;
      }
   }
   return true;
}

CmdExecStatus
CmdExec::errorOption(CmdOptionError err, const string& opt) const
{
   switch (err) {
      case CMD_OPT_MISSING:
         cerr << "Error: Missing option";
         if (opt.size()) cerr << " after (" << opt << ")";
         cerr << "!!" << endl;
      break;
      case CMD_OPT_EXTRA:
         cerr << "Error: Extra option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_ILLEGAL:
         cerr << "Error: Illegal option!! (" << opt << ")" << endl;
      break;
      case CMD_OPT_FOPEN_FAIL:
         cerr << "Error: cannot open file \"" << opt << "\"!!" << endl;
      break;
      default:
         cerr << "Error: Unknown option error type!! (" << err << ")" << endl;
      exit(-1);
   }
   return CMD_EXEC_ERROR;
}

