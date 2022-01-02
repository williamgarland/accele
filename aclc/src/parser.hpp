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
	std::deque<List<Token*>> queuedDeletedTokens;

   private:
	Token* lh(int pos);
	Token* match(TokenType type);
	void matchAndDelete(TokenType type);
	void advance();
	void advanceAndDelete();
	int mark();
	void resetToMark();
	void popMark(bool deleteQueuedTokens = true);
	bool isSpeculating();
	bool hasNext();
	void sync(int pos);
	void fill(int n);

	// Relex the current token and be as conservative as possible, meaning that
	// if only one character can be a token, it will be matched as such.
	Token* relex();

	void popScope();

   public:
	Parser(CompilerContext& ctx, Lexer&& lexer);
	~Parser();
	Ast* parse();

   private:
	Function* parseFunction(const TokenType* modifiersArray, int modifiersLen,
							bool allowOperatorIds);
	WarningMetaDeclaration* parseGlobalWarningMeta();
	Variable* parseNonClassVariable(const TokenType* modifiersArray,
									int modifiersLen);
	Variable* parseNonClassConstant(const TokenType* modifiersArray,
									int modifiersLen);
	Alias* parseAlias(const TokenType* modifiersArray, int modifiersLen);
	Class* parseClass(const TokenType* modifiersArray, int modifiersLen);
	Struct* parseStruct(const TokenType* modifiersArray, int modifiersLen);
	Template* parseTemplate(const TokenType* modifiersArray, int modifiersLen);
	Enum* parseEnum(const TokenType* modifiersArray, int modifiersLen);
	Namespace* parseNamespace(const TokenType* modifiersArray,
							  int modifiersLen);
	void parseClassContent(List<Node*>& dest);
	void parseTemplateContent(List<Node*>& dest);
	void parseEnumContent(List<Node*>& dest);
	void parseNamespaceContent(List<Node*>& dest);
	Variable* parseClassVariable();
	Variable* parseClassConstant();
	VariableBlock* parseVariableBlock(const SourceMeta& meta);
	FunctionBlock* parseGetBlock();
	SetBlock* parseSetBlock();
	FunctionBlock* parseInitBlock();
	Constructor* parseConstructor();
	Destructor* parseDestructor();
	Variable* parseTemplateVariable();
	Variable* parseTemplateConstant();
	EnumCase* parseEnumCase();
	Import* parseImport();
	Import* parseStandardImport();
	Import* parseFromImport();
	ImportSource* parseImportSource();
	ImportTarget* parseImportTarget();
	MetaDeclaration* parseSourceLock(const List<Node*>& globalContent);
	Node* parseGlobalContent();
	TypeRef* parseTypeRef();
	Expression* parseExpression();
	void parseModifiers(const TokenType* types, int typesLen,
						List<Modifier*>& dest);
	WarningMetaDeclaration* parseWarningMetaModifier();
	void parseParameters(List<Parameter*>& dest);
	Parameter* parseParameter();
	void parseGenerics(List<GenericType*>& dest);
	GenericType* parseGenericType();
	FunctionBlock* parseFunctionBlock();
	void parseFunctionBlockContent(List<Node*>& dest);
	Node* parseSingleFunctionBlockContent();
	IfBlock* parseIfBlock();
	WhileBlock* parseWhileBlock();
	RepeatBlock* parseRepeatBlock();
	ForBlock* parseForBlock();
	SwitchBlock* parseSwitchBlock();
	void parseSwitchBlockCases(List<SwitchCaseBlock*>& dest);
	TryBlock* parseTryBlock();
	CatchBlock* parseCatchBlock();
	Variable* parseLocalVariable();
	Variable* parseLocalConstant();
	ThrowStatement* parseThrowStatement();
	ReturnStatement* parseReturnStatement();
	WarningMetaDeclaration* parseLocalWarningMeta();
	TypeRef* parseTypeBase();
	TypeRef* parseTypeSuffix(TypeRef* base);
	SimpleTypeRef* parseSimpleTypeBase(SimpleTypeRef* parent);
	void parseGenericImpl(List<TypeRef*>& dest);
	FunctionTypeRef* parseFunctionTypeRef(TypeRef* parameters);
	TypeRef* parseSubscriptTypeRef(TypeRef* base);
	Expression* parseAssignmentExpression();
	Expression* parseL2Expression();
	Expression* parseLambdaExpression();
	void parseLambdaParameters(List<Parameter*>& dest);
	void parseLambdaBody(List<Node*>& dest);
	Expression* parseTernaryExpression();
	Expression* parseLogicalOrExpression();
	Expression* parseLogicalAndExpression();
	Expression* parseBitwiseOrExpression();
	Expression* parseBitwiseXorExpression();
	Expression* parseBitwiseAndExpression();
	Expression* parseEqualityExpression();
	Expression* parseRelationalExpression();
	Expression* parseNilCoalescingExpression();
	Expression* parseCastingExpression();
	Expression* parseRangeExpression();
	Expression* parseBitshiftExpression();
	Expression* parseAdditiveExpression();
	Expression* parseMultiplicativeExpression();
	Expression* parseExponentialExpression();
	Expression* parsePrefixExpression();
	Expression* parsePostfixExpression();
	Expression* parseAccessCallExpression();
	Expression* parseCallExpressionEnd(Expression* caller);
	Expression* parsePrimaryExpression();
	Expression* parseIdentifierExpression();
	Expression* parseArrayOrMapLiteralExpression();
	void parseExpressionList(List<Expression*>& dest);

	/*
	Parses the next newline (or equivalent) token.
	If the next token is a newline or a semicolon, the token will be advanced
	and deleted. If the next token is not a valid newline or newline
	equivalent token, an error will be thrown. If greedy is set to true, this
	function will consume all sequential newline and/or semicolon tokens (but
	only if the first token was a newline or semicolon token).
	*/
	void parseNewlineEquiv(bool greedy = true);

	/*
	Parses all next newline tokens.
	Only newline tokens (i.e. acl::TokenType::NL) will be parsed.
	This function will parse as many sequential newline tokens as it can.
	If the next token is not a newline, this function does nothing.
	*/
	void skipNewlines(bool includeSemicolons = false);
};

bool isModifier(TokenType type);
bool isNewlineEquivalent(TokenType type);
bool isTypeSuffixStart(TokenType type);
bool isGenericsStart(TokenType type);
bool isAssignmentOperator(TokenType type);
bool isEqualityOperator(TokenType type);
bool isRelationalOperator(TokenType type);
bool isCastingOperator(TokenType type);
bool isRangeOperator(TokenType type);
bool isBitshiftOperator(TokenType type);
bool isAdditiveOperator(TokenType type);
bool isMultiplicativeOperator(TokenType type);
bool isPrefixOperator(TokenType type);
bool isPostfixOperator(TokenType type);
bool isAccessOperator(TokenType type);
bool isCallOperator(TokenType type);
bool isLiteral(TokenType type);
bool isFunctionOperator(TokenType type);

// This includes modifiers for constants too
bool isLocalVariableModifier(TokenType type);
}  // namespace acl