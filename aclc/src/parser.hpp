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
	CompilerContext& ctx;
	Scope* currentScope;

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
	Parser(CompilerContext& ctx, Lexer&& lexer);
	~Parser();
	Ast* parse();

   private:
	Function* parseGlobalFunction();
	WarningMetaDeclaration* parseWarningMeta();
	Variable* parseGlobalVariable();
	Variable* parseGlobalConstant();
	Alias* parseGlobalAlias();
	Class* parseGlobalClass();
	Struct* parseGlobalStruct();
	Template* parseGlobalTemplate();
	Enum* parseGlobalEnum();
	Namespace* parseGlobalNamespace();
	Import* parseImport();
	MetaDeclaration* parseSourceLock(const List<Node*>& globalContent);

	/*
	Parses the next newline (or equivalent) token.
	If the next token is a newline or a semicolon, the token will be advanced
	and deleted. If the next token is not a valid newline or newline
	equivalent token, an error will be thrown. If greedy is set to true, this
	function will consume all sequential newline and/or semicolon tokens (but
	only if the first token was a newline or semicolon token).
	*/
	void parseNewlineEquiv(bool greedy = true);
};

bool isModifier(TokenType type);
bool isNewlineEquivalent(TokenType type);
}  // namespace acl