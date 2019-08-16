class AST {
public:
  virtual ~AST() {}
  virtual void printOn(std::ostream &out) const = 0;
  virtual std::string getStringName(){ return "AST()";}
  virtual void sem() {}
  virtual void semForward(){}
  void ERROR (const char * fmt, ...){
    std::cout << fmt ;
  };
};
