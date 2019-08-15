#include "../error.h"
#include "../lexer/lexer.hpp"
#include "symbol.hpp"
#include <cstring>
#include <iostream>
#include <vector>



inline std::ostream& operator<<(std::ostream &out, Types t) {
  switch (t) {
  case TYPE_INTEGER: out << "int"; break;
  case TYPE_BOOLEAN: out << "bool"; break;
  case TYPE_REAL: out << "real"; break;
  case TYPE_ARRAY: out << "array known length"; break;
  case TYPE_CHAR: out << "char"; break;
  case TYPE_STRING: out << "string"; break;
  case TYPE_POINTER: out << "pointer"; break;
  case TYPE_PROCEDURE: out << "type procedure"; break;
  case TYPE_NIL: out << "nil"; break;
  case TYPE_RES: out << "res"; break;
  }
  return out;
}



extern std::vector<int> rt_stack;

inline std::ostream& operator<<(std::ostream &out, const AST &t) {
  t.printOn(out);
  return out;
}

class Expr: public AST {
public:
  virtual int eval() const = 0;
  bool type_check(Type *t) {

    if (type == t) {
      return 1;
    }
    else{
      return 0;
    }
  }
  Type *getType(){
    return type;
  }
  Type *type;
  virtual bool isResult(){
      return false;
  }
  bool isNew;
};

class Expr_list: public AST{
public:
  Expr_list(): expr_list(){}
  ~Expr_list() {
    for (Expr *e : expr_list) delete e;
  }
  void append_expr(Expr *e) { expr_list.push_back(e); }
  void append_begin(Expr *e) { expr_list.insert(expr_list.begin(), e); }
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
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Expr_list(";
    bool first = true;
    for (Expr *e : expr_list) {
      if (!first) s +=  ", ";
      first = false;
      s += e->getStringName();
    }
    s += ")";
    return s;
  }
  std::vector<Expr *> getList(){
    return expr_list;
  }
  virtual void sem() override{
    for (Expr *e : expr_list) e->sem();
  }
private:
   std::vector<Expr *> expr_list;
};



class Rval: public Expr {
public:
  virtual int eval() const override {
    std::cout << "Evaluating Rval";
    return -1;
  }
};
class Lval: public Expr {
public:
  virtual int eval() const override {
    std::cout << "Evaluating Lval";
    return -1;
  }
};

class Result: public Lval {
public:
  Result(): var("result"), offset(-1){type = new TypeRes();}
  virtual void printOn(std::ostream &out) const override {
    out << "Result(" << var << "@" << offset << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    std::string va;
    va = var;
    std::string v;
    v = offset;
    s += "Result(" + va + "@" + v + ")";
    return s;
  }
  virtual int eval() const override {
    return rt_stack[offset];
  }
  virtual bool isResult() override{
      return true;
  }
  virtual void sem() override{  }
private:
  std::string var;
  int offset;
};

