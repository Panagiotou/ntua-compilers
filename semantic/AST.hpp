#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Transforms/Scalar.h>
#include <llvm/Transforms/Scalar/GVN.h>
using namespace llvm;

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
  // Global LLVM variables related to the LLVM suite.
  static LLVMContext TheContext;
  static IRBuilder<> Builder;
  static std::unique_ptr<Module> TheModule;
  static std::unique_ptr<legacy::FunctionPassManager> TheFPM;

  // Global LLVM variables related to the generated code.
  static GlobalVariable *TheVars;
  static GlobalVariable *TheRealVars;
  static GlobalVariable *TheNL;
  static Function *TheWriteInteger;
  static Function *TheWriteString;

  // Useful LLVM types.
  static Type *i1;
  static Type *i8;
  static Type *i32;
  static Type *i64;
  static Type *DoubleTyID;


  // Useful LLVM helper functions.
  ConstantInt* c1(char c) const {
    return ConstantInt::get(TheContext, APInt(1, c, true));
  }
  ConstantInt* c8(char c) const {
    return ConstantInt::get(TheContext, APInt(8, c, true));
  }
  ConstantInt* c32(int n) const {
    return ConstantInt::get(TheContext, APInt(32, n, true));
  }
  ConstantFP* fp32(double f) const {
    return ConstantFP::get(TheContext, APFloat(f));
  }
  virtual Value* compile() const = 0;
  virtual Value* compile_r() const = 0;
  void llvm_compile_and_dump() {
    // Initialize the module and the optimization passes.
    TheModule = make_unique<Module>("pcl program", TheContext);
    TheFPM = make_unique<legacy::FunctionPassManager>(TheModule.get());
    TheFPM->add(createPromoteMemoryToRegisterPass());
    TheFPM->add(createInstructionCombiningPass());
    TheFPM->add(createReassociatePass());
    TheFPM->add(createGVNPass());
    TheFPM->add(createCFGSimplificationPass());
    TheFPM->doInitialization();
    // Define and initialize global symbols.
    // @vars = global [26 x i32] zeroinitializer, align 16
    ArrayType *vars_type = ArrayType::get(i32, 26);
    TheVars = new GlobalVariable(
        *TheModule, vars_type, false, GlobalValue::PrivateLinkage,
        ConstantAggregateZero::get(vars_type), "vars");
    TheVars->setAlignment(16);

    ArrayType *real_vars_type = ArrayType::get(DoubleTyID, 26);
    TheRealVars = new GlobalVariable(
        *TheModule, real_vars_type, false, GlobalValue::PrivateLinkage,
        ConstantAggregateZero::get(real_vars_type), "real_vars");
    TheRealVars->setAlignment(16);

    // @nl = private constant [2 x i8] c"\0A\00", align 1
    ArrayType *nl_type = ArrayType::get(i8, 2);
    TheNL = new GlobalVariable(
        *TheModule, nl_type, true, GlobalValue::PrivateLinkage,
        ConstantArray::get(nl_type,
          std::vector<Constant *> { c8('\n'), c8('\0') }
        ), "nl");
    TheNL->setAlignment(1);
    // declare void @writeInteger(i64)
    FunctionType *writeInteger_type =
      FunctionType::get(Type::getVoidTy(TheContext),
                        std::vector<Type *> { i64 }, false);
    TheWriteInteger =
      Function::Create(writeInteger_type, Function::ExternalLinkage,
                       "writeInteger", TheModule.get());
    // declare void @writeString(i8*)
    FunctionType *writeString_type =
      FunctionType::get(Type::getVoidTy(TheContext),
                        std::vector<Type *> { PointerType::get(i8, 0) }, false);
    TheWriteString =
      Function::Create(writeString_type, Function::ExternalLinkage,
                       "writeString", TheModule.get());
    // Define and start the main function.
    Function *main =
      cast<Function>(TheModule->getOrInsertFunction("main", i32));
    BasicBlock *BB = BasicBlock::Create(TheContext, "entry", main);
    Builder.SetInsertPoint(BB);
    // Emit the program code.
    compile();
    // TheModule->print(outs(), nullptr);
    Builder.CreateRet(c32(0));
    // Verify the IR.
    bool bad = verifyModule(*TheModule, &errs());
    if (bad) {
      std::cerr << "The IR is bad!" << std::endl;
      std::exit(1);
    }
    TheFPM->run(*main);
    // Print out the IR.
    TheModule->print(outs(), nullptr);
  }

};
