#pragma once
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include "OurType.hpp"


struct SymbolEntry {
  OurType *type;
  int offset;
  std::string s;
  SymbolEntry() {}
  SymbolEntry(OurType *t, int ofs, std::string c) : type(t), offset(ofs), s(c){}
};

class Formal_list;
class Stmt;
class Scope {
public:
  Scope() : locals(), offset(-1), size(0) {}
  Scope(int ofs) : locals(), offset(ofs), size(0) {}
  int getOffset() const { return offset; }
  int getSize() const { return size; }
  SymbolEntry *lookup(std::string c) {
    if (locals.find(c) == locals.end()) return nullptr;
    return &(locals[c]);
  }
  void insert(std::string c, OurType *t) {
    if (locals.find(c) != locals.end()) {
      print();
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, c);
    ++size;
    procedures[c] = false;
    procedureFormals[c] = nullptr;
    functions[c] = false;
    functionFormals[c] = nullptr;
    label[c] = false;
  }
  void insertLabel(std::string c, OurType *t) {
    if (locals.find(c) != locals.end()) {
      print();
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, c);
    ++size;
    procedures[c] = false;
    procedureFormals[c] = nullptr;
    functions[c] = false;
    functionFormals[c] = nullptr;
    label[c] = true;
  }
  bool isLabel(std::string c){
    return label[c];
  }
  void insertProcedure(std::string c, OurType *t, Formal_list *f) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, c);
    ++size;
    procedures[c] = true;
    procedureFormals[c] = f;
    label[c] = false;
    localForPQueue.push_back(c);

  }
  bool isProcedure(std::string c){
    return procedures[c];
  }
  bool isLib(std::string c){
    return isLibV[c];
  }
  void insertFunction(std::string c, OurType *t, Formal_list *f) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, c);
    ++size;
    functions[c] = true;
    functionFormals[c] = f;
    label[c] = false;
    localForPQueue.push_back(c);
  }
  void printParents(){
    std::cout << "Parents\n";
    for(std::string s : localForPQueue){
      std::cout << "\t" << s <<"\n";
    }
  }
  Formal_list *getFormalsProcedure(std::string c){
    return procedureFormals[c];
  }
  Formal_list *getFormalsFunction(std::string c){
    return functionFormals[c];
  }
  bool isFunction(std::string c){
    return functions[c];
  }
  void print(){
    for(auto it = locals.cbegin(); it != locals.cend(); ++it)
    {
      if(isProcedure(it->first)){
        std::cout << "\t" << it->first << ": " << it->second.type->val << " (is a procedure) \n";
      }
      else if(isFunction(it->first)){
        std::cout << "\t" << it->first << ": " << it->second.type->val << " (is a function) \n";
      }
      else{
        std::cout << "\t" << it->first << ": " << it->second.type->val << "(is a variable) \n";
      }
    }
  }
  std::string getParentFunction(){
    if(localForPQueue.size()>0){
      return localForPQueue.back();
    }
    else{
      std::cout << "Cant find parrent function!";
      exit(1);
    }
  }
  void makeNew(std::string c){
    isNewV[c] = true;
  }
  bool isNew(std::string c){
    if (isNewV.find(c) == isNewV.end()) return false;
    return true;
  }
  void insertProcedureForward(std::string c, OurType *t, Formal_list *f){ insertProcedure(c, t, f); isForwardV[c] = true; }
  void insertFunctionForward(std::string c, OurType *t, Formal_list *f){ insertFunction(c, t, f); isForwardV[c] = true; }
  void insertFunctionLib(std::string c, OurType *t, Formal_list *f){ insertFunction(c, t, f); isLibV[c] = true; }
  void insertProcedureLib(std::string c, OurType *t, Formal_list *f){ insertProcedure(c, t, f); isLibV[c] = true; }
  void insertForward(std::string c, OurType *t){ insert(c, t); isForwardV[c] = true; }
  void removeForward(std::string c){
    std::map<std::string, bool>::iterator it;
    it=isForwardV.find(c);
    isForwardV.erase (it);
  }
  bool isForward(std::string c){
    if (isForwardV.find(c) == isForwardV.end()) return false;
    return true;
  }
  bool isemptyForward(){
    return isForwardV.empty();
  }
  std::vector<std::string> getForPForward(){
    std::map<std::string, bool>::iterator it;
    std::vector<std::string> r;
    for ( it = isForwardV.begin(); it != isForwardV.end(); it++ )
    {
      if(isFunction(it->first) || isProcedure(it->first)){
        r.push_back(it->first);
      }
    }
    return r;
  }
  bool exists(std::string c){
    if (locals.find(c) == locals.end()) return false;
    return true;
  }
  void insertParent(std::string c){
    localForPQueue.push_back(c);
  }
  void insertLabelStmt(std::string c, Stmt *s){
    labelStmt[c] = s;
  }
  bool LabelHasStmt(std::string c){
    if (labelStmt.find(c) == labelStmt.end()) return false;
    return true;
  }
private:
  std::map<std::string , SymbolEntry> locals;
  std::vector<std::string> localForPQueue;
  std::map<std::string, bool> isNewV;
  std::map<std::string, bool> isForwardV;
  std::map<std::string, bool> isLibV;
  std::map<std::string , bool> procedures;
  std::map<std::string , Formal_list *> procedureFormals;
  std::map<std::string , bool> functions;
  std::map<std::string , Formal_list *> functionFormals;
  std::map<std::string , bool> label;
  std::map<std::string , Stmt *> labelStmt;
  int offset;
  int size;
};

