#include "AST.hpp"

enum Types { TYPE_INTEGER, TYPE_BOOLEAN, TYPE_REAL, TYPE_ARRAY, TYPE_CHAR, TYPE_STRING, TYPE_POINTER, TYPE_PROCEDURE, TYPE_NIL, TYPE_RES, TYPE_LABEL };


class OurType: public AST{
public:
  virtual void printOn(std::ostream &out) const override {
    out << "Type()";
  }
  virtual bool operator==(const OurType &that) const { return false; }
  virtual Value* compile() const override { return 0;}
  Types val;
  OurType *oftype;
  int size;
};

class TypeNil: public OurType{
public:
  TypeNil(){
    val = TYPE_NIL;
    oftype = nullptr;
    size = -1;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Nil()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Nil()";
    return s;
  }
  virtual bool operator==(const OurType &that) const override {
    if(that.val == TYPE_NIL || that.val == TYPE_POINTER){
      return true;
    }
    return false;
  }
  virtual Value* compile() const override { return 0;}

};

class TypeRes: public OurType{
public:
  TypeRes(){
    val = TYPE_RES;
    oftype = nullptr;
    size = -1;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "TypeRes()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "TypeRes()";
    return s;
  }
  virtual bool operator==(const OurType &that) const override {
    std::cout << "Variable result is used uninitialized";
    exit(1);
  }
  virtual Value* compile() const override { return 0;}

};

class TypeLabel: public OurType{
public:
  TypeLabel(){
    val = TYPE_LABEL;
    oftype = nullptr;
    size = -1;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "TypeLabel()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "TypeLabel()";
    return s;
  }
  virtual bool operator==(const OurType &that) const override {
    return false;
  }
  virtual Value* compile() const override { return 0;}

};


class Integer: public OurType{
public:
  Integer(){ val = TYPE_INTEGER; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "Integer()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Integer()";
    return s;
  }
  virtual bool operator==(const OurType &that) const override {
    if(that.val == TYPE_INTEGER){
      return true;
    }
    return false;
  }
  virtual Value* compile() const override { return 0;}

};

class Char: public OurType{
public:
  Char(){ val = TYPE_CHAR; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "Char()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Char()";
    return s;
  }
  virtual bool operator==(const OurType &that) const override {
    if(that.val == TYPE_CHAR){
      return true;
    }
    return false;
  }
  virtual Value* compile() const override { return 0;}

};
class Real: public OurType{
public:
  Real(){ val = TYPE_REAL; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "Real()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Real()";
    return s;
  }
  virtual bool operator==(const OurType &that) const override {
    if(that.val == TYPE_REAL){
      return true;
    }
    return false;
  }
  virtual Value* compile() const override { return 0;}


};
class Boolean: public OurType{
public:
  Boolean(){ val = TYPE_BOOLEAN; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "Boolean()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Boolean()";
    return s;
  }
  virtual bool operator==(const OurType &that) const override {
    if(that.val == TYPE_BOOLEAN){
      return true;
    }
    return false;
  }
  virtual Value* compile() const override { return 0;}


};

class ProcedureType: public OurType{
public:
  ProcedureType(){ val = TYPE_PROCEDURE; oftype = nullptr; size = -1;}
  virtual void printOn(std::ostream &out) const override {
    out << "ProcedureType()";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "ProcedureType()";
    return s;
  }
  virtual Value* compile() const override { return 0;}


};

class Array: public OurType{
public:
  Array(OurType *t, int s = -1){
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
  virtual std::string getStringName() override {
    std::string s = "";
    if(size > 0){
      s += "Array(";
      s += "of size: " + size;
      s += " and type:";
      s += oftype->getStringName();
      s += ")";
      return s;
    }
    else{
      s += "Array(";
      s += " of type:";
      s += oftype->getStringName();
      s += ")";
      return s;
    }
  }
  virtual bool operator==(const OurType &that) const override {
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
  virtual Value* compile() const override { return 0;}

};

class Pointer: public OurType{
public:
  Pointer(OurType *t){
    val = TYPE_POINTER;
    oftype = t;
    size = -1;
  }
  virtual void printOn(std::ostream &out) const override {
    out << "Pointer(";
    out << " of type:"; oftype->printOn(out);
    out << ")";
  }
  virtual std::string getStringName() override {
    std::string s = "";
    s += "Pointer(";
    s += " of type:";
    s += oftype->getStringName();
    s += ")";
    return s;
  }
  virtual bool operator==(const OurType &that) const override {
    if(that.val == TYPE_NIL){
      return true;
    }
    if(that.val == TYPE_POINTER){
      if(*oftype == *that.oftype) return true;
    }
    return false;
  }
  virtual Value* compile() const override { return 0;}
  
};
