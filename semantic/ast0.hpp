#include "../error.h"
#include <iostream>
#include <vector>

class AST {
public:
  virtual ~AST() {}
  virtual void printOn(std::ostream &out) const = 0;
  virtual void sem() {}
  void ERROR (const char * fmt, ...){
    std::cout << fmt ;
  };
};