class SymbolTable {
public:
  void openScope() {
    int ofs = scopes.empty() ? 0 : scopes.back().getOffset();
    scopes.push_back(Scope(ofs));
  }
  void closeScope() { scopes.pop_back(); };

  SymbolEntry *lookup(std::string c) {
    SymbolEntry *e;
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
        e = i->lookup(c);
        if(e) return e;
    }
    std::cerr << "Unknown variable " << c << std::endl;
    exit(1);
  }

  bool existsResult(){
    SymbolEntry *e;
    e = scopes.back().lookup("result");
    if (e != nullptr){
      return 1;
    }
    return 0;
  }
  bool existsLastScope(std::string c){
    SymbolEntry *e;
    e = scopes.back().lookup(c);
    if (e != nullptr){
      return 1;
    }
    return 0;
  }
  SymbolEntry *getSymbolEntry(std::string c){
    SymbolEntry *e;
    e = scopes.back().lookup(c);
    return e;
  }
  void printScopes(){
    int k = 0;
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      std::cout << "Printing Scope " << k << "\n";
      i->print();
      k++;
    }
  }
  Formal_list *getFormalsProcedureAll(std::string c){
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      if(i->getFormalsProcedure(c)){
              return i->getFormalsProcedure(c);
      }
    }
    return nullptr;
  }
  Formal_list *getFormalsFunctionAll(std::string c){
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      if(i->getFormalsFunction(c)){
              return i->getFormalsFunction(c);
      }
    }
    return nullptr;
  }
  void printLastScope(){
    std::cout << "Printing Scope \n";
    scopes.back().print();
  }
  int getSizeOfCurrentScope() const { return scopes.back().getSize(); }
  void insert(std::string c, OurType *t) { scopes.back().insert(c, t); }
  bool isProcedure(std::string s){
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      if(i->exists(s)) return i->isProcedure(s);
    }
    return false;
  }
  void insertLabel(std::string c, OurType *t) { scopes.back().insertLabel(c, t); }
  void insertProcedure(std::string c, OurType *t, Formal_list *f) { scopes.back().insertProcedure(c, t, f); }
  bool isFunction(std::string s){
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      if(i->exists(s)) return i->isFunction(s);
    }
    return false;
  }
  bool isLib(std::string s){
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      if(i->exists(s)) return i->isLib(s);
    }
    return false;
  }
  void insertProcedureForward(std::string c, OurType *t, Formal_list *f){ scopes.back().insertProcedureForward(c, t, f); }
  void insertFunctionForward(std::string c, OurType *t, Formal_list *f){ scopes.back().insertFunctionForward(c, t, f); }
  void insertForward(std::string c, OurType *t){ scopes.back().insertForward(c, t); }

  void insertFunction(std::string c, OurType *t, Formal_list *f) { scopes.back().insertFunction(c, t, f); }
  void insertFunctionLib(std::string c, OurType *t, Formal_list *f) { scopes.back().insertFunctionLib(c, t, f); }
  void insertProcedureLib(std::string c, OurType *t, Formal_list *f){ scopes.back().insertProcedureLib(c, t, f); }


  std::string getParent(){
    std::string s;
    std::cout<<scopes.size();
    if(scopes.size() == 1){
      s = scopes.back().getParentFunction();
      return s;
    }
    if(scopes.size() >= 2){
      s = scopes[scopes.size() - 2].getParentFunction();
      return s;
    }
    else{
      std::cout << "Cant find parent function\n";
      exit(1);
    }
  }
  Formal_list *getFormalsProcedure(std::string c){
    return scopes.back().getFormalsProcedure(c);
  }
  Formal_list *getFormalsFunction(std::string c){
    return scopes.back().getFormalsFunction(c);
  }
  void makeNew(std::string c){
    scopes.back().makeNew(c);
  }
  bool isNew(std::string c){
    return scopes.back().isNew(c);
  }
  bool isLabel(std::string c){
    return scopes.back().isLabel(c);
  }
  void insertLabelStmt(std::string c, Stmt *s){
    scopes.back().insertLabelStmt(c, s);
  }
  bool isForward(std::string c){
    return scopes.back().isForward(c);
  }
  void removeForward(std::string c){
    scopes.back().removeForward(c);
  }
  bool isemptyForward(){
    return scopes.back().isemptyForward();
  }
  bool LabelHasStmt(std::string s){
    return scopes.back().LabelHasStmt(s);
  }
  std::vector<std::string> getForPForward(){
    return scopes.back().getForPForward();
  }
  void printParents(){
    int k = 0;
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      std::cout << "Printing Parents for Scope " << k << "\n";
      i->printParents();
      k++;
    }
  }
  void insertParent(std::string s){
    findScopeToinsert(s);
  }
  void findScopeToinsert(std::string s){
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      if(i->exists(s)) i->insertParent(s); return;
    }
    std::cout << "Cant find scope of " << s << "\n";
    exit(1);
    return ;
  }
  int getSize(){
    return scopes.size();
  }
private:
  std::vector<Scope> scopes;
};

extern SymbolTable st;
