.PHONY: clean distclean default

CXX=c++
CXXFLAGS=-Wall -std=c++11

default: pcl

lexer/lexer.cpp: lexer/lexer.l
	flex -s -o lexer/lexer.cpp lexer/lexer.l

lexer/lexer.o: lexer/lexer.cpp lexer/lexer.hpp parser/parser.hpp semantic/ast.hpp semantic/symbol.hpp

lexer/lexer: lexer/lexer.o

parser/parser.hpp parser/parser.cpp: parser/parser.y
	bison -dv -o parser/parser.cpp parser/parser.y

parser/parser.o: parser/parser.cpp lexer/lexer.hpp semantic/ast.hpp semantic/symbol.hpp

pcl: lexer/lexer.o parser/parser.o
	$(CXX) $(CXXFLAGS) -o pcl lexer/lexer.o parser/parser.o

clean:
	$(RM) lexer/lexer.cpp lexer/lexer lexer/*.o
	$(RM) parser/parser.cpp parser/*.cpp parser/*.o parser/parser.output parser/parser.hpp

distclean: clean
	$(RM) pcl
