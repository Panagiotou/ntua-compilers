#include "../error.h"
#include "symbol.hpp"
#include <cstring>
#include <iostream>
#include <vector>

#define DEBUG true

inline std::ostream& operator<<(std::ostream &out, Types t) {
  switch (t) {
  case TYPE_INTEGER: out << "int"; break;
  case TYPE_BOOLEAN: out << "bool"; break;
  case TYPE_REAL: out << "real"; break;
  case TYPE_ARRAY: out << "array known length"; break;
  case TYPE_IARRAY: out << "array unknown length"; break;
  case TYPE_CHAR: out << "char"; break;
  case TYPE_STRING: out << "string"; break;
  case TYPE_POINTER: out << "pointer"; break;
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
  bool type_check(Types t) {
    sem();
    if (type != t) {
      return 0;
    }
    else{
      return 1;
    }
  }
  Types type;
};

class Expr_list: public AST{
public:
  Expr_list(): expr_list(){}
  ~Expr_list() {
    for (Expr *e : expr_list) delete e;
  }
  void append_expr(Expr *e) { expr_list.push_back(e); }
  virtual void printOn(std::ostream &out) const override {
    out << "Expr_list(";
    bool first = true;
    for (Expr *e : expr_list) {
      if (!first) out << ", ";
      first = false;
      e->printOn(out);
    }
    out << ")";
  }
private:
   std::vector<Expr *> expr_list;
};

class Call: public AST{
public:
  Call(char *i, Expr_list *e = nullptr){
    id = i;
    expr_list = e;
  }
  ~Call(){
    delete id; delete expr_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Call(";
    out << id;
    expr_list->printOn(out);
    out << ")";
  }
private:
  char *id;
  Expr_list *expr_list;
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
    //Types type = e->type;
    offset = e->offset;
  }
private:
  char *var;
  int offset;
};




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

class Type: public AST{
protected:
  Types val;
};

class Integer: public Type{
public:
  Integer(): val(TYPE_INTEGER){}
  virtual void printOn(std::ostream &out) const override {
    out << "Integer()";
  }
private:
  Types val;
};
class String: public Type{
public:
  String(): val(TYPE_STRING){}
  virtual void printOn(std::ostream &out) const override {
    out << "String()";
  }
private:
  Types val;
};
class Char: public Type{
public:
  Char(): val(TYPE_CHAR){}
  virtual void printOn(std::ostream &out) const override {
    out << "Char()";
  }
private:
  Types val;
};
class Real: public Type{
public:
  Real(): val(TYPE_REAL){}
  virtual void printOn(std::ostream &out) const override {
    out << "Real()";
  }
private:
  Types val;
};
class Boolean: public Type{
public:
  Boolean(): val(TYPE_BOOLEAN){}
  virtual void printOn(std::ostream &out) const override {
    out << "Boolean()";
  }
private:
  Types val;
};

class Constint: public Rval {
public:
  Constint(int c): con(c) {     if(DEBUG){
        this->printOn(std::cout);
      }
    type = new Integer();}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(";
    out << con ;
    out << ")";
  }
  virtual int eval() const override { return con; }
  virtual void sem() override { type = new Integer(); }
  Type *type;
  virtual int get(){
    return con;
  }
private:
  int con;
};

class Array: public Type{
public:
  Array(Type *t, Constint *s = nullptr){
    val = TYPE_ARRAY;
    if(s != nullptr) size = s->get();
    else size = -1;
    oftype = t;
  }
  virtual void printOn(std::ostream &out) const override {
    if(size > 0){
      out << "Array(";
      out << "of size: " << size;
      out << " and type:"; oftype->printOn(out);
      out << ")";
    }
    else{
      out << "Array(";
      out << " of type:"; oftype->printOn(out);
      out << ")";
    }
  }
private:
  Type *oftype;
  int size;
};

class Pointer: public Type{
public:
  Pointer(Type *t){
    val = TYPE_POINTER;
    oftype = t;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Pointer(";
    out << " of type:"; oftype->printOn(out);
    out << ")";
  }
private:
  Type *oftype;
};



class Constchar: public Rval {
public:
  Constchar(char c): con(c) {     if(DEBUG){
        this->printOn(std::cout);
      }
    type = new Char();}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  virtual void sem() override { type = new Char(); }
  Type *type;
private:
  char con;
};

class Conststring: public Lval {
public:
  Conststring(char *c): con(c) {     if(DEBUG){
        this->printOn(std::cout);
      }
    type = new String();}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  virtual void sem() override { type = new String(); }
  Type *type;
private:
  char *con;
};

class Constreal: public Rval {
public:
  Constreal(double c): con(c) {     if(DEBUG){
        this->printOn(std::cout);
      }
    type = new Real();}
  virtual void printOn(std::ostream &out) const override {
    out << "Const(" << con << ")";
  }
  virtual int eval() const override { return 0; } //wrong
  virtual void sem() override { type = new Real(); }
  Type *type;
private:
  double con;
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
    stmt_list->printOn(out);
    out << ")";
  }
private:
  Stmt_list *stmt_list;
};

class Header: public AST{
};

class Id;

class Id_list: public AST{
public:
  Id_list(): id_list(){}
  ~Id_list() {
    for (char *id : id_list) delete id;
  }
  void append_id(char *id) { id_list.push_back(id); }
  virtual void printOn(std::ostream &out) const override {
    out << "Id_list(";
    bool first = true;
    for (char *id : id_list) {
      if (!first) out << ", ";
      first = false;
      out << id;
    }
    out << ")";
  }
private:
   std::vector<char *> id_list;
};


class Label: public AST{
public:
  Label(Id_list *i_l){
    id_list = i_l;
  };
  ~Label(){
    delete id_list;
  };
  virtual void printOn(std::ostream &out) const override {
    out << "Label(";
    id_list->printOn(out);
    out << ")";
  };
private:
  Id_list *id_list;
};

class Decl: public AST {
public:
  Decl(Id_list *i_list, Type *t){
    id_list = i_list;
    type = t;
  }

  virtual void printOn(std::ostream &out) const override {
    out << "Decl(";
    id_list->printOn(out);
     out << " : " << type << ")";
  }
private:
  Id_list *id_list;
  Type *type;
};

class Decl_list: public AST{
public:
  Decl_list(): decl_list(){}
  ~Decl_list() {
    for (Decl *d : decl_list) delete d;
  }
  void append_decl(Decl *d) { decl_list.push_back(d); }
  virtual void printOn(std::ostream &out) const override {
    out << "Decl_list(";
    bool first = true;
    for (Decl *d : decl_list) {
      if (!first) out << ", ";
      first = false;
      d->printOn(out);
    }
    out << ")";
  }
private:
   std::vector<Decl *> decl_list;
};


class Formal: public AST {
public:
  Formal(Id_list *i_list, Type *t){
    id_list = i_list;
    type = t;
  }

  virtual void printOn(std::ostream &out) const override {
    out << "Formal(";
    id_list->printOn(out);
     out << " : " << type << ")";
  }
private:
  Id_list *id_list;
  Type *type;
};

class Formal_list: public AST{
public:
  Formal_list(): formal_list(){}
  ~Formal_list() {
    for (Formal *f : formal_list) delete f;
  }
  void append_formal(Formal *f) { formal_list.push_back(f); }
  virtual void printOn(std::ostream &out) const override {
    out << "Formal_list(";
    bool first = true;
    for (Formal *f : formal_list) {
      if (!first) out << ", ";
      first = false;
      f->printOn(out);
    }
    out << ")";
  }
private:
   std::vector<Formal *> formal_list;
};

class Procedure: public Header{
public:
  Procedure(char *i, Formal_list *f = nullptr){
    id = i;
    formal_list = f;
  }
  ~Procedure(){
    delete id; delete formal_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Procedure(";
    out << id;
    formal_list->printOn(out);
    out << ")";
  }
private:
  char *id;
  Formal_list *formal_list;
};

class Function: public Header{
public:
  Function(char *i, Type *t, Formal_list *f = nullptr){
    id = i;
    type = t;
    formal_list = f;
  }
  ~Function(){
    delete id; delete formal_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Function(";
    out << id;
    formal_list->printOn(out);
    out << type;
    out << ")";
  }
private:
  char *id;
  Type *type;
  Formal_list *formal_list;
};

class Body;


class Local: public AST{
public:
  Local(Decl_list *d){
    // "var"
    decl_list = d;
    localType = "var";
  };
  Local(Label *l){
    // label
    label = l;
    localType = "label";
  };
  Local(Header *h, Body *b){
    // function or procedure
    header = h;
    body = b;
    localType = "forp";
  };
  Local(Header *h){
    // forward
    header = h;
    localType = "forward";
  };
  ~Local(){};
  virtual void printOn(std::ostream &out) const override {
    out << "Local()";
  };
private:
  Decl_list *decl_list;
  Label *label;
  Header *header;
  Body *body;
  char const *localType;
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
    delete b;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Body(";
    bool first = true;
    for (Local *l : local_list) {
      if (!first) out << ", ";
      first = false;
      l->printOn(out);
    }
    out << *block;
    out << ")";
  }
private:
  std::vector<Local *> local_list;
  Block *block;
};
