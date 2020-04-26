#include "../error.h"
#include "symbol.hpp"
#include <cstring>
#include <iostream>
#include <vector>
#include "ast0.hpp"

class Local;

class Stmt: public AST {
public:
  virtual void run() const = 0;
};

class Stmt_list: public AST{
public:
  Stmt_list(): stmt_list(){}
  ~Stmt_list() {
    for (Stmt *s : stmt_list) delete s;
  }
  void append_stmt(Stmt *s) { stmt_list.push_back(s); }
  virtual void printOn(std::ostream &out) const override {
    out << "Stmt_list(";
    bool first = true;
    for (Stmt *s : stmt_list) {
      if (!first) out << ", ";
      first = false;
      s->printOn(out);
    }
    out << ")";
  }
private:
   std::vector<Stmt *> stmt_list;
};

class Block: public AST{
public:
  Block(Stmt_list *s){
    stmt_list = s;
  }
  ~Block(){
    delete stmt_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Block(";
    //stmt_list->printOn(out);
    out << ")";
  }
private:
  Stmt_list *stmt_list;
};

class Body: public AST{
public:
  Body(): local_list(), block(){}
  ~Body() {
    for (Local *l : local_list) delete l;
    delete block;
  }
  void append_local(Local *l) { local_list.push_back(l); }
  virtual void sem() override {
    st.openScope();
    for (Local *l : local_list) l->sem();
    block->sem();
    st.closeScope();
  }
  void merge(Block *b) {
    block = b;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Body(";
    bool first = true;
    for (Local *l : local_list) {
      if (!first) out << ", ";
      first = false;
      l->printOn(out);
    }
    if(block) block->printOn(out);
    out << ")";
  }
private:
  std::vector<Local *> local_list;
  Block *block;
};
