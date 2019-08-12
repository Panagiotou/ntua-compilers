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
    functions[c] = false;
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
private:
  std::map<std::string , SymbolEntry> locals;
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
    e = scopes.back().lookup(c);
    if (e != nullptr) return e;
    e = scopes.back().lookup("_" + c);
    if (e != nullptr) return e;
    std::cerr << "Unknown variable " << c << std::endl;
    exit(1);
  }
  bool existsResult(){
    SymbolEntry *e;
    e = scopes.back().lookup("result");
    if (e != nullptr){
      return 0;
    }
    return 1;
  }
  void printScopes(){
    int k = 0;
    for (auto i = scopes.rbegin(); i != scopes.rend(); ++i) {
      std::cout << "Printing Scope " << k << "\n";
      i->print();
      k++;
    }
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
  void insertFunction(std::string c, Type *t, Formal_list *f) { scopes.back().insertFunction(c, t, f); }

  Formal_list *getFormalsProcedure(std::string c){
    return scopes.back().getFormalsProcedure(c);
  }
  Formal_list *getFormalsFunction(std::string c){
    return scopes.back().getFormalsFunction(c);
  }
private:
  std::vector<Scope> scopes;
};

extern SymbolTable st;

extern SymbolTable st;
