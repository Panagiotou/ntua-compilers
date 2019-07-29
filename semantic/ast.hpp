#include "../error.h"
#include "symbol.hpp"
#include <cstring>
#include <iostream>

#define DEBUG true

inline std::ostream& operator<<(std::ostream &out, Type t) {
  switch (t) {
  case TYPE_INTEGER: out << "int"; break;
  case TYPE_BOOLEAN: out << "bool"; break;
  case TYPE_REAL: out << "real"; break;
  case TYPE_ARRAY: out << "array known length"; break;
  case TYPE_IARRAY: out << "array unknown length"; break;
  case TYPE_CHAR: out << "char"; break;
  case TYPE_STRING: out << "string"; break;
  }
  return out;
}

class AST {
public:
  virtual ~AST() {}
  virtual void printOn(std::ostream &out) const = 0;
  virtual void sem() {}
  void ERROR (const char * fmt, ...){
    std::cout << fmt ;
  };
};

extern std::vector<int> rt_stack;

inline std::ostream& operator<<(std::ostream &out, const AST &t) {
  t.printOn(out);
  return out;
}


class Expr: public AST {
public:
  virtual int eval() const = 0;
  bool type_check(Type t) {
    sem();
    if (type != t) {
      return 0;
    }
    else{
      return 1;
    }
  }
  Type type;
};

class Rval: public Expr {};
class Lval: public Expr {};

class BinOp: public Rval {
public:
  BinOp(Expr *l, char *o, Expr *r): left(l), op(o), right(r) {
    if(DEBUG){
      this->printOn(std::cout);
    }
   }
  ~BinOp() { delete left; delete right; }
  virtual void printOn(std::ostream &out) const override {
    out << op << "(" << *left << ", " << *right << ")";
  }
  virtual bool check_number(Expr *left, Expr *right){
    return ((left->type_check(TYPE_REAL) && right->type_check(TYPE_INTEGER))||(left->type_check(TYPE_INTEGER) && right->type_check(TYPE_REAL))||(left->type_check(TYPE_REAL) && right->type_check(TYPE_REAL))||(left->type_check(TYPE_INTEGER) && right->type_check(TYPE_INTEGER)));
  }
  virtual void sem() override {
    if(! strcmp(op, "+") || ! strcmp(op, "*") || ! strcmp(op, "-")){
      if( left->type_check(TYPE_INTEGER) && right->type_check(TYPE_INTEGER)){
        type = TYPE_INTEGER;
      }
      else if(left->type_check(TYPE_REAL) || right->type_check(TYPE_REAL)){
        type = TYPE_REAL;
      }
      else{
        ERROR("Type mismatch!"); exit(1);
      }
    }
    if(! strcmp(op, "/")){
      if(check_number(left, right)){
        type = TYPE_REAL;
      }
      else{
        ERROR("Type mismatch!"); exit(1);
      }
    }
    if(! strcmp(op, "mod") || ! strcmp(op, "div")){
      if( left->type_check(TYPE_INTEGER) && right->type_check(TYPE_INTEGER)){
        type = TYPE_INTEGER;
      }
      else{
        ERROR("Type mismatch!"); exit(1);
      }
    }
    if(! strcmp(op, "=") || ! strcmp(op, "<>")){
      if(check_number(left, right) || ((left->type == right->type) && (!left->type_check(TYPE_ARRAY) || !left->type_check(TYPE_IARRAY)))){
        type = TYPE_BOOLEAN;
      }
      else{
        ERROR("Type mismatch!"); exit(1);
      }
    }
    if(! strcmp(op, "<") || ! strcmp(op, ">") || ! strcmp(op, "<=") || ! strcmp(op, ">=")){
      if(check_number(left, right)){
        type = TYPE_BOOLEAN;
      }
      else{
        ERROR("Type mismatch!"); exit(1);
      }
    }
    if(! strcmp(op, "or") || ! strcmp(op, "and")){
      if(left->type_check(TYPE_BOOLEAN) && right->type_check(TYPE_BOOLEAN)){
        type = TYPE_BOOLEAN;
      }
      else{
        ERROR("Type mismatch!"); exit(1);
      }
    }
  }
  virtual int eval() const override {
    if(! strcmp(op, "+")) return left->eval() + right->eval();
    if(! strcmp(op, "-")) return left->eval() - right->eval();
    if(! strcmp(op, "*")) return left->eval() * right->eval();
    if(! strcmp(op, "/")) return left->eval() / right->eval();
    if(! strcmp(op, "=")) return left->eval() == right->eval();
    if(! strcmp(op, "<")) return left->eval() < right->eval();
    if(! strcmp(op, ">")) return left->eval() > right->eval();
    if(! strcmp(op, "<=")) return left->eval() <= right->eval();
    if(! strcmp(op, ">=")) return left->eval() >= right->eval();
    if(! strcmp(op, "<>")) return left->eval() != right->eval();
    if(! strcmp(op, "div")) return left->eval() / right->eval();
    if(! strcmp(op, "mod")) return left->eval() % right->eval();
    if(! strcmp(op, "or")) return left->eval() || right->eval();
    if(! strcmp(op, "and")) return left->eval() && right->eval();
    return 0;  // this will never be reached.
  }
private:
  Expr *left;
  char *op;
  Expr *right;
};


