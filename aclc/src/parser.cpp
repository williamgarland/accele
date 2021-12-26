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

Parser::Parser(CompilerContext& ctx, Lexer&& lexer)
	: ctx(ctx), lexer(lexer), current(0), currentScope(nullptr) {}

Parser::~Parser() {}

Ast* Parser::parse() {
	sync(0);  // Insert initial token into buffer

	GlobalScope* globalScope = new GlobalScope(lh(0)->meta, {});
	currentScope = globalScope;

	while (hasNext()) {
		auto t = lh(0);
		int current = 0;
		while (isModifier(t->type)) t = lh(++current);

		if (t->type == TokenType::FUN)
			globalScope->content.push_back(parseGlobalFunction());
		else if (t->type == TokenType::META_ENABLEWARNING ||
				 t->type == TokenType::META_DISABLEWARNING)
			globalScope->content.push_back(parseWarningMeta());
		else if (t->type == TokenType::VAR)
			globalScope->content.push_back(parseGlobalVariable());
		else if (t->type == TokenType::CONST)
			globalScope->content.push_back(parseGlobalConstant());
		else if (t->type == TokenType::ALIAS)
			globalScope->content.push_back(parseGlobalAlias());
		else if (t->type == TokenType::CLASS)
			globalScope->content.push_back(parseGlobalClass());
		else if (t->type == TokenType::STRUCT)
			globalScope->content.push_back(parseGlobalStruct());
		else if (t->type == TokenType::TEMPLATE)
			globalScope->content.push_back(parseGlobalTemplate());
		else if (t->type == TokenType::ENUM)
			globalScope->content.push_back(parseGlobalEnum());
		else if (t->type == TokenType::NAMESPACE)
			globalScope->content.push_back(parseGlobalNamespace());
		else if (t->type == TokenType::IMPORT)
			globalScope->content.push_back(parseImport());
		else if (t->type == TokenType::META_SRCLOCK)
			globalScope->content.push_back(
				parseSourceLock(globalScope->content));
		else if (t->type == TokenType::NL || t->type == TokenType::SEMICOLON)
			advanceAndDelete();
		else
			throw AclException(ASP_GLOBAL_SCOPE, t->meta,
							   "Invalid token in global scope");
	}

	return new Ast(globalScope);
}

void Parser::parseNewlineEquiv(bool greedy) {
	auto t = lh(0);
	if (t->type == TokenType::NL || t->type == TokenType::SEMICOLON) {
		advanceAndDelete();
		while (greedy && lh(0)->type == TokenType::NL ||
			   lh(0)->type == TokenType::SEMICOLON)
			advanceAndDelete();
	} else if (!isNewlineEquivalent(t->type))
		throw AclException(ASP_LINE_TERMINATION, t->meta,
						   "Expected newline or newline-equivalent token");
}

// ---------- Global Functions ---------- //
bool isModifier(TokenType type) {
	return type == TokenType::INTERNAL || type == TokenType::PUBLIC ||
		   type == TokenType::PRIVATE || type == TokenType::PROTECTED ||
		   type == TokenType::STATIC || type == TokenType::UNSAFE ||
		   type == TokenType::ATOMIC || type == TokenType::REF ||
		   type == TokenType::STRONG || type == TokenType::WEAK ||
		   type == TokenType::GREEDY || type == TokenType::FINAL ||
		   type == TokenType::OVERRIDE || type == TokenType::INFIX ||
		   type == TokenType::PREFIX || type == TokenType::POSTFIX ||
		   type == TokenType::THROWING || type == TokenType::NOEXCEPT ||
		   type == TokenType::ASYNC || type == TokenType::EXTERN ||
		   type == TokenType::META_DEPRECATED ||
		   type == TokenType::META_EXTERNALINIT ||
		   type == TokenType::META_STACKALLOC ||
		   type == TokenType::META_LAXTHROW || type == TokenType::META_NORETURN;
}

bool isNewlineEquivalent(TokenType type) {
	return type == TokenType::RBRACE || type == TokenType::RBRACKET ||
		   type == TokenType::RPAREN || type == TokenType::COMMA ||
		   type == TokenType::EOF_TOKEN;
}
}  // namespace acl