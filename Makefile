.PHONY: clean distclean default

CXX=c++
CXXFLAGS=-Wall -std=c++11

lexer.cpp: lexer/lexer.l
	flex -s -o lexer/lexer.cpp lexer/lexer.l

clean:
	$(RM) lexer.cpp parser.cpp parser.hpp parser.output *.o

distclean: clean
	$(RM) minibasic
