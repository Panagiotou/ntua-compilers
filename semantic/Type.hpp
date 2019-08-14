#include "AST.hpp"

enum Types { TYPE_INTEGER, TYPE_BOOLEAN, TYPE_REAL, TYPE_ARRAY, TYPE_CHAR, TYPE_STRING, TYPE_POINTER, TYPE_PROCEDURE, TYPE_NIL, TYPE_RES };


class Type: public AST{
public:
  virtual void printOn(std::ostream &out) const override {
    out << "Type()";
  }
  virtual bool operator==(const Type &that) const { return false; }
  Types val;
  Type *oftype;
  int size;
};

class TypeNil: public Type{
public:
  TypeNil(){
    val = TYPE_NIL;
    oftype = nullptr;
    size = -1;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Nil()";
  }
  virtual bool operator==(const Type &that) const override {
    if(that.val == TYPE_NIL || that.val == TYPE_POINTER){
      return true;
    }
    return false;
  }
};

class TypeRes: public Type{
public:
  TypeRes(){
    val = TYPE_RES;
    oftype = nullptr;
    size = -1;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "TypeRes()";
  }
  virtual bool operator==(const Type &that) const override {
    std::cout << "Variable result is used uninitialized";
    exit(1);
  }
};


class Integer: public Type{
public:
  Integer(){ val = TYPE_INTEGER; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "Integer()";
  }
  virtual bool operator==(const Type &that) const override {
    if(that.val == TYPE_INTEGER){
      return true;
    }
    return false;
  }
};

class Char: public Type{
public:
  Char(){ val = TYPE_CHAR; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "Char()";
  }
  virtual bool operator==(const Type &that) const override {
    if(that.val == TYPE_CHAR){
      return true;
    }
    return false;
  }

};
class Real: public Type{
public:
  Real(){ val = TYPE_REAL; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "Real()";
  }
  virtual bool operator==(const Type &that) const override {
    if(that.val == TYPE_REAL){
      return true;
    }
    return false;
  }

};
class Boolean: public Type{
public:
  Boolean(){ val = TYPE_BOOLEAN; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "Boolean()";
  }
  virtual bool operator==(const Type &that) const override {
    if(that.val == TYPE_BOOLEAN){
      return true;
    }
    return false;
  }

};

class ProcedureType: public Type{
public:
  ProcedureType(){ val = TYPE_PROCEDURE; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "ProcedureType()";
  }

};

class Array: public Type{
public:
  Array(Type *t, int s = -1){
    val = TYPE_ARRAY;
    if(s>0) size = s;
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
  virtual bool operator==(const Type &that) const override {
    if(that.val == TYPE_ARRAY){
      if(size > 0){
        if(*oftype == *that.oftype && size == that.size ) return true;
      }
      else{
        if(*oftype == *that.oftype) return true;
      }
    }
    else if(that.val == TYPE_STRING){
      return true;
    }
    return false;
  }
};

class Pointer: public Type{
public:
  Pointer(Type *t){
    val = TYPE_POINTER;
    oftype = t;
    size = -1;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Pointer(";
    out << " of type:"; oftype->printOn(out);
    out << ")";
  }
  virtual bool operator==(const Type &that) const override {
    if(that.val == TYPE_NIL){
      return true;
    }
    if(that.val == TYPE_POINTER){
      if(*oftype == *that.oftype) return true;
    }
    return false;
  }
};
