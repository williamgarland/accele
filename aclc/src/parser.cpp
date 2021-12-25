#include "parser.hpp"

#include "exceptions.hpp"

namespace acl {
Token* Parser::lh(int pos) {
	sync(pos);
	return buffer[current + pos];
}

Token* Parser::match(TokenType type) {
	auto t = lh(0);
	if (t->type != type) throw TokenMismatchException(type, t);
	advance();
	return t;
}

void Parser::matchAndDelete(TokenType type) {
	auto t = match(type);
	if (!isSpeculating()) delete t;
}

void Parser::advance() {
	current++;
	if (current == buffer.size() && !isSpeculating()) {
		current = 0;
		buffer.clear();
	}
	sync(0);
}

void Parser::advanceAndDelete() {
	auto t = lh(0);
	advance();
	if (!isSpeculating()) delete t;
}

int Parser::mark() {
	int result = current;
	marks.push_back(result);
	return result;
}

void Parser::resetToMark() {
	current = marks.back();
	marks.pop_back();
}

void Parser::popMark() { marks.pop_back(); }

bool Parser::isSpeculating() { return !marks.empty(); }

bool Parser::hasNext() {
	return buffer[buffer.size() - 1]->type != TokenType::EOF_TOKEN;
}

void Parser::sync(int pos) {
	if (current + pos > (int)buffer.size() - 1) {
		fill((current + pos) - ((int)buffer.size() - 1));
	}
}

void Parser::fill(int n) {
	for (int i = 0; i < n; i++) {
		buffer.push_back(lexer.nextToken());
	}
}

Parser::Parser(Lexer&& lexer) : lexer(lexer), current(0) {}

Parser::~Parser() {}

Ast* Parser::parse() {
	sync(0);  // Insert initial token into buffer

	List<Node*> globalContent;

	return nullptr;
}
}  // namespace acl