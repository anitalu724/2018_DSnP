/****************************************************************************
  FileName     [ cmdReader.cpp ]
  PackageName  [ cmd ]
  Synopsis     [ Define command line reader member functions ]
  Author       [ Chung-Yang (Ric) Huang ]
  Copyright    [ Copyleft(c) 2007-2012 LaDs(III), GIEE, NTU, Taiwan ]
****************************************************************************/
#include <cassert>
#include <cstring>
#include "cmdParser.h"

using namespace std;

//----------------------------------------------------------------------
//    Extrenal funcitons
//----------------------------------------------------------------------
void mybeep();
char mygetc(istream&);
ParseChar checkChar(char, istream&);


//----------------------------------------------------------------------
//    Member Function for class Parser
//----------------------------------------------------------------------
void
CmdParser::readCmd()
{
   if (_dofile.is_open()) {
      readCmdInt(_dofile);
      _dofile.close();
   }
   else
      readCmdInt(cin);
}

void
CmdParser::readCmdInt(istream& istr)
{
   resetBufAndPrintPrompt();

   while (1) {
      char ch = mygetc(istr);
      ParseChar pch = checkChar(ch, istr);
      
      if (pch == INPUT_END_KEY) break;
      switch (pch) {
         case LINE_BEGIN_KEY :
         case HOME_KEY       : moveBufPtr(_readBuf); break;
         case LINE_END_KEY   :
         case END_KEY        : moveBufPtr(_readBufEnd); break;
         case BACK_SPACE_KEY : /* TODO */ if (moveBufPtr(_readBufPtr - 1)) deleteChar();
                               break;  
         case DELETE_KEY     : deleteChar(); break;
         case NEWLINE_KEY    : addHistory();
                               cout << char(NEWLINE_KEY);
                               printPrompt(); break;
         case ARROW_UP_KEY   : moveToHistory(_historyIdx - 1); break;
         case ARROW_DOWN_KEY : moveToHistory(_historyIdx + 1); break;
         case ARROW_RIGHT_KEY: /* TODO */ moveBufPtr(_readBufPtr + 1); break;
         case ARROW_LEFT_KEY : /* TODO */ moveBufPtr(_readBufPtr - 1); break;
         case PG_UP_KEY      : moveToHistory(_historyIdx - PG_OFFSET); break;
         case PG_DOWN_KEY    : moveToHistory(_historyIdx + PG_OFFSET); break;
         case TAB_KEY        : /* TODO */ for (int i = 0; i < TAB_POSITION-((_readBufPtr-_readBuf) % TAB_POSITION); i++) cerr << char(32); break;
         case INSERT_KEY     : // not yet supported; fall through to UNDEFINE
         case UNDEFINED_KEY  : mybeep(); break;
         default:  // printable character
            insertChar(char(pch)); break;
      }
      #ifdef TA_KB_SETTING
      taTestOnly();
      #endif
   }
}


// This function moves _readBufPtr to the "ptr" pointer
// It is used by left/right arrowkeys, home/end, etc.
//
// Suggested steps:
// 1. Make sure ptr is within [_readBuf, _readBufEnd].
//    If not, make a beep sound and return false. (DON'T MOVE)
// 2. Move the cursor to the left or right, depending on ptr
// 3. Update _readBufPtr accordingly. The content of the _readBuf[] will
//    not be changed
//
// [Note] This function can also be called by other member functions below
//        to move the _readBufPtr to proper position.
bool
CmdParser::moveBufPtr(char* const ptr)
{
   // TODO...
   if ((_readBufPtr == _readBufEnd && ptr == _readBufEnd) || (_readBufPtr == _readBufEnd && ptr == _readBufPtr + 1) || ((_readBufPtr == _readBuf) && (ptr == _readBuf)) || ((_readBufPtr == _readBuf) && (ptr == _readBufPtr - 1))) {
      mybeep();
      return false;
    }
   else if (ptr == _readBufEnd){
        for (int i = 0;i < (_readBufEnd - _readBufPtr); i++) cerr << char(32);
        for (int i = 0; i < (_readBufEnd - _readBuf); i++) cerr << char(8);
        for (int i = 0; i < (_readBufEnd - _readBuf); i++) cerr << _readBuf[i];
        _readBufPtr = _readBufEnd;
        return true;
    }
    else if (ptr == _readBuf){
        for (int i = 0; i < (_readBufPtr - _readBuf); i++) cerr << char(8);
        _readBufPtr = _readBufPtr - (_readBufPtr - _readBuf);
        return true;
    }
    else if (ptr == _readBufPtr - 1) {
        cerr << char(8);
        _readBufPtr--;
        return true;
    }
    else if (ptr == _readBufPtr + 1){     
        for (int i = 0; i < (_readBufPtr - _readBuf); i++) cerr << char(8);
        for (int i = 0; i < (_readBufEnd - _readBuf); i++) cerr << _readBuf[i];
        for (int i = 0; i < (_readBufEnd - _readBufPtr - 1); i++) cerr << char(8);
       _readBufPtr = ptr;
        return true;
    }  
} 
// [Notes]
// 1. Delete the char at _readBufPtr
// 2. mybeep() and return false if at _readBufEnd
// 3. Move the remaining string left for one character
// 4. The cursor should stay at the same position
// 5. Remember to update _readBufEnd accordingly.
// 6. Don't leave the tailing character.
// 7. Call "moveBufPtr(...)" if needed.
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling deleteChar()---
//
// cmd> This is he command
//              ^
//
bool
CmdParser::deleteChar()
{
   // TODO...
   if (_readBufEnd == _readBufPtr){
     mybeep();
     return false;
   }
   else{
     for (int i = (_readBufPtr - _readBuf); i < (_readBufEnd - _readBuf); i++) _readBuf[i] = _readBuf[i+1];
     for (int i = 0; i < (_readBufPtr - _readBuf); i++) cerr << char(8);
     for (int i = 0; i < (_readBufEnd - _readBuf); i++) cerr << _readBuf[i];
     cerr << char(32);
     for (int i = 0; i < (_readBufEnd - _readBufPtr); i++) cerr << char(8);
     _readBufEnd--;
     *_readBufEnd = 0;
     return true;
   }
   
}

// 1. Insert character 'ch' at _readBufPtr
// 2. Move the remaining string right for one character
// 3. The cursor should move right for one position afterwards
//
// For example,
//
// cmd> This is the command
//              ^                (^ is the cursor position)
//
// After calling insertChar('k') ---
//
// cmd> This is kthe command
//               ^
//
void
CmdParser::insertChar(char ch, int rep)
{
   // TODO...
   /*cerr << "(" << (void*)_readBufEnd << "," << (void*)_readBufPtr << "," << (void*)_readBuf << ")";
   cerr << "(" << *_readBufEnd << "," << *_readBufPtr << "," << *_readBuf << ")";
   cerr << "(" << &_readBufEnd << "," << &_readBufPtr << "," << &_readBuf << ")";*/
   if (_readBufEnd - _readBufPtr != 0){
      for (int i = (_readBufEnd - _readBuf); i > (_readBufPtr - _readBuf); i--){
          _readBuf[i] = _readBuf[i-1];
      }
   }
  *(_readBufPtr) = ch;
  _readBufEnd++;
  *(_readBufEnd) = 0;

  for (int i = 0; i < (_readBufPtr - _readBuf); i++) cerr << char(8);
  for (int i = 0; i < (_readBufEnd - _readBuf); i++)  cerr << _readBuf[i];
  _readBufPtr++; 
  for (int i = 0; i < (_readBufEnd - _readBufPtr); i++) cerr << char(8);
}

// 1. Delete the line that is currently shown on the screen
// 2. Reset _readBufPtr and _readBufEnd to _readBuf
// 3. Make sure *_readBufEnd = 0
//
// For example,
//
// cmd> This is t he command
//              ^                (^ is the cursor position)
//
// After calling deleteLine() ---
//
// cmd>
//      ^
//
void
CmdParser::deleteLine()
{
   // TODO...
   for (int i = 0; i < (_readBufPtr - _readBuf); i++) cout << char(8);
   for (int i = 0; i < (_readBufEnd - _readBuf); i++) cout << " ";
   for (int i = 0; i < (_readBufEnd - _readBuf); i++) cout << char(8);
    _readBufEnd = _readBufPtr = _readBuf;
   *_readBufEnd = 0;
}
// This functions moves _historyIdx to index and display _history[index]
// on the screen.
//
// Need to consider:
// If moving up... (i.e. index < _historyIdx)
// 1. If already at top (i.e. _historyIdx == 0), beep and do nothing.
// 2. If at bottom, temporarily record _readBuf to history.
//    (Do not remove spaces, and set _tempCmdStored to "true")
// 3. If index < 0, let index = 0.
//
// If moving down... (i.e. index > _historyIdx)
// 1. If already at bottom, beep and do nothing
// 2. If index >= _history.size(), let index = _history.size() - 1.
//
// Assign _historyIdx to index at the end.
//
// [Note] index should not = _historyIdx
//

