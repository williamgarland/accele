#pragma once

#include <deque>

#include "ast.hpp"
#include "common.hpp"
#include "lexer.hpp"

namespace acl {
class Parser {
	Lexer lexer;
	List<Token*> buffer;
	int current;
	std::deque<int> marks;

   private:
	Token* lh(int pos);
	Token* match(TokenType type);
	void matchAndDelete(TokenType type);
	void advance();
	void advanceAndDelete();
	int mark();
	void resetToMark();
	void popMark();
	bool isSpeculating();
	bool hasNext();
	void sync(int pos);
	void fill(int n);

   public:
	Parser(Lexer&& lexer);
	~Parser();
	Ast* parse();
};
}  // namespace acl