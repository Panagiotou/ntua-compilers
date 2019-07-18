.PHONY: clean distclean default

CXX=c++
CXXFLAGS=-Wall -std=c++11

default: lexer/lexer

lexer/lexer.cpp: lexer/lexer.l
	flex -s -o lexer/lexer.cpp lexer/lexer.l

lexer/lexer.o: lexer/lexer.cpp lexer/lexer.hpp

lexer/lexer: lexer/lexer.o

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) minibasic