void
CmdParser::moveToHistory(int index)
{
   // TODO...
   if (_historyIdx == _history.size()) _tempCmdStored = false;
   if (((index == _historyIdx + 1) && (_historyIdx == _history.size())) || 
       ((index == _historyIdx + PG_OFFSET) && (_historyIdx == _history.size())) ||
       ((index == _historyIdx - 1) && (_historyIdx == 0)) || 
       ((index == _historyIdx - PG_OFFSET) && (_historyIdx == 0))) mybeep();

   else {
     if (index == _historyIdx - 1){        //arrow_up
      _tempCmdStored = true;
        if(_historyIdx == _history.size()){
          string tempStored = _readBuf;
          _history.push_back(tempStored);
        }
        _historyIdx = index;
        retrieveHistory();
     }
     else if (index == _historyIdx + 1){      //arrow_down
        if (index >= _history.size()) index = _history.size() - 1;
        _historyIdx = index;
        retrieveHistory();
        if (index == _history.size() - 1){
          _tempCmdStored = false;
          _history.pop_back();
        }
     }
     else if (index == _historyIdx - PG_OFFSET){
        _tempCmdStored = true;
        if(_historyIdx == _history.size()){
          string tempStored = _readBuf;
          _history.push_back(tempStored);
        }
        if (_historyIdx - PG_OFFSET < 0){
          _historyIdx = 0;
          retrieveHistory();
        }
        else {
          _historyIdx = index;
          retrieveHistory();
        }
        
     }
     else if (index == _historyIdx + PG_OFFSET){
       if (index > _history.size()){
         _historyIdx = _history.size() - 1;
         retrieveHistory();
         _tempCmdStored = false;
         _history.pop_back();
       }
       else {
         _historyIdx = index;
         retrieveHistory();
         _tempCmdStored = false;
         _history.pop_back();
       }
     }
   }
}


// This function adds the string in _readBuf to the _history.
// The size of _history may or may not change. Depending on whether 
// there is a temp history string.
//
// 1. Remove ' ' at the beginning and end of _readBuf
// 2. If not a null string, add string to _history.
//    Be sure you are adding to the right entry of _history.
// 3. If it is a null string, don't add anything to _history.
// 4. Make sure to clean up "temp recorded string" (added earlier by up/pgUp,
//    and reset _tempCmdStored to false
// 5. Reset _historyIdx to _history.size() // for future insertion
// 6. Reset _readBufPtr and _readBufEnd to _readBuf
// 7. Make sure *_readBufEnd = 0 ==> _readBuf becomes null string
//
void
CmdParser::addHistory()
{
   // TODO...
   //if (_readBuf == NULL) cerr << "NULL";
   if (*_readBuf != 0) {
      string tmp = _readBuf;
      //int counter1 = 0;
      int size = tmp.size();
      //bool null_test = true;

//NULL_TEST_BACKWARD:      
        for (int i = tmp.size();;i--) {
          if (tmp[i-1] == ' ') {
            tmp.pop_back();
            size--;
          }
          else goto NULL_TEST_FORWARD;
        }
NULL_TEST_FORWARD:
    //size -= counter1;
    for (int i = 0; i < size; i++){
      if (tmp[i] == ' '){
        for (int j = 0; j < size; j++){
          tmp[j] = tmp[j+1];
        }
        tmp.pop_back();
        size--;
      }
      else goto TEST_END;
    }
TEST_END:
  //tmp.size() = size;
      if (_tempCmdStored) _history.pop_back();
      _history.push_back(tmp);
      *_readBuf = 0; 
      tmp = "";
      
      _tempCmdStored = false;
      _historyIdx = _history.size();
      _readBufEnd = _readBufPtr = _readBuf;
      *_readBufEnd = 0;

      

   }
   

}


// 1. Replace current line with _history[_historyIdx] on the screen
// 2. Set _readBufPtr and _readBufEnd to end of line
//
// [Note] Do not change _history.size().
//
void
CmdParser::retrieveHistory()
{
   deleteLine();
   strcpy(_readBuf, _history[_historyIdx].c_str());
   cout << _readBuf;
   _readBufPtr = _readBufEnd = _readBuf + _history[_historyIdx].size();
}