class UnOp: public Rval {
public:
  UnOp(char *o, Expr *r): op(o), right(r) {    if(DEBUG){
        this->printOn(std::cout);
      }}
  ~UnOp() { delete right; }
  virtual void printOn(std::ostream &out) const override {
    out << op << "(" << *right << ")";
  }
  virtual void sem() override {
    if(! strcmp(op, "+") || ! strcmp(op, "-")){
      if(right->type_check(TYPE_INTEGER) || right->type_check(TYPE_REAL)){
        type = right->type;
      }
      else{
        ERROR("Type mismatch!"); exit(1);
      }
    }
    if( ! strcmp(op, "not")){
      if(right->type_check(TYPE_BOOLEAN)){
        type = TYPE_BOOLEAN;
      }
      else{
        ERROR("Type mismatch!"); exit(1);
      }
    }
  }
  virtual int eval() const override {
    if(! strcmp(op, "+")) return  right->eval();
    if(! strcmp(op, "-")) return -right->eval();
    if(! strcmp(op, "not")) return !right->eval();
    return 0;  // this will never be reached.
  }
private:
  char *op;
  Expr *right;
};

class Id: public Lval {
public:
  Id(char *v): var(v), offset(-1){     if(DEBUG){
        this->printOn(std::cout);
      }}
  virtual void printOn(std::ostream &out) const override {
    out << "Id(" << var << "@" << offset << ")";
  }
  virtual int eval() const override {
    return rt_stack[offset];
  }
  virtual void sem() override {
    SymbolEntry *e = st.lookup(var);
    type = e->type;
    offset = e->offset;
  }
private:
  char *var;
  int offset;
};


class Constint: public Rval {
public:
  Constint(int c): con(c) {     if(DEBUG){
        this->printOn(std::cout);
      }}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(" << con << ")";
  }
  virtual int eval() const override { return con; }
  virtual void sem() override { type = TYPE_INTEGER; }
private:
  int con;
};

class Constchar: public Rval {
public:
  Constchar(char c): con(c) {     if(DEBUG){
        this->printOn(std::cout);
      }}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  virtual void sem() override { type = TYPE_CHAR; }
private:
  char con;
};

class Conststring: public Lval {
public:
  Conststring(char *c): con(c) {     if(DEBUG){
        this->printOn(std::cout);
      }}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  virtual void sem() override { type = TYPE_STRING; }
private:
  char *con;
};

class Constreal: public Rval {
public:
  Constreal(double c): con(c) {     if(DEBUG){
        this->printOn(std::cout);
      }}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  virtual void sem() override { type = TYPE_REAL; }
private:
  double con;
};


class Stmt: public AST {
public:
  virtual void run() const = 0;
};

class If: public Stmt {
public:
  If(Expr *c, Stmt *s1, Stmt *s2 = nullptr):
    cond(c), stmt1(s1), stmt2(s2) {     if(DEBUG){
          this->printOn(std::cout);
        }}
  ~If() { delete cond; delete stmt1; delete stmt2; }
  virtual void printOn(std::ostream &out) const override {
    out << "If(" << *cond << ", " << *stmt1;
    if (stmt2 != nullptr) out << ", " << *stmt2;
    out << ")";
  }
  virtual void sem() override {
    if(cond->type_check(TYPE_BOOLEAN)){
      stmt1->sem();
      if (stmt2 != nullptr) stmt2->sem();
    }
    else{
      ERROR("Type mismatch!"); exit(1);
    }
  }
  virtual void run() const override {
    if (cond->eval())
      stmt1->run();
    else if (stmt2 != nullptr)
      stmt2->run();
  }
private:
  Expr *cond;
  Stmt *stmt1;
  Stmt *stmt2;
};

class While: public Stmt {
public:
  While(Expr *e, Stmt *s): expr(e), stmt(s) {     if(DEBUG){
        this->printOn(std::cout);
      }}
  ~While() { delete expr; delete stmt; }
  virtual void printOn(std::ostream &out) const override {
    out << "While(" << *expr << ", " << *stmt << ")";
  }
  virtual void sem() override {
    if(expr->type_check(TYPE_BOOLEAN)){
      stmt->sem();
    }
    else{
      ERROR("Type mismatch!"); exit(1);
    }
  }
  virtual void run() const override {
    while(expr->eval()){
      stmt->run();
    }
  }
private:
  Expr *expr;
  Stmt *stmt;
};
