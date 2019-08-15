class AST {
public:
  virtual ~AST() {}
  virtual void printOn(std::ostream &out) const = 0;
  virtual std::string getStringName(){ return "AST()";}
  virtual void sem() {}
  virtual void semPorF( AST *h, char* c = nullptr, AST *t = nullptr){}
  virtual void sem_(){}
  virtual void semForward(){}
  void ERROR (const char * fmt, ...){
    std::cout << fmt ;
  };
};