class BinOp: public Rval {
public:
  BinOp(Expr *l, char *o, Expr *r): left(l), op(o), right(r) {
   }
  ~BinOp() { delete left; delete right; }
  virtual void printOn(std::ostream &out) const override {
    out << "BinOp(";
    left->printOn(out);
    if(op) out << op;
    right->printOn(out);
    out << ")";
  }
  virtual std::string  getStringName() override {
    std::string s = "";
    s += "BinOp(";
    s += left->getStringName();
    if(op) s += op;
    s += right->getStringName();
    s += ")";
    return s;
  }
  virtual bool check_number(Expr *left, Expr *right){

    return ((left->type->val == TYPE_REAL) && (right->type->val == TYPE_INTEGER))||((left->type->val ==TYPE_INTEGER) && (right->type->val ==TYPE_REAL))||((left->type->val ==TYPE_REAL) && (right->type->val == TYPE_REAL))||((left->type->val == TYPE_INTEGER) && (right->type->val == TYPE_INTEGER));
  }
  virtual void sem() override {
    left->sem();
    right->sem();

    if(left->type->val == TYPE_RES){
      left->type = st.lookup("result")->type;
    }
    if(right->type->val == TYPE_RES){
      right->type = st.lookup("result")->type;
    }

    if(! strcmp(op, "+") || ! strcmp(op, "*") || ! strcmp(op, "-")){
      if( left->type->val == TYPE_INTEGER && right->type->val == TYPE_INTEGER){
        type = new Integer();
      }
      else if(left->type->val == TYPE_REAL || right->type->val == TYPE_REAL){
        type = new Real();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "/")){
      if(check_number(left, right)){
        type = new Real();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "mod") || ! strcmp(op, "div")){
      if( left->type->val == TYPE_INTEGER && right->type->val == TYPE_INTEGER){
        type = new Integer();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "=") || ! strcmp(op, "<>")){
      if(check_number(left, right) || ((left->type->val == right->type->val) && (left->type->val != TYPE_ARRAY))){
        type = new Boolean();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "<") || ! strcmp(op, ">") || ! strcmp(op, "<=") || ! strcmp(op, ">=")){
      if(check_number(left, right)){
        type = new Boolean();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
      }
    }
    if(! strcmp(op, "or") || ! strcmp(op, "and")){
      if(left->type->val == TYPE_BOOLEAN && right->type->val == TYPE_BOOLEAN){
        type = new Boolean();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << left->type->val << op << right->type->val << "\n"; printOn(std::cout); exit(1);
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
  UnOp(char *o, Expr *r): op(o), right(r) {}
  ~UnOp() { delete right; }
  virtual void printOn(std::ostream &out) const override {
    out << "UnOp(";
    if(op) out << op;
    right->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "UnOp(";
    if(op) s += op;
    s += right->getStringName();
    s += ")";
    return s;
  }
  virtual void sem() override {
    right->sem();
    if(right->type->val == TYPE_RES){
      right->type = st.lookup("result")->type;
    }
    if(! strcmp(op, "+") || ! strcmp(op, "-")){
      if(right->type->val == TYPE_INTEGER || right->type->val == TYPE_REAL){
        type = right->type;
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << op << right->type->val << "\n"; exit(1);
      }
    }
    if( ! strcmp(op, "not")){
      if(right->type->val == TYPE_BOOLEAN){
        type = new Boolean();
      }
      else{
        ERROR("Type mismatch!\n"); std::cout << op << right->type->val << "\n"; exit(1);
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
  Id(char* v): var(v), offset(-1){   }
  virtual void printOn(std::ostream &out) const override {
    out << "Id(" << var << "@" << offset << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    std::string va;
    va = var;
    std::string v;
    v = offset;
    s += "Id(" + va + "@" + v + ")";
    return s;
  }
  virtual int eval() const override {
    return rt_stack[offset];
  }
  virtual void sem() override {
    // SymbolEntry *e = st.lookup(var);
    // Types type = e->type;
    // offset = e->offset;
    std::string s = var;
    type = st.lookup(s)->type;
  }
private:
  char* var;
  int offset;

};

class ArrayItem: public Lval {
public:
  ArrayItem(Lval *l, Expr *e){
    lval = l;
    expr = e;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "ArrayItem(";
    if(lval)lval->printOn(out);
    if(expr)expr->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "ArrayItem(";
    if(lval) s+= lval->getStringName();
    if(expr) s+= expr->getStringName();
    s += ")";
    return s;
  }
  virtual int eval() const override {
    std::cout << "Evaluating ArrayItem";
    return -1;
  }
  virtual void sem() override {
    lval->sem();
    expr->sem();
    if(lval->type->val == TYPE_RES){
      lval->type = st.lookup("result")->type;
    }
    if(lval->type->val != TYPE_ARRAY){
      printOn(std::cout);
      std::cout << "\n is not of type array!\n";
      exit(1);
    }
    else{
      if(expr->type->val != TYPE_INTEGER){
        printOn(std::cout);
        std::cout << "\nbracket expression is not of type integer!\n";
        exit(1);
      }
    }
    type = lval->type->oftype;
  }
private:
  Lval *lval;
  Expr *expr;
};

class Reference: public Rval {
public:
  Reference(Lval *l){
    lval = l;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Reference(";
    if(lval)lval->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s +=  "Reference(";
    if(lval) s+= lval->getStringName();
    s += ")";
    return s;
  }
  virtual int eval() const override {
    std::cout << "Evaluating Reference";
    return -1;
  }
  virtual void sem() override{
      lval->sem();
      if(lval->type->val == TYPE_RES){
        lval->type = st.lookup("result")->type;
      }
      type = new Pointer(lval->type);
  }
private:
  Lval *lval;
};




class Stmt: public AST {
public:
  virtual void run() const = 0;
};



class IdLabel: public Stmt{
public:
  IdLabel(char* i, Stmt *s){
    id = i;
    stmt = s;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "IdLabel(";
    if(id) out << id << " ";
    if(stmt) stmt->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s ="";
    s += "IdLabel(";
    std::string var;
    var = id;
    if(id) s+= var + " ";
    if(stmt) s+= stmt->getStringName();
    s+= ")";
    return s;
  }
  virtual void run() const override {
    std::cout << "Running IdLabel";
  }
private:
  char* id;
  Stmt *stmt;
};

class Assign: public Stmt{
public:
  Assign(Lval *l, Expr *e){
    lval = l;
    exprRight = e;
    exprPointer = nullptr;
  }
  Assign(Expr *e1, Expr *e2){
    exprPointer = e1;
    exprRight = e2;
    lval = nullptr;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Assign(";
    if(lval && exprRight){lval->printOn(out); out << " := "; exprRight->printOn(out);}
    else if(exprPointer && exprRight){ exprPointer->printOn(out); out << " ^ := "; exprRight->printOn(out);}
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Assign(";
    if(lval && exprRight){s += lval->getStringName(); s +=  " := "; s += exprRight->getStringName();}
    else if(exprPointer && exprRight){ s += exprPointer->getStringName(); s +=  " ^ := "; exprRight->getStringName();}
    s +=  ")";
    return s;
  }
  virtual void run() const override {
    std::cout << "Running Assign";
  }
  virtual void sem() override{
    if(lval && exprRight){
      lval->sem();
      exprRight->sem();
      if(lval->isResult()){
        //result
        if(!st.existsResult()){
          st.insert("result", exprRight->type);
        }
        std::string funName;
        Type *funType;
        funName  = st.getParentFunction();
        funType = st.lookup(funName)->type;
        Type *resultType = exprRight->type;
        if(!(*resultType == *funType)){
          std::cout << "Function " << funName.erase(0, 1) << " is of type ";
          funType->printOn(std::cout);
          std::cout << " but returns type ";
          resultType->printOn(std::cout);
          std::cout << "\n";
          exit(1);
        }
        lval->type = resultType;
      }
      else{
        // not result
        if(!(*lval->type == *exprRight->type)){
          std::cout << "Assign Type missmatch!\n";
          printOn(std::cout);
          std::cout << "\n";
          lval->type->printOn(std::cout);
          std::cout << " := ";
          exprRight->type->printOn(std::cout);
          std::cout << "\n";
          exit(1);
        }
      }
    }
    else if(exprPointer && exprRight){
      // exprPointer ^ := exprRight
      exprPointer->sem();
      exprRight->sem();
      if(exprRight->type->val == TYPE_NIL){
        printOn(std::cout);
        std::cout << "\nnil cant be dereferenced\n";
        exit(1);
      }
      if(exprPointer->isResult()){
        //result
        if(!(exprRight->type->val == TYPE_POINTER )){
          std::cout << "Assign Type missmatch!\n";
          std::cout << "Expression: ";
          exprRight->printOn(std::cout);
          std::cout << " must be a Pointer to some type\n";
          exit(1);
        }
        if(!st.existsResult()){
          st.insert("result", exprRight->type->oftype);
        }

        std::string funName;
        Type *funType;
        funName  = st.getParentFunction();
        funType = st.lookup(funName)->type;
        Type *resultType = exprRight->type->oftype;
        if(!(*resultType == *funType)){
          std::cout << "Function " << funName.erase(0, 1) << " is of type ";
          funType->printOn(std::cout);
          std::cout << " but returns type ";
          resultType->printOn(std::cout);
          std::cout << "\n";
          exit(1);
        }
        exprPointer->type = resultType;
      }
      else{
        // not result
        if(!(exprRight->type->val == TYPE_POINTER )){
          std::cout << "Assign Type missmatch!\n";
          std::cout << "Expression: ";
          exprRight->printOn(std::cout);
          std::cout << " must be a Pointer to some type\n";
          exit(1);
        }
        else{
          if(!(*exprPointer->type == *exprRight->type->oftype)){
            std::cout << "Assign Type missmatch!\n";
            printOn(std::cout);
            std::cout << "\n";
            lval->type->printOn(std::cout);
            std::cout << " := ";
            exprRight->type->printOn(std::cout);
            std::cout << "\n";
            exit(1);
          }
        }
      }
    }
  }
private:
  Expr *exprPointer;
  Expr *exprRight;
  Lval *lval;
};

class Return: public Stmt{
public:
  Return(){};
  virtual void printOn(std::ostream &out) const override {
    out << "Return(";
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Return(";
    s +=  ")";
    return s;
  }
  virtual void run() const override {
    std::cout << "Running Return";
  }
private:
};

class Id;

class Id_list: public AST{
public:
  Id_list(): id_list(){}
  ~Id_list() {
    for (char *id : id_list) delete id;
  }
  void append_id(char* id) { id_list.push_back(id); }
  void append_begin(char *i) { id_list.insert(id_list.begin(), i); }

  virtual void printOn(std::ostream &out) const override {
    out << "Id_list(";
    bool first = true;
    for (char* id : id_list) {
      if (!first) out << ", ";
      first = false;
      if(id) out << id << " ";
    }
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Id_list(";
    bool first = true;
    for (char* id : id_list) {
      if (!first) s += ", ";
      first = false;
      std::string var;
      var = id;
      if(id) s += var + " ";
    }
    s += ")";
    return s;
  }
  std::vector<char* > getlist(){
    return id_list;
  }
  int length(){
    return id_list.size();
  }
  std::vector<char* > charList(){
    return id_list;
  }
private:
   std::vector<char* > id_list;
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
    type->printOn(out);
    out << ")";
 }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Formal(";
    s += id_list->getStringName();
    s += type->getStringName();
    s += ")";
    return s;
 }
 virtual void semForward() override{
   for (char *id : id_list->getlist()) {
     std::string var = id;
     st.insertForward(var, type);
   }
 }
 virtual void sem() override{
   for (char *id : id_list->getlist()) {
     std::string var = id;
     if(!st.isForward(var)){
       st.insert(var, type);
     }
   }
 }
 virtual void sem_() override{
   for (char *id : id_list->getlist()) {
     std::string var = id;
     var = "_" + var;
     if(!st.isForward(var)){
       st.insert(var, type);
     }
   }
 }
 Type *getType(){
   return type;
 }
 std::vector<char* > getIdList(){
   return id_list->charList();
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
  void append_begin(Formal *f) { formal_list.insert(formal_list.begin(), f); }

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
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Formal_list(";
    bool first = true;
    for (Formal *f : formal_list) {
      if (!first) s += ", ";
      first = false;
      s += f->getStringName();
    }
    s += ")";
    return s;
  }
  std::vector<Formal *> getList(){
    return formal_list;
  }
  virtual void semForward() override{
    for (Formal *f : formal_list) f->semForward();
  }
  virtual void sem() override{
    for (Formal *f : formal_list) f->sem();
  }
  virtual void sem_() override{
    for (Formal *f : formal_list) f->sem_();
  }
private:
   std::vector<Formal *> formal_list;
};

class Call: public Stmt{
public:
  Call(){
    id = nullptr;
    expr_list = nullptr;
  }
  Call(char* i, Expr_list *e = nullptr){
    id = i;
    expr_list = e;
  }
  ~Call(){
    delete id; delete expr_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Call(";
    if(id) out << id << " ";
    if(expr_list) expr_list->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Call(";
    std::string var;
    var = id;
    if(id) s += var + " ";
    if(expr_list) s += expr_list->getStringName();
    s += ")";
    return s;
  }
  virtual void run() const override {
    std::cout << "Running Call";
  }
  virtual void sem() override {
    std::string s = id;
    if(expr_list) expr_list->sem();
    SymbolEntry *se;
    se = st.lookup(s);
    s = se->s;
    if(st.isProcedure(s)){
      std::vector<Formal *> formal_list;
      formal_list = st.getFormalsProcedure(s)->getList();
      int i = 0;
      int argumentsExpected = 0;
      int argumentsProvided = 0;
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        argumentsExpected += FormalTimes;
      }
      argumentsProvided = expr_list->getList().size();
      if(argumentsExpected != argumentsProvided){
        std::cout << "Procedure " << s << " expected " << argumentsExpected << " arguments " << " got " << argumentsProvided;
        exit(1);
      }
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        for (int j=0; j<FormalTimes; j++){
          if(!(*f->getType() == *expr_list->getList().at(i)->getType())){
            ERROR("Type mismatch on procedure arguments!\n");
            std::cout << "In procedure "<< s << " arguments:\n";
            std::cout << f->getIdList().at(j);
            std::cout << "\n and \n";
            expr_list->getList().at(i)->printOn(std::cout);
            std::cout << "\nHave different types of ";
            f->getType()->printOn(std::cout);
            std::cout << " and ";
            expr_list->getList().at(i)->getType()->printOn(std::cout);
            exit(1);
          }
          i += 1;
        }
      }
      if(st.isForward(s)){
        std::cout << "Cant call procedure " << s << " because it is forward declared, but not defined.\n";
        exit(1);
      }
    }
    else if(st.isFunction(s)){
      std::vector<Formal *> formal_list;
      formal_list = st.getFormalsFunction(s)->getList();
      int i = 0;
      int argumentsExpected = 0;
      int argumentsProvided = 0;
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        argumentsExpected += FormalTimes;
      }
      argumentsProvided = expr_list->getList().size();
      if(argumentsExpected != argumentsProvided){
        std::cout << "Procedure " << s << " expected " << argumentsExpected << " arguments " << " got " << argumentsProvided;
        exit(1);
      }
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        for (int j=0; j<FormalTimes; j++){
          if(!(*f->getType() == *expr_list->getList().at(i)->getType())){
            ERROR("Type mismatch on function arguments!\n");
            std::cout << "In function "<< s << " arguments:\n";
            std::cout << f->getIdList().at(j);
            std::cout << "\n and \n";
            expr_list->getList().at(i)->printOn(std::cout);
            std::cout << "\nHave different types of ";
            f->getType()->printOn(std::cout);
            std::cout << " and ";
            expr_list->getList().at(i)->getType()->printOn(std::cout);
            exit(1);
          }
          i += 1;
        }
      }
    }
    if(st.isForward(s)){
      std::cout << "Cant call function " << s << " because it is forward declared, but not defined.\n";
      exit(1);
    }
  }

private:
  char* id;
  Expr_list *expr_list;
};

class Callr: public Rval{
public:
  Callr(){
    id = nullptr;
    expr_list = nullptr;
  }
  Callr(char *i, Expr_list *e = nullptr){
    id = i;
    expr_list = e;
  }
  ~Callr(){
    delete expr_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Callr(";
    if(id) out << id << " ";
    if(expr_list) expr_list->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Callr(";
    std::string var;
    var = id;
    if(id) s += var + " ";
    if(expr_list) s += expr_list->getStringName();
    s += ")";
    return s;
  }
  virtual void sem() override {
    std::string s = id;
    type = st.lookup(s)->type;
    if(expr_list) expr_list->sem();
    SymbolEntry *se;
    se = st.lookup(s);
    s = se->s;
    if(st.isProcedure(s)){
      std::vector<Formal *> formal_list;
      formal_list = st.getFormalsProcedure(s)->getList();
      int i = 0;
      int argumentsExpected = 0;
      int argumentsProvided = 0;
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        argumentsExpected += FormalTimes;
      }
      argumentsProvided = expr_list->getList().size();
      if(argumentsExpected != argumentsProvided){
        std::cout << "Procedure " << s << " expected " << argumentsExpected << " arguments " << " got " << argumentsProvided;
        exit(1);
      }
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        for (int j=0; j<FormalTimes; j++){
          if(!(*f->getType() == *expr_list->getList().at(i)->getType())){
            ERROR("Type mismatch on procedure arguments!\n");
            std::cout << "In procedure "<< s << " arguments:\n";
            std::cout << f->getIdList().at(j);
            std::cout << "\n and \n";
            expr_list->getList().at(i)->printOn(std::cout);
            std::cout << "\nHave different types of ";
            f->getType()->printOn(std::cout);
            std::cout << " and ";
            expr_list->getList().at(i)->getType()->printOn(std::cout);
            exit(1);
          }
          i += 1;
        }
      }
    }
    else if(st.isFunction(s)){
      std::vector<Formal *> formal_list;
      formal_list = st.getFormalsFunction(s)->getList();
      int i = 0;
      int argumentsExpected = 0;
      int argumentsProvided = 0;
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        argumentsExpected += FormalTimes;
      }
      argumentsProvided = expr_list->getList().size();
      if(argumentsExpected != argumentsProvided){
        std::cout << "Procedure " << s << " expected " << argumentsExpected << " arguments " << " got " << argumentsProvided;
        exit(1);
      }
      for (Formal *f : formal_list) {
        int FormalTimes;
        FormalTimes = f->getIdList().size();
        for (int j=0; j<FormalTimes; j++){
          if(!(*f->getType() == *expr_list->getList().at(i)->getType())){
            ERROR("Type mismatch on function arguments!\n");
            std::cout << "In function "<< s << " arguments:\n";
            std::cout << f->getIdList().at(j);
            std::cout << "\n and \n";
            expr_list->getList().at(i)->printOn(std::cout);
            std::cout << "\nHave different types of ";
            f->getType()->printOn(std::cout);
            std::cout << " and ";
            expr_list->getList().at(i)->getType()->printOn(std::cout);
            exit(1);
          }
          i += 1;
        }
      }
    }
  }
private:
  char *id;
  Expr_list *expr_list;
};

class New: public Stmt{
public:
  New(Lval *l){
    lval = l;
    exprPointer = nullptr;
    exprBrackets = nullptr;
  }
  New(Expr *e){
    exprPointer = e;
    lval = nullptr;
    exprBrackets = nullptr;
  }
  New(Expr *e1, Expr *e2){
    exprBrackets = e1;
    exprPointer = e2;
    lval = nullptr;
  }
  New(Expr *e, Lval *l){
    exprBrackets = e;
    lval = l;
    exprPointer = nullptr;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "New(";
    if(lval) lval->printOn(out);
    if(exprPointer) exprPointer->printOn(out);
    if(exprBrackets) exprBrackets->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "New(";
    if(lval) s += lval->getStringName();
    if(exprPointer) s+=exprPointer->getStringName();
    if(exprBrackets) s+= exprBrackets->getStringName();
    s += ")";
    return s;
  }
  virtual void run() const override {
    std::cout << "Running New";
  }
  virtual void sem() override {
    if(lval && !exprPointer && exprBrackets){
      // "new" "[" expr "]" l-value
      lval->sem();
      exprBrackets->sem();
      if(lval->type->val == TYPE_RES){
        lval->type = st.lookup("result")->type;
      }
      if(exprBrackets->type->val == TYPE_RES){
        exprBrackets->type = st.lookup("result")->type;
      }
      if(lval->type->val != TYPE_POINTER){
        printOn(std::cout);
        std::cout << "\nIn expression new [expr] l-value, l-value must be a pointer but is of type ";
        lval->type->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      else{
        if(lval->type->oftype->val != TYPE_ARRAY){
          printOn(std::cout);
          std::cout << "\nIn expression new [expr] l-value, l-value must be a pointer to array but is a pointer to ";
          lval->type->oftype->printOn(std::cout);
          std::cout << "\n";
          exit(1);
        }
      }
      if(exprBrackets->type->val != TYPE_INTEGER){
        printOn(std::cout);
        std::cout << "\nIn expression new [expr] l-value, expr must be of type integer, but it is of type ";
        exprBrackets->type->oftype->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      st.makeNew(lval->getStringName());
    }
    else if(!lval && exprPointer && exprBrackets){
      // "new" "[" expr "]" expr "^"
      exprPointer->sem();
      exprBrackets->sem();
      if(exprPointer->type->val == TYPE_RES){
        exprPointer->type = st.lookup("result")->type;
      }
      if(exprBrackets->type->val == TYPE_RES){
        exprBrackets->type = st.lookup("result")->type;
      }
      if(exprPointer->type->val != TYPE_POINTER){
        printOn(std::cout);
        std::cout << "\nIn expression new [expr] expr ^, expr must be a pointer but is of type ";
        exprPointer->type->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      else{
        if(exprPointer->type->oftype->val != TYPE_POINTER){
          printOn(std::cout);
          std::cout << "\nIn expression new [expr] expr ^, (expr ^) must be a pointer, but is of type ";
          exprPointer->type->oftype->printOn(std::cout);
          std::cout << "\n";
          exit(1);
        }
        else{
          if(exprPointer->type->oftype->oftype->val != TYPE_ARRAY){
            printOn(std::cout);
            std::cout << "\nIn expression new [expr] expr ^, (expr ^) must be a pointer to array but is a pointer to ";
            exprPointer->type->oftype->oftype->printOn(std::cout);
            std::cout << "\n";
            exit(1);
          }
        }
      }
      if(exprBrackets->type->val != TYPE_INTEGER){
        printOn(std::cout);
        std::cout << "\nIn expression new [expr] l-value, expr must be of type integer, but it is of type ";
        exprBrackets->type->oftype->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      st.makeNew(exprPointer->getStringName());
    }
    else if(!lval && exprPointer && !exprBrackets){
      // "new" expr "^"
      exprPointer->sem();
      if(exprPointer->type->val == TYPE_RES){
        exprPointer->type = st.lookup("result")->type;
      }
      if(exprPointer->type->val != TYPE_POINTER){
        printOn(std::cout);
        std::cout << "\nIn expression new expr ^, expr must be a pointer but is of type ";
        exprPointer->type->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      else{
        if(exprPointer->type->oftype->val != TYPE_POINTER){
          printOn(std::cout);
          std::cout << "\nIn expression new expr ^, (expr ^) must be a pointer but is of type ";
          exprPointer->type->oftype->printOn(std::cout);
          std::cout << "\n";
          exit(1);
        }
      }
      st.makeNew(exprPointer->getStringName());
    }
    else{
      // "new" l-value
      lval->sem();
      if(lval->type->val == TYPE_RES){
        lval->type = st.lookup("result")->type;
      }
      if(lval->type->val != TYPE_POINTER){
        printOn(std::cout);
        std::cout << "\nIn expression new l-value, l-value must be a pointer but is of type ";
        lval->type->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      st.makeNew(lval->getStringName());
    }

  }
private:
  Lval *lval;
  Expr *exprPointer;
  Expr *exprBrackets;
};

class Goto: public Stmt{
public:
  Goto(char* i){
    id = i;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Goto(";
    out << id << " ";
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Goto(";
    std::string var;
    var = id;
    s += var + " ";
    s += ")";
    return s;
  }
  virtual void run() const override {
    std::cout << "Running Goto";
  }
private:
  char* id;
};


class Stmt_list: public AST{
public:
  Stmt_list(): stmt_list(){}
  ~Stmt_list() {
    for (Stmt *s : stmt_list) delete s;
  }
  void append_stmt(Stmt *s) { if(s) stmt_list.push_back(s); }
  void append_begin(Stmt *s) { if(s) stmt_list.insert(stmt_list.begin(), s); }
  virtual void printOn(std::ostream &out) const override {
    out << "\nStmt_list(";
    bool first = true;
    for (Stmt *s : stmt_list) {
      if (!first) out << ", ";
      first = false;
      out << "\n\t";
      s->printOn(out);
    }
    out << "\n)";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "\nStmt_list(";
    bool first = true;
    for (Stmt *s1 : stmt_list) {
      if (!first) s += ", ";
      first = false;
      s += "\n\t";
      s += s1->getStringName();
    }
    s += "\n)";
    return s;
  }
  virtual void sem() override {
    for (Stmt *s : stmt_list) s->sem();
  }
private:
   std::vector<Stmt *> stmt_list;
};


class Constint: public Rval {
public:
  Constint(int c): con(c) {
    type = new Integer();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Constint(";
    out << con ;
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Constint(";
    s += con ;
    s += ")";
    return s;
  }
  virtual int eval() const override { return con; }
  // virtual void sem() override { type = new Integer(); }
  virtual int get(){
    return con;
  }
private:
  int con;
};



class Constchar: public Rval {
public:
  Constchar(char c): con(c) {
    type = new Char();}
  virtual void printOn(std::ostream &out) const override {
    out << "Constchar(" << con << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    std::string var;
    var = con;
    s += "Constchar(" + var + ")";
    return s;
  }
  virtual int eval() const override { return 0; } //wrong
  // virtual void sem() override { type = new Char(); }
private:
  char con;
};

class Conststring: public Lval {
public:
  Conststring(char *c): con(c) {
    type = new Array(new Char());}
  virtual void printOn(std::ostream &out) const override {
    out << "Conststring(" << con << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    std::string var;
    var = con;
    s += "Conststring(" + var + ")";
    return s;
  }
  virtual int eval() const override { return 0; } //wrong
  // virtual void sem() override { type = new String(); }
private:
  char *con;
};

class Constreal: public Rval {
public:
  Constreal(double c): con(c) {
    type = new Real();}
  virtual void printOn(std::ostream &out) const override {
    out << "Constreal(" << con << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    std::string var;
    var = con;
    s += "Constreal(" + var + ")";
    return s;
  }
  virtual int eval() const override { return 0; } //wrong
  // virtual void sem() override { type = new Real(); }
private:
  double con;
};

class Constboolean: public Rval {
public:
  Constboolean(std::string b){
    if(b.compare("true")){
      con = true;
    }
    else{
      con = false;
    }
    type = new Boolean();
  }
  Constboolean(bool b){
    con = b;
    type = new Boolean();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Constboolean(" << con << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    std::string var;
    var = con;
    s += "Constboolean(" + var + ")";
    return s;
  }
  virtual int eval() const override { return 0; } //wrong
  // virtual void sem() override { type = new Boolean(); }
private:
  bool con;
};

class NilR: public Rval {
public:
  NilR(){
    con = nullptr;
    type = new TypeNil();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "NilR()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "NilR()";
    return s;
  }
  virtual int eval() const override { return 0; } //wrong
private:
  char *con;
};

class NilL: public Lval {
public:
  NilL(){
    con = nullptr;
    type = new TypeNil();
  }
  virtual void printOn(std::ostream &out) const override {
    out << "NilL()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "NilL()";
    return s;
  }
  virtual int eval() const override { return 0; } //wrong
private:
  char *con;
};

class Dispose: public Stmt{
public:
  Dispose(Lval *l, bool b){
    lval = l;
    expr = nullptr;
    isBracket = b;
  }
  Dispose(Expr *e, bool b){
    expr = e;
    lval = nullptr;
    isBracket = b;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Dispose(";
    if(lval) lval->printOn(out);
    else expr->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Dispose(";
    if(lval) s += lval->getStringName();
    else s += expr->getStringName();
    s += ")";
    return s;
  }
  virtual void run() const override {
    std::cout << "Running Dispose";
  }
  virtual void sem() override {
    if(lval && !expr && !isBracket){
      // dispose l-value
      lval->sem();
      if(lval->type->val == TYPE_RES){
        lval->type = st.lookup("result")->type;
      }
      if(lval->type->val != TYPE_POINTER){
        printOn(std::cout);
        std::cout << "\nIn expression dispose l-value, l-value must be a pointer but is of type ";
        lval->type->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      if(!st.isNew(lval->getStringName())){
        printOn(std::cout);
        std::cout << "\nIn expression dispose l-value, l-value must have had been created by new l-value\n";
        exit(1);
      }
      lval = new NilL();
    }
    else if(lval && !expr && isBracket){
      // dispose [] l-value
      lval->sem();
      if(lval->type->val == TYPE_RES){
        lval->type = st.lookup("result")->type;
      }
      if(lval->type->val != TYPE_POINTER){
        printOn(std::cout);
        std::cout << "\nIn expression dispose [] l-value, l-value must be a pointer but is of type ";
        lval->type->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      if(!st.isNew(lval->getStringName())){
        printOn(std::cout);
        std::cout << "\nIn expression dispose [] l-value, l-value must have had been created by new l-value\n";
        exit(1);
      }
      if(lval->type->oftype->val != TYPE_ARRAY){
        printOn(std::cout);
        std::cout << "\nIn expression dispose [] l-value, l-value must be a pointer to array but is a pointer to ";
        lval->type->oftype->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      lval = new NilL();
    }
    else if(!lval && expr && !isBracket){
      // dispose expr "^"
      expr->sem();
      if(expr->type->val == TYPE_RES){
        expr->type = st.lookup("result")->type;
      }
      if(expr->type->val != TYPE_POINTER){
        printOn(std::cout);
        std::cout << "\nIn expression dispose expr ^, expr must be a pointer but is of type ";
        expr->type->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      else{
        if(!st.isNew(expr->getStringName())){
          printOn(std::cout);
          std::cout << "\nIn expression dispose expr ^, expr ^ must have had been created by new expr ^\n";
          exit(1);
        }
        if(expr->type->oftype->val != TYPE_POINTER){
          printOn(std::cout);
          std::cout << "\nIn expression disposes expr ^, (expr ^) must be a pointer, but is of type ";
          expr->type->oftype->printOn(std::cout);
          std::cout << "\n";
          exit(1);
        }
        expr = new NilL();
      }
    }
    else{
      // dispose [] expr "^"
      expr->sem();
      if(expr->type->val == TYPE_RES){
        expr->type = st.lookup("result")->type;
      }
      if(expr->type->val != TYPE_POINTER){
        printOn(std::cout);
        std::cout << "\nIn expression dispose [] expr ^, expr must be a pointer but is of type ";
        expr->type->printOn(std::cout);
        std::cout << "\n";
        exit(1);
      }
      else{
        if(!st.isNew(expr->getStringName())){
          printOn(std::cout);
          std::cout << "\nIn expression dispose [] expr ^, [] expr ^ must have had been created by new [] expr ^\n";
          exit(1);
        }
        if(expr->type->oftype->val != TYPE_POINTER){
          printOn(std::cout);
          std::cout << "\nIn expression dispose [] expr ^, (expr ^) must be a pointer, but is of type ";
          expr->type->oftype->printOn(std::cout);
          std::cout << "\n";
          exit(1);
        }
        else{
          if(expr->type->oftype->oftype->val != TYPE_ARRAY){
            printOn(std::cout);
            std::cout << "\nIn expression dispose [] expr ^, (expr ^) must be a pointer to array but is a pointer to ";
            expr->type->oftype->oftype->printOn(std::cout);
            std::cout << "\n";
            exit(1);
          }
        }
        expr = new NilL();
      }
    }
}

private:
  Lval *lval;
  Expr *expr;
  bool isBracket;
};

class If: public Stmt {
public:
  If(Expr *c, Stmt *s1, Stmt *s2 = nullptr):
    cond(c), stmt1(s1), stmt2(s2) {    }
  ~If() { delete cond; delete stmt1; delete stmt2; }
  virtual void printOn(std::ostream &out) const override {
    out << "If(";
    if(cond) cond->printOn(out);
    if(stmt1) stmt1->printOn(out);
    if (stmt2) stmt2->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "If(";
    if(cond) s += cond->getStringName();
    if(stmt1) s += stmt1->getStringName();
    if (stmt2) s += stmt2->getStringName();
    s += ")";
    return s;
  }
  virtual void sem() override {
    cond->sem();
    if(cond->type->val == TYPE_RES){
      cond->type = st.lookup("result")->type;
    }
    if(cond->type->val == TYPE_BOOLEAN){
      stmt1->sem();
      if (stmt2 != nullptr) stmt2->sem();
    }
    else{
      ERROR("Type mismatch, cond is not bool!\n"); printOn(std::cout); exit(1);
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
  While(Expr *e, Stmt *s): expr(e), stmt(s) { }
  ~While() { delete expr; delete stmt; }
  virtual void printOn(std::ostream &out) const override {
    out << "While(";
    if(expr) expr->printOn(std::cout);
    out << ", ";
    if(stmt) stmt->printOn(std::cout);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "While(";
    if(expr) s += expr->getStringName();
    s += ", ";
    if(stmt) stmt->getStringName();
    s += ")";
    return s;
  }
  virtual void sem() override {
    expr->sem();
    if(expr->type->val == TYPE_RES){
      expr->type = st.lookup("result")->type;
    }
    if(expr->type->val == TYPE_BOOLEAN){
      stmt->sem();
    }
    else{
      ERROR("Type mismatch!\n"); exit(1);
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


class Block: public Stmt{
public:
  Block(Stmt_list *s = nullptr){
    if(s) stmt_list = s;
  }
  ~Block(){
    delete stmt_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Block(";
    if(stmt_list) stmt_list->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Block(";
    if(stmt_list) s += stmt_list->getStringName();
    s += ")";
    return s;
  }
  virtual void sem() override {
    stmt_list->sem();
  }
virtual void run() const override {
  std::cout << "Running block";
}
private:
  Stmt_list *stmt_list;
};

class Header: public AST{
public:
  virtual char *getFunctionName(){return nullptr;};
  virtual Type *getFunctionType(){return nullptr;};
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
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Label(";
    s += id_list->getStringName();
    s += ")";
    return s;
  };
  virtual void sem() override {
    id_list->sem();
  }
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
    type->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Decl(";
    s += id_list->getStringName();
    s += type->getStringName();
    s += ")";
    return s;
  }
  virtual void sem() override{
    for (char *id : id_list->getlist()) {
      std::string var = id;
      st.insert(var, type);
    }
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
  void append_begin(Decl *d) { decl_list.insert(decl_list.begin(), d); }

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
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Decl_list(";
    bool first = true;
    for (Decl *d : decl_list) {
      if (!first) s += ", ";
      first = false;
      s += d->getStringName();
    }
    s += ")";
    return s;
  }
  virtual void sem() override {
    for (Decl *d : decl_list) d->sem();
  }
private:
   std::vector<Decl *> decl_list;
};




class Procedure: public Header{
public:
  Procedure(char* i, Formal_list *f = nullptr){
    id = i;
    formal_list = f;
  }
  ~Procedure(){
    delete id; delete formal_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Procedure(";
    if(id) out << id << " ";
    formal_list->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Procedure(";
    std::string var;
    var = id;
    if(id) s += var + " ";
    s += formal_list->getStringName();
    s += ")";
    return s;
  }
  virtual void semForward() override{
    std::string s = id;
    formal_list->semForward();
    st.insertProcedureForward(s, new ProcedureType(), formal_list);
  }
  virtual void sem() override {
    std::string s = id;
    formal_list->sem();
    if(st.isForward(s)){
      //Procedure was previously forward declared
      std::string prev;
      std::string now;
      prev = st.getFormalsProcedure(s)->getStringName();
      now = formal_list->getStringName();
      if(prev.compare(now)){
        std::cout << "Procedure " << s << " was previously declared with arguments: " << prev << " but now it is defined with arguments " << now << "\n";
        exit(1);
      }
      st.removeForward(s);
    }
    else{
      st.insertProcedure(s, new ProcedureType(), formal_list);
    }
  }
  virtual void sem_() override {
    std::string s = id;
    s = "_" + s;
    formal_list->sem_();
    if(st.isForward(s)){
      //Procedure was previously forward declared
      std::string prev;
      std::string now;
      prev = st.getFormalsProcedure(s)->getStringName();
      now = formal_list->getStringName();
      if(! prev.compare(now)){
        std::cout << "Procedure " << s << " was previously declared with arguments: " << prev << " but now it is defined with arguments " << now << "\n";
        exit(1);
      }
      st.removeForward(s);
    }
    else{
      st.insertProcedure(s, new ProcedureType(), formal_list);
    }
  }
private:
  char* id;
  Formal_list *formal_list;
};

class Function: public Header{
public:
  Function(char* i, Type *t, Formal_list *f = nullptr){
    id = i;
    type = t;
    formal_list = f;
  }
  ~Function(){
    delete id; delete formal_list;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Function(";
    out << id << " ";
    formal_list->printOn(out);
    type->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Function(";
    std::string var;
    var = id;
    s += var + " ";
    s += formal_list->getStringName();
    s += type->getStringName();
    s += ")";
    return s;
  }
  virtual void semForward() override{
    std::string s = id;
    formal_list->semForward();
    st.insertFunctionForward(s, type, formal_list);
  }
  virtual void sem() override {
    std::string s = id;
    formal_list->sem();
    if(st.isForward(s)){
      //Function was previously forward declared
      std::string prev;
      std::string now;
      prev = st.getFormalsFunction(s)->getStringName();
      now = formal_list->getStringName();
      if(! prev.compare(now)){
        std::cout << "Function " << s << " was previously declared with arguments: " << prev << " but now it is defined with arguments " << now << "\n";
        exit(1);
      }
      st.removeForward(s);
    }
    else{
      st.insertFunction(s, type, formal_list);
    }
  }
  virtual void sem_() override {
    std::string s = id;
    s = "_" + s;
    formal_list->sem_();
    if(st.isForward(s)){
      //Function was previously forward declared
      std::string prev;
      std::string now;
      prev = st.getFormalsFunction(s)->getStringName();
      now = formal_list->getStringName();
      if(! prev.compare(now)){
        std::cout << "Function " << s << " was previously declared with arguments: " << prev << " but now it is defined with arguments " << now << "\n";
        exit(1);
      }
      st.removeForward(s);
    }
    else{
      st.insertFunction(s, type, formal_list);
    }
  }
  virtual char *getFunctionName() override{
    return id;
  }
  virtual Type *getFunctionType() override{
    return type;
  }
private:
  char* id;
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
  Local(Header *h, AST *b){
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
    out << "Local(";
    if(localType.compare("var") == 0){
      decl_list->printOn(out);
    }
    else if(localType.compare("label") == 0){
      label->printOn(out);
    }
    else if(localType.compare("forp") == 0){
      header->printOn(out);
      body->printOn(out);
    }
    else if(localType.compare("forward") == 0){
      header->printOn(out);
    }
    out << ")";
  };
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Local(";
    if(localType.compare("var") == 0){
      s += decl_list->getStringName();
    }
    else if(localType.compare("label") == 0){
      s += label->getStringName();
    }
    else if(localType.compare("forp") == 0){
      s += header->getStringName();
      s += body->getStringName();
    }
    else if(localType.compare("forward") == 0){
      s += header->getStringName();
    }
    s += ")";
    return s;
  };
  virtual void sem() override {
    if(localType.compare("var") == 0){
      decl_list->sem();
    }
    else if(localType.compare("label") == 0){
      label->sem();
    }
    else if(localType.compare("forp") == 0){
      header->sem();
      body->semPorF(header, header->getFunctionName(), header->getFunctionType());
    }
    else if(localType.compare("forward") == 0){
      header->semForward();
    }
  }
  char *getFunctionName(){
    return header->getFunctionName();
  }
  Type *getFunctionType(){
    return header->getFunctionType();
  }
private:
  Decl_list *decl_list;
  Label *label;
  Header *header;
  AST *body;
  std::string localType;
};

class Local_list: public AST{
public:
  Local_list(): local_list(){}
  ~Local_list() {
    for (Local *l : local_list) delete l;
  }
  void append_local(Local *l) { local_list.push_back(l); }
  void append_begin(Local *l) { local_list.insert(local_list.begin(), l); }

  virtual void printOn(std::ostream &out) const override {
    out << "Local_list(";
    bool first = true;
    for (Local *l : local_list) {
      if (!first) out << ", ";
      first = false;
      l->printOn(out);
    }
    out << ")\n";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Local_list(";
    bool first = true;
    for (Local *l : local_list) {
      if (!first) s += ", ";
      first = false;
      s += l->getStringName();
    }
    s += ")\n";
    return s;
  }
  virtual void sem() override {
    for (Local *l : local_list) l->sem();
  }
private:
   std::vector<Local *> local_list;
};

class Body: public AST{
public:
  Body(Local_list *l, Block *b){
    local_list = l;
    block = b;
  }
  ~Body() {
    delete local_list;
    delete block;
  }
  virtual void sem() override {
    st.openScope();
    local_list->sem();
    block->sem();
    st.closeScope();
  }
  void semPorF( AST *h, char *funName = nullptr, AST* funType = nullptr) override {
    st.openScope();
    h->sem_();
    local_list->sem();
    block->sem();
    if(!st.existsResult() && funName){
        std::cout << "Function " << funName << " does not have a result\n";
        exit(1);
    }
    st.closeScope();
  }
  void merge(Block *b) {
    block = b;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Body(";
    if(local_list) local_list->printOn(out);
    if(block) block->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Body(";
    if(local_list) s += local_list->getStringName();
    if(block) s += block->getStringName();
    s += ")";
    return s;
  }
private:
  Local_list *local_list;
  Block *block;
};
