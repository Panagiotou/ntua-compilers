#pragma once
#include <iostream>
#include <cstdlib>
#include <vector>
#include <map>
#include "Type.hpp"


struct SymbolEntry {
  Type *type;
  int offset;
  std::string s;
  SymbolEntry() {}
  SymbolEntry(Type *t, int ofs, std::string c) : type(t), offset(ofs), s(c){}
};

class Formal_list;
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
  void insert(std::string c, Type *t) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
          print();
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, c);
    ++size;
    procedures[c] = false;
    procedureFormals[c] = nullptr;
    functions[c] = false;
    functionFormals[c] = nullptr;
  }
  void insertProcedure(std::string c, Type *t, Formal_list *f) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, c);
    ++size;
    procedures[c] = true;
    procedureFormals[c] = f;
    functionFormals[c] = nullptr;

  }
  bool isProcedure(std::string c){
    return procedures[c];
  }
  void insertFunction(std::string c, Type *t, Formal_list *f) {
    if (locals.find(c) != locals.end()) {
      std::cerr << "Duplicate variable " << c << std::endl;
      exit(1);
    }
    locals[c] = SymbolEntry(t, offset++, c);
    ++size;
    functions[c] = true;
    functionFormals[c] = f;
    procedureFormals[c] = nullptr;

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
    for(auto it = locals.cbegin(); it != locals.cend(); ++it){
      if(isFunction(it->first)||isProcedure(it->first)){
          return it->first;
      }
    }
    std::cout << "Cant find parrent function!";
    exit(1);
  }
  void makeNew(std::string c){
    isNewV[c] = true;
  }
  bool isNew(std::string c){
    if (isNewV.find(c) == isNewV.end()) return false;
    return true;
  }
  void insertProcedureForward(std::string c, Type *t, Formal_list *f){ insertProcedure(c, t, f); isForwardV[c] = true; }
  void insertFunctionForward(std::string c, Type *t, Formal_list *f){ insertFunction(c, t, f); isForwardV[c] = true; }
  void insertForward(std::string c, Type *t){ insert(c, t); isForwardV[c] = true; }
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
private:
  std::map<std::string , SymbolEntry> locals;
  std::map<std::string, bool> isNewV;
  std::map<std::string, bool> isForwardV;
  std::map<std::string , bool> procedures;
  std::map<std::string , Formal_list *> procedureFormals;
  std::map<std::string , bool> functions;
  std::map<std::string , Formal_list *> functionFormals;

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
  void insert(std::string c, Type *t) { scopes.back().insert(c, t); }
  bool isProcedure(std::string c){
    return scopes.back().isProcedure(c);
  }
  void insertProcedure(std::string c, Type *t, Formal_list *f) { scopes.back().insertProcedure(c, t, f); }
  bool isFunction(std::string c){
    return scopes.back().isFunction(c);
  }
  void insertProcedureForward(std::string c, Type *t, Formal_list *f){ scopes.back().insertProcedureForward(c, t, f); }
  void insertFunctionForward(std::string c, Type *t, Formal_list *f){ scopes.back().insertFunctionForward(c, t, f); }
  void insertForward(std::string c, Type *t){ scopes.back().insertForward(c, t); }

  void insertFunction(std::string c, Type *t, Formal_list *f) { scopes.back().insertFunction(c, t, f); }

  std::string getParentFunction(){
    std::string s;
    if(scopes.size() == 1){
      s = scopes.back().getParentFunction();
      return s;
    }
    if(scopes.size()-2 >= 0){
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
  bool isForward(std::string c){
    return scopes.back().isForward(c);
  }
  void removeForward(std::string c){
    scopes.back().removeForward(c);
  }
  bool isemptyForward(){
    return scopes.back().isemptyForward();
  }
  std::vector<std::string> getForPForward(){
    return scopes.back().getForPForward();
  }
private:
  std::vector<Scope> scopes;
};

extern SymbolTable st;
