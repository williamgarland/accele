#include "parser.hpp"

#include <filesystem>

namespace {
constexpr int GLOBAL_FUNCTION_MODIFIERS_LEN = 10;
const acl::TokenType GLOBAL_FUNCTION_MODIFIERS[GLOBAL_FUNCTION_MODIFIERS_LEN] =
	{acl::TokenType::INTERNAL,
	 acl::TokenType::UNSAFE,
	 acl::TokenType::THROWING,
	 acl::TokenType::NOEXCEPT,
	 acl::TokenType::ASYNC,
	 acl::TokenType::EXTERN,
	 acl::TokenType::META_NORETURN,
	 acl::TokenType::META_DEPRECATED,
	 acl::TokenType::META_ENABLEWARNING,
	 acl::TokenType::META_DISABLEWARNING};

constexpr int LAMBDA_MODIFIERS_LEN = 2;
const acl::TokenType LAMBDA_MODIFIERS[LAMBDA_MODIFIERS_LEN] = {
	acl::TokenType::ASYNC, acl::TokenType::UNSAFE};

constexpr int GLOBAL_VARIABLE_MODIFIERS_LEN = 9;
const acl::TokenType GLOBAL_VARIABLE_MODIFIERS[GLOBAL_VARIABLE_MODIFIERS_LEN] =
	{acl::TokenType::INTERNAL,
	 acl::TokenType::ATOMIC,
	 acl::TokenType::GREEDY,
	 acl::TokenType::STRONG,
	 acl::TokenType::WEAK,
	 acl::TokenType::UNSAFE,
	 acl::TokenType::META_DEPRECATED,
	 acl::TokenType::META_ENABLEWARNING,
	 acl::TokenType::META_DISABLEWARNING};

constexpr int GLOBAL_ALIAS_MODIFIERS_LEN = 4;
const acl::TokenType GLOBAL_ALIAS_MODIFIERS[GLOBAL_ALIAS_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL, acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING, acl::TokenType::META_DISABLEWARNING};

constexpr int PARAMETER_MODIFIERS_LEN = 8;
const acl::TokenType PARAMETER_MODIFIERS[PARAMETER_MODIFIERS_LEN] = {
	acl::TokenType::ATOMIC,
	acl::TokenType::GREEDY,
	acl::TokenType::STRONG,
	acl::TokenType::WEAK,
	acl::TokenType::REF,
	acl::TokenType::CONST,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING};

constexpr int FUNCTION_BLOCK_MODIFIERS_LEN = 3;
const acl::TokenType FUNCTION_BLOCK_MODIFIERS[FUNCTION_BLOCK_MODIFIERS_LEN] = {
	acl::TokenType::UNSAFE, acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING};

constexpr int LOCAL_VARIABLE_MODIFIERS_LEN = 8;
const acl::TokenType LOCAL_VARIABLE_MODIFIERS[LOCAL_VARIABLE_MODIFIERS_LEN] = {
	acl::TokenType::REF,
	acl::TokenType::ATOMIC,
	acl::TokenType::GREEDY,
	acl::TokenType::STRONG,
	acl::TokenType::WEAK,
	acl::TokenType::UNSAFE,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING};

constexpr int LOCAL_ALIAS_MODIFIERS_LEN = 2;
const acl::TokenType LOCAL_ALIAS_MODIFIERS[LOCAL_ALIAS_MODIFIERS_LEN] = {
	acl::TokenType::META_ENABLEWARNING, acl::TokenType::META_DISABLEWARNING};

constexpr int GLOBAL_CLASS_MODIFIERS_LEN = 6;
const acl::TokenType GLOBAL_CLASS_MODIFIERS[GLOBAL_CLASS_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,			acl::TokenType::FINAL,
	acl::TokenType::META_LAXTHROW,		acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING, acl::TokenType::META_DISABLEWARNING};

constexpr int GLOBAL_STRUCT_MODIFIERS_LEN = 5;
const acl::TokenType GLOBAL_STRUCT_MODIFIERS[GLOBAL_STRUCT_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL, acl::TokenType::META_LAXTHROW,
	acl::TokenType::META_DEPRECATED, acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING};

constexpr int GLOBAL_TEMPLATE_MODIFIERS_LEN = 4;
const acl::TokenType GLOBAL_TEMPLATE_MODIFIERS[GLOBAL_TEMPLATE_MODIFIERS_LEN] =
	{acl::TokenType::INTERNAL, acl::TokenType::META_DEPRECATED,
	 acl::TokenType::META_ENABLEWARNING, acl::TokenType::META_DISABLEWARNING};

constexpr int GLOBAL_ENUM_MODIFIERS_LEN = 4;
const acl::TokenType GLOBAL_ENUM_MODIFIERS[GLOBAL_ENUM_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL, acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING, acl::TokenType::META_DISABLEWARNING};

constexpr int GLOBAL_NAMESPACE_MODIFIERS_LEN = 4;
const acl::TokenType
	GLOBAL_NAMESPACE_MODIFIERS[GLOBAL_NAMESPACE_MODIFIERS_LEN] = {
		acl::TokenType::INTERNAL, acl::TokenType::META_DEPRECATED,
		acl::TokenType::META_ENABLEWARNING,
		acl::TokenType::META_DISABLEWARNING};

constexpr int CLASS_CLASS_MODIFIERS_LEN = 9;
const acl::TokenType CLASS_CLASS_MODIFIERS[CLASS_CLASS_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::FINAL,
	acl::TokenType::META_LAXTHROW,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED};

constexpr int CLASS_STRUCT_MODIFIERS_LEN = 8;
const acl::TokenType CLASS_STRUCT_MODIFIERS[CLASS_STRUCT_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::META_LAXTHROW,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED};

constexpr int CLASS_TEMPLATE_MODIFIERS_LEN = 7;
const acl::TokenType CLASS_TEMPLATE_MODIFIERS[CLASS_TEMPLATE_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED};

constexpr int CLASS_ENUM_MODIFIERS_LEN = 7;
const acl::TokenType CLASS_ENUM_MODIFIERS[CLASS_ENUM_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED};

constexpr int CLASS_NAMESPACE_MODIFIERS_LEN = 7;
const acl::TokenType CLASS_NAMESPACE_MODIFIERS[CLASS_NAMESPACE_MODIFIERS_LEN] =
	{acl::TokenType::INTERNAL,
	 acl::TokenType::META_DEPRECATED,
	 acl::TokenType::META_ENABLEWARNING,
	 acl::TokenType::META_DISABLEWARNING,
	 acl::TokenType::PUBLIC,
	 acl::TokenType::PRIVATE,
	 acl::TokenType::PROTECTED};

constexpr int NAMESPACE_CLASS_MODIFIERS_LEN = 8;
const acl::TokenType NAMESPACE_CLASS_MODIFIERS[NAMESPACE_CLASS_MODIFIERS_LEN] =
	{acl::TokenType::INTERNAL,
	 acl::TokenType::FINAL,
	 acl::TokenType::META_LAXTHROW,
	 acl::TokenType::META_DEPRECATED,
	 acl::TokenType::META_ENABLEWARNING,
	 acl::TokenType::META_DISABLEWARNING,
	 acl::TokenType::PUBLIC,
	 acl::TokenType::PRIVATE};

constexpr int NAMESPACE_STRUCT_MODIFIERS_LEN = 7;
const acl::TokenType
	NAMESPACE_STRUCT_MODIFIERS[NAMESPACE_STRUCT_MODIFIERS_LEN] = {
		acl::TokenType::INTERNAL,
		acl::TokenType::META_LAXTHROW,
		acl::TokenType::META_DEPRECATED,
		acl::TokenType::META_ENABLEWARNING,
		acl::TokenType::META_DISABLEWARNING,
		acl::TokenType::PUBLIC,
		acl::TokenType::PRIVATE};

constexpr int NAMESPACE_TEMPLATE_MODIFIERS_LEN = 6;
const acl::TokenType
	NAMESPACE_TEMPLATE_MODIFIERS[NAMESPACE_TEMPLATE_MODIFIERS_LEN] = {
		acl::TokenType::INTERNAL,
		acl::TokenType::META_DEPRECATED,
		acl::TokenType::META_ENABLEWARNING,
		acl::TokenType::META_DISABLEWARNING,
		acl::TokenType::PUBLIC,
		acl::TokenType::PRIVATE};

constexpr int NAMESPACE_ENUM_MODIFIERS_LEN = 6;
const acl::TokenType NAMESPACE_ENUM_MODIFIERS[NAMESPACE_ENUM_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE};

constexpr int NAMESPACE_NAMESPACE_MODIFIERS_LEN = 6;
const acl::TokenType
	NAMESPACE_NAMESPACE_MODIFIERS[NAMESPACE_NAMESPACE_MODIFIERS_LEN] = {
		acl::TokenType::INTERNAL,
		acl::TokenType::META_DEPRECATED,
		acl::TokenType::META_ENABLEWARNING,
		acl::TokenType::META_DISABLEWARNING,
		acl::TokenType::PUBLIC,
		acl::TokenType::PRIVATE};

constexpr int CLASS_VARIABLE_MODIFIERS_LEN = 13;
const acl::TokenType CLASS_VARIABLE_MODIFIERS[CLASS_VARIABLE_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::ATOMIC,
	acl::TokenType::GREEDY,
	acl::TokenType::STRONG,
	acl::TokenType::WEAK,
	acl::TokenType::UNSAFE,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED,
	acl::TokenType::STATIC};

constexpr int CLASS_ALIAS_MODIFIERS_LEN = 7;
const acl::TokenType CLASS_ALIAS_MODIFIERS[CLASS_ALIAS_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED};

constexpr int NAMESPACE_ALIAS_MODIFIERS_LEN = 6;
const acl::TokenType NAMESPACE_ALIAS_MODIFIERS[NAMESPACE_ALIAS_MODIFIERS_LEN] =
	{acl::TokenType::INTERNAL,
	 acl::TokenType::META_DEPRECATED,
	 acl::TokenType::META_ENABLEWARNING,
	 acl::TokenType::META_DISABLEWARNING,
	 acl::TokenType::PUBLIC,
	 acl::TokenType::PRIVATE};

constexpr int NAMESPACE_VARIABLE_MODIFIERS_LEN = 11;
const acl::TokenType
	NAMESPACE_VARIABLE_MODIFIERS[NAMESPACE_VARIABLE_MODIFIERS_LEN] = {
		acl::TokenType::INTERNAL,
		acl::TokenType::ATOMIC,
		acl::TokenType::GREEDY,
		acl::TokenType::STRONG,
		acl::TokenType::WEAK,
		acl::TokenType::UNSAFE,
		acl::TokenType::META_DEPRECATED,
		acl::TokenType::META_ENABLEWARNING,
		acl::TokenType::META_DISABLEWARNING,
		acl::TokenType::PUBLIC,
		acl::TokenType::PRIVATE};

constexpr int NAMESPACE_FUNCTION_MODIFIERS_LEN = 12;
const acl::TokenType
	NAMESPACE_FUNCTION_MODIFIERS[NAMESPACE_FUNCTION_MODIFIERS_LEN] = {
		acl::TokenType::INTERNAL,
		acl::TokenType::UNSAFE,
		acl::TokenType::THROWING,
		acl::TokenType::NOEXCEPT,
		acl::TokenType::ASYNC,
		acl::TokenType::EXTERN,
		acl::TokenType::META_NORETURN,
		acl::TokenType::META_DEPRECATED,
		acl::TokenType::META_ENABLEWARNING,
		acl::TokenType::META_DISABLEWARNING,
		acl::TokenType::PUBLIC,
		acl::TokenType::PRIVATE};

constexpr int ENUM_CASE_MODIFIERS_LEN = 7;
const acl::TokenType ENUM_CASE_MODIFIERS[ENUM_CASE_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::UNSAFE,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING};

constexpr int CLASS_FUNCTION_MODIFIERS_LEN = 19;
const acl::TokenType CLASS_FUNCTION_MODIFIERS[CLASS_FUNCTION_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::UNSAFE,
	acl::TokenType::THROWING,
	acl::TokenType::NOEXCEPT,
	acl::TokenType::ASYNC,
	acl::TokenType::EXTERN,
	acl::TokenType::META_NORETURN,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED,
	acl::TokenType::STATIC,
	acl::TokenType::FINAL,
	acl::TokenType::OVERRIDE,
	acl::TokenType::INFIX,
	acl::TokenType::PREFIX,
	acl::TokenType::POSTFIX};

constexpr int TEMPLATE_FUNCTION_MODIFIERS_LEN = 18;
const acl::TokenType
	TEMPLATE_FUNCTION_MODIFIERS[TEMPLATE_FUNCTION_MODIFIERS_LEN] = {
		acl::TokenType::INTERNAL,
		acl::TokenType::UNSAFE,
		acl::TokenType::THROWING,
		acl::TokenType::NOEXCEPT,
		acl::TokenType::ASYNC,
		acl::TokenType::EXTERN,
		acl::TokenType::META_NORETURN,
		acl::TokenType::META_DEPRECATED,
		acl::TokenType::META_ENABLEWARNING,
		acl::TokenType::META_DISABLEWARNING,
		acl::TokenType::PUBLIC,
		acl::TokenType::PRIVATE,
		acl::TokenType::PROTECTED,
		acl::TokenType::STATIC,
		acl::TokenType::OVERRIDE,
		acl::TokenType::INFIX,
		acl::TokenType::PREFIX,
		acl::TokenType::POSTFIX};

constexpr int ENUM_FUNCTION_MODIFIERS_LEN = 17;
const acl::TokenType ENUM_FUNCTION_MODIFIERS[ENUM_FUNCTION_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::UNSAFE,
	acl::TokenType::THROWING,
	acl::TokenType::NOEXCEPT,
	acl::TokenType::ASYNC,
	acl::TokenType::EXTERN,
	acl::TokenType::META_NORETURN,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::STATIC,
	acl::TokenType::OVERRIDE,
	acl::TokenType::INFIX,
	acl::TokenType::PREFIX,
	acl::TokenType::POSTFIX};

constexpr int CONSTRUCTOR_MODIFIERS_LEN = 10;
const acl::TokenType CONSTRUCTOR_MODIFIERS[CONSTRUCTOR_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::UNSAFE,
	acl::TokenType::THROWING,
	acl::TokenType::NOEXCEPT,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED};

constexpr int DESTRUCTOR_MODIFIERS_LEN = 2;
const acl::TokenType DESTRUCTOR_MODIFIERS[DESTRUCTOR_MODIFIERS_LEN] = {
	acl::TokenType::META_ENABLEWARNING, acl::TokenType::META_DISABLEWARNING};

constexpr int GET_BLOCK_MODIFIERS_LEN = 11;
const acl::TokenType GET_BLOCK_MODIFIERS[GET_BLOCK_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::UNSAFE,
	acl::TokenType::THROWING,
	acl::TokenType::NOEXCEPT,
	acl::TokenType::ASYNC,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED};

constexpr int SET_BLOCK_MODIFIERS_LEN = 11;
const acl::TokenType SET_BLOCK_MODIFIERS[SET_BLOCK_MODIFIERS_LEN] = {
	acl::TokenType::INTERNAL,
	acl::TokenType::UNSAFE,
	acl::TokenType::THROWING,
	acl::TokenType::NOEXCEPT,
	acl::TokenType::ASYNC,
	acl::TokenType::META_DEPRECATED,
	acl::TokenType::META_ENABLEWARNING,
	acl::TokenType::META_DISABLEWARNING,
	acl::TokenType::PUBLIC,
	acl::TokenType::PRIVATE,
	acl::TokenType::PROTECTED};

constexpr int INIT_BLOCK_MODIFIERS_LEN = 2;
const acl::TokenType INIT_BLOCK_MODIFIERS[INIT_BLOCK_MODIFIERS_LEN] = {
	acl::TokenType::META_ENABLEWARNING, acl::TokenType::META_DISABLEWARNING};
}  // namespace

namespace acl {
void getTypesForPanicTerminator(PanicTerminator terminator,
								List<TokenType>& dest) {
	switch (terminator) {
		case PanicTerminator::BLOCK_END:
			dest.push_back(TokenType::RBRACE);
			break;
		case PanicTerminator::STATEMENT_END:
			dest.push_back(TokenType::NL);
			dest.push_back(TokenType::SEMICOLON);
			dest.push_back(TokenType::EOF_TOKEN);
			dest.push_back(TokenType::RBRACE);
			break;
		default:
			break;
	}
}

Token* Parser::lh(int pos) {
	sync(pos);
	return buffer[current + pos];
}

Token* Parser::match(TokenType type) {
	auto t = lh(0);
	if (t->type != type) {
		if (!isSpeculating()) diagnoser.diagnoseInvalidToken(type, t);
		panic();
	}
	advance();
	return t;
}

void Parser::matchAndDelete(TokenType type) {
	auto t = match(type);
	if (isSpeculating())
		queuedDeletedTokens.back().push_back(t);
	else
		delete t;
}

void Parser::advance() {
	current++;
	if ((std::size_t)current == buffer.size() && !isSpeculating()) {
		current = 0;
		buffer.clear();
	}
	sync(0);
}

void Parser::advanceAndDelete() {
	auto t = lh(0);
	advance();
	if (isSpeculating())
		queuedDeletedTokens.back().push_back(t);
	else
		delete t;
}

int Parser::mark() {
	int result = current;
	marks.push_back(result);
	queuedDeletedTokens.push_back({});
	return result;
}

void Parser::resetToMark() {
	current = marks.back();
	marks.pop_back();
	queuedDeletedTokens.pop_back();
}

void Parser::popMark(bool deleteQueuedTokens) {
	marks.pop_back();
	auto& queued = queuedDeletedTokens.back();
	if (deleteQueuedTokens) {
		for (auto& t : queued) delete t;
	}
	queuedDeletedTokens.pop_back();
}

bool Parser::isSpeculating() { return !marks.empty(); }

bool Parser::hasNext() {
	sync(0);
	return buffer[buffer.size() - 1]->type != TokenType::EOF_TOKEN;
}

void Parser::sync(int pos) {
	if (current + pos > (int)buffer.size() - 1) {
		fill((current + pos) - ((int)buffer.size() - 1));
	}
}

void Parser::fill(int n) {
	for (int i = 0; i < n; i++) {
		try {
			auto t = lexer.nextToken();
			buffer.push_back(t);
		} catch (LexerPanicException& e) {
			panic();
		}
	}
}

[[noreturn]] void Parser::panic() {
	List<TokenType> targetTypes;
	getTypesForPanicTerminator(panicTerminator, targetTypes);
	while (hasNext() && !listContains(targetTypes, lh(0)->type)) advance();
	panicking = true;
	if (!isSpeculating()) didPanic = true;
	throw ParserPanicException();
}

Token* Parser::relex() {
	List<Token*> newTokens;
	Relexer(ctx, lh(0)).relex(newTokens);

	if (newTokens.empty()) {
		return lh(0);  // We couldn't relex it, so just return the current token
	}

	buffer.erase(buffer.begin() + current);
	buffer.insert(buffer.begin() + current, newTokens.begin(), newTokens.end());

	return newTokens[0];
}

void Parser::popScope() {
	if (isFunctionScope(currentScope)) {
		List<Symbol*> newSymbols;
		for (auto& symbol : currentScope->symbols) {
			if (dynamic_cast<Parameter*>(symbol) ||
				dynamic_cast<GenericType*>(symbol)) {
				newSymbols.push_back(symbol);
			}
		}
		currentScope->symbols.clear();
		for (auto& ns : newSymbols) currentScope->symbols.push_back(ns);
	}
	currentScope = currentScope->parentScope;
}

Parser::Parser(CompilerContext& ctx, Lexer&& lexer)
	: ctx(ctx),
	  lexer(lexer),
	  current(0),
	  currentScope(nullptr),
	  panicking(false),
	  didPanic(false),
	  diagnoser(ctx, std::cout) {}

Parser::~Parser() {}

Ast* Parser::parse() {
	lexer.setRecoverySentinels({'\r', '\n', ';'});
	panicTerminator = PanicTerminator::STATEMENT_END;
	sync(0);  // Insert initial token into buffer

	GlobalScope* globalScope = new GlobalScope(lh(0)->meta, {});
	currentScope = globalScope;

	skipNewlines(true);
	while (hasNext()) {
		lexer.setRecoverySentinels({'\r', '\n', ';'});
		try {
			auto content = parseGlobalContent();
			globalScope->content.push_back(content);
		} catch (ParserPanicException& e) {
			panicking = false;

			if (lh(0)->type == TokenType::RBRACE) advance();
		}

		skipNewlines(true);
	}

	if (didPanic) {
		exit(1);
	}

	return new Ast(globalScope);
}

Node* Parser::parseGlobalContent() {
	panicTerminator = PanicTerminator::STATEMENT_END;

	try {
		skipNewlines(true);

		auto t = lh(0);
		int current = 0;
		while (isModifier(t->type) || t->type == TokenType::NL) {
			if (t->type == TokenType::META_ENABLEWARNING ||
				t->type == TokenType::META_DISABLEWARNING) {
				// 3 for keyword, lparen, and initial string literal
				current += 3;

				// 2 for comma and next string literal
				while (lh(current)->type == TokenType::COMMA) current += 2;

				t = lh(++current);
			} else
				t = lh(++current);
		}

		if (t->type == TokenType::FUN)
			return parseFunction(GLOBAL_FUNCTION_MODIFIERS,
								 GLOBAL_FUNCTION_MODIFIERS_LEN, false);
		else if (t->type == TokenType::META_ENABLEWARNING ||
				 t->type == TokenType::META_DISABLEWARNING)
			return parseGlobalWarningMeta();
		else if (t->type == TokenType::META_NOBUILTINS) {
			advance();
			parseNewlineEquiv();
			return new MetaDeclaration(t);
		} else if (t->type == TokenType::VAR)
			return parseNonClassVariable(GLOBAL_VARIABLE_MODIFIERS,
										 GLOBAL_VARIABLE_MODIFIERS_LEN);
		else if (t->type == TokenType::CONST)
			return parseNonClassConstant(GLOBAL_VARIABLE_MODIFIERS,
										 GLOBAL_VARIABLE_MODIFIERS_LEN);
		else if (t->type == TokenType::ALIAS)
			return parseAlias(GLOBAL_ALIAS_MODIFIERS,
							  GLOBAL_ALIAS_MODIFIERS_LEN);
		else if (t->type == TokenType::CLASS)
			return parseClass(GLOBAL_CLASS_MODIFIERS,
							  GLOBAL_CLASS_MODIFIERS_LEN);
		else if (t->type == TokenType::STRUCT)
			return parseStruct(GLOBAL_STRUCT_MODIFIERS,
							   GLOBAL_STRUCT_MODIFIERS_LEN);
		else if (t->type == TokenType::TEMPLATE)
			return parseTemplate(GLOBAL_TEMPLATE_MODIFIERS,
								 GLOBAL_TEMPLATE_MODIFIERS_LEN);
		else if (t->type == TokenType::ENUM)
			return parseEnum(GLOBAL_ENUM_MODIFIERS, GLOBAL_ENUM_MODIFIERS_LEN);
		else if (t->type == TokenType::NAMESPACE)
			return parseNamespace(GLOBAL_NAMESPACE_MODIFIERS,
								  GLOBAL_NAMESPACE_MODIFIERS_LEN);
		else if (t->type == TokenType::IMPORT) {
			auto result = parseImport();
			dynamic_cast<GlobalScope*>(currentScope)->addImport(result);
			return result;
		} else if (t->type == TokenType::META_SRCLOCK)
			return parseSourceLock(
				dynamic_cast<GlobalScope*>(currentScope)->content);
		StringBuffer sb;
		sb << "Unexpected token " << t->data << " in global scope";
		if (!isSpeculating())
			diagnoser.diagnose(ec::INVALID_TOKEN, t->meta, t->data.length(),
							   sb.str());
		panic();
	} catch (DuplicateSymbolException& e) {
		if (!isSpeculating())
			diagnoser.diagnoseDuplicateSymbol(e.original, e.duplicate);
		panic();
	} catch (DuplicateImportException& e) {
		if (!isSpeculating())
			diagnoser.diagnoseDuplicateImport(e.original, e.duplicate);
	} catch (ParserPanicException& e) {
		panicTerminator = PanicTerminator::STATEMENT_END;
		panic();
	} catch (AcceleException& e) {
		if (!isSpeculating()) {
			if (e.sourceMeta)
				diagnoser.diagnose(e.ec, *e.sourceMeta, e.highlightLength,
								   e.message);
			else
				diagnoser.diagnose(e.ec, e.message);
		}
		panic();
	}

	panic();
}

Function* Parser::parseFunction(const TokenType* modifiersArray,
								int modifiersLen, bool allowOperatorIds) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::FUN);

	// You can't have global operator functions, so we only accept identifiers
	// here
	Token* id;
	if (allowOperatorIds && isFunctionOperator(lh(0)->type)) {
		id = lh(0);
		advance();
	} else
		id = match(TokenType::ID);

	List<GenericType*> generics;
	if (lh(0)->type == TokenType::LT) {
		parseGenerics(generics);
	}

	matchAndDelete(TokenType::LPAREN);
	List<Parameter*> parameters;
	parseParameters(parameters);
	matchAndDelete(TokenType::RPAREN);

	TypeRef* declaredReturnType = nullptr;
	if (lh(0)->type == TokenType::MINUS_ARROW) {
		advanceAndDelete();
		declaredReturnType = parseTypeRef();
	}

	Function* function =
		new Function(modifiers, id, generics, parameters, declaredReturnType,
					 {}, currentScope, false);
	currentScope->addSymbol(function);
	currentScope = function;

	if (lh(0)->type == TokenType::EQUALS) {
		auto meta = lh(0)->meta;
		advanceAndDelete();
		ReturnStatement* returnStatement =
			new ReturnStatement(meta, parseExpression());
		function->content.push_back(returnStatement);
		parseNewlineEquiv();

		function->hasBody = true;
	} else if (lh(0)->type == TokenType::LBRACE) {
		advanceAndDelete();
		parseFunctionBlockContent(function->content);
		matchAndDelete(TokenType::RBRACE);

		function->hasBody = true;
	} else {
		parseNewlineEquiv();
	}

	popScope();

	return function;
}

void Parser::parseModifiers(const TokenType* types, int typesLen,
							List<Modifier*>& dest) {
	skipNewlines();
	auto t = lh(0);
	while (isModifier(t->type)) {
		bool foundModifier = false;
		for (int i = 0; i < typesLen; i++) {
			if (t->type == types[i]) {
				if (t->type == TokenType::META_ENABLEWARNING ||
					t->type == TokenType::META_DISABLEWARNING) {
					dest.push_back(parseWarningMetaModifier());
				} else {
					dest.push_back(new Modifier{t});
					advance();
				}
				t = lh(0);
				foundModifier = true;
				break;
			}
		}
		if (!foundModifier) {
			if (!isSpeculating()) diagnoser.diagnoseInvalidModifier(t);
			panic();
		}
		skipNewlines();
		t = lh(0);
	}
}

WarningMetaDeclaration* Parser::parseWarningMetaModifier() {
	Token* t = lh(0);
	advance();
	matchAndDelete(TokenType::LPAREN);
	List<Token*> args;
	args.push_back(match(TokenType::STRING_LITERAL));
	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		args.push_back(match(TokenType::STRING_LITERAL));
	}
	matchAndDelete(TokenType::RPAREN);
	return new WarningMetaDeclaration(t, args, nullptr);
}

void Parser::parseNewlineEquiv(bool greedy) {
	auto t = lh(0);
	if (t->type == TokenType::NL || t->type == TokenType::SEMICOLON) {
		advanceAndDelete();
		while (greedy && (lh(0)->type == TokenType::NL ||
						  lh(0)->type == TokenType::SEMICOLON))
			advanceAndDelete();
	} else if (!isNewlineEquivalent(t->type)) {
		if (!isSpeculating())
			diagnoser.diagnoseInvalidToken(
				"newline or newline-equivalent token", t);
		panic();
	}
}

int Parser::skipNewlines(bool includeSemicolons) {
	int result = 0;
	while (lh(0)->type == TokenType::NL ||
		   (includeSemicolons && lh(0)->type == TokenType::SEMICOLON)) {
		advanceAndDelete();
		result++;
	}
	return result;
}

#ifndef __GNUC__
#pragma region TypeRef
#endif

TypeRef* Parser::parseTypeRef() {
	TypeRef* result;
	if (lh(0)->type == TokenType::LPAREN && lh(1)->type == TokenType::RPAREN &&
		lh(1)->type == TokenType::MINUS_ARROW)
		result = parseFunctionTypeRef(nullptr);
	else
		result = parseTypeBase();
	while (isTypeSuffixStart(lh(0)->type)) result = parseTypeSuffix(result);
	return result;
}

TypeRef* Parser::parseTypeBase() {
	auto t = lh(0);
	if (t->type == TokenType::LPAREN) {
		auto meta = t->meta;
		advanceAndDelete();
		List<TypeRef*> elements;
		elements.push_back(parseTypeRef());
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			elements.push_back(parseTypeRef());
		}
		matchAndDelete(TokenType::RPAREN);
		return new TupleTypeRef(meta, elements);
	} else if (t->type == TokenType::LBRACKET) {
		auto meta = t->meta;
		advanceAndDelete();
		TypeRef* key = parseTypeRef();
		matchAndDelete(TokenType::COLON);
		TypeRef* value = parseTypeRef();
		matchAndDelete(TokenType::RBRACKET);
		return new MapTypeRef(meta, key, value);
	} else if (t->type == TokenType::GLOBAL) {
		advance();
		skipNewlines();
		matchAndDelete(TokenType::DOT);
		skipNewlines();
		return parseSimpleTypeBase(new SimpleTypeRef(t->meta, t, {}, nullptr));
	} else {
		return parseSimpleTypeBase(nullptr);
	}
}

SimpleTypeRef* Parser::parseSimpleTypeBase(SimpleTypeRef* parent) {
	auto id = match(TokenType::ID);
	List<TypeRef*> generics;
	if (isGenericsStart(lh(0)->type)) parseGenericImpl(generics);

	SimpleTypeRef* result = new SimpleTypeRef(id->meta, id, generics, parent);

	if (lh(0)->type == TokenType::DOT) {
		advanceAndDelete();
		return parseSimpleTypeBase(result);
	}

	return result;
}

TypeRef* Parser::parseTypeSuffix(TypeRef* base) {
	auto t = lh(0);
	if (t->type == TokenType::MINUS_ARROW) return parseFunctionTypeRef(base);
	if (t->type == TokenType::LBRACKET) return parseSubscriptTypeRef(base);
	if (t->type == TokenType::TRIPLE_DOT ||
		t->type == TokenType::EXCLAMATION_POINT ||
		t->type == TokenType::QUESTION_MARK || t->type == TokenType::ASTERISK) {
		advance();
		return new SuffixTypeRef(t->meta, base, t);
	}
	if (t->type == TokenType::DOUBLE_QUESTION_MARK ||
		t->type == TokenType::QUESTION_MARK_DOT ||
		t->type == TokenType::DOUBLE_ASTERISK ||
		t->type == TokenType::ASTERISK_EQUALS ||
		t->type == TokenType::DOUBLE_ASTERISK_EQUALS) {
		auto actualSuffix = relex();
		advance();
		return new SuffixTypeRef(actualSuffix->meta, base, actualSuffix);
	}

	if (!isSpeculating())
		diagnoser.diagnose(ec::UNKNOWN, t->meta, t->data.length(),
						   "Invalid type reference suffix");
	panic();
}

FunctionTypeRef* Parser::parseFunctionTypeRef(TypeRef* parameters) {
	List<TypeRef*> parameterTypes;
	if (parameters) {
		if (TupleTypeRef* asTuple = dynamic_cast<TupleTypeRef*>(parameters)) {
			for (auto& t : asTuple->elementTypes) parameterTypes.push_back(t);
			asTuple->deleteElementTypes = false;
			delete parameters;
		} else {
			parameterTypes.push_back(parameters);
		}
	} else {
		matchAndDelete(TokenType::LPAREN);
		matchAndDelete(TokenType::RPAREN);
	}
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::MINUS_ARROW);
	TypeRef* returnType = parseTypeRef();
	return new FunctionTypeRef(meta, parameterTypes, returnType);
}

TypeRef* Parser::parseSubscriptTypeRef(TypeRef* base) {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::LBRACKET);
	TypeRef* keyType = nullptr;
	if (lh(0)->type != TokenType::RBRACKET) keyType = parseTypeRef();
	matchAndDelete(TokenType::RBRACKET);
	if (keyType) return new MapTypeRef(meta, keyType, base);
	return new ArrayTypeRef(meta, base);
}

#ifndef __GNUC__
#pragma endregion
#endif

#ifndef __GNUC__
#pragma region Expression
#endif

Expression* Parser::parseExpression() { return parseAssignmentExpression(); }

Expression* Parser::parseAssignmentExpression() {
	auto left = parseL2Expression();
	if (isAssignmentOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		auto right = parseAssignmentExpression();
		return new BinaryExpression(op->meta, op, left, right);
	}
	return left;
}

Expression* Parser::parseL2Expression() {
	mark();
	try {
		auto result = parseLambdaExpression();
		popMark(true);
		return result;
	} catch (AcceleException& e) {
		resetToMark();
		return parseTernaryExpression();
	}
}

Expression* Parser::parseLambdaExpression() {
	List<Modifier*> modifiers;
	parseModifiers(LAMBDA_MODIFIERS, LAMBDA_MODIFIERS_LEN, modifiers);
	List<Parameter*> parameters;
	parseLambdaParameters(parameters);
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::EQUALS_ARROW);
	auto result =
		new LambdaExpression(meta, modifiers, parameters, {}, currentScope);
	currentScope = result;
	parseLambdaBody(result->content);
	popScope();
	return result;
}

void Parser::parseLambdaParameters(List<Parameter*>& dest) {
	if (lh(0)->type == TokenType::LPAREN) {
		advanceAndDelete();
		if (lh(0)->type != TokenType::RPAREN) {
			parseParameters(dest);
		}
		matchAndDelete(TokenType::RPAREN);
	} else {
		dest.push_back(parseParameter());
	}
}

void Parser::parseLambdaBody(List<Node*>& dest) {
	skipNewlines();
	if (lh(0)->type == TokenType::LBRACE) {
		advanceAndDelete();
		parseFunctionBlockContent(dest);
		matchAndDelete(TokenType::RBRACE);
	} else {
		auto meta = lh(0)->meta;
		dest.push_back(new ReturnStatement(meta, parseExpression()));
	}
}

Expression* Parser::parseTernaryExpression() {
	auto arg0 = parseLogicalOrExpression();
	if (lh(0)->type == TokenType::QUESTION_MARK) {
		auto meta = lh(0)->meta;
		advanceAndDelete();
		auto arg1 = parseExpression();
		matchAndDelete(TokenType::COLON);
		auto arg2 = parseExpression();
		return new TernaryExpression(meta, arg0, arg1, arg2);
	}
	return arg0;
}

Expression* Parser::parseLogicalOrExpression() {
	auto left = parseLogicalAndExpression();
	while (lh(0)->type == TokenType::DOUBLE_PIPE ||
		   lh(0)->type == TokenType::OR) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseLogicalAndExpression());
	}
	return left;
}

Expression* Parser::parseLogicalAndExpression() {
	auto left = parseBitwiseOrExpression();
	while (lh(0)->type == TokenType::DOUBLE_AMPERSAND ||
		   lh(0)->type == TokenType::AND) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseBitwiseOrExpression());
	}
	return left;
}

Expression* Parser::parseBitwiseOrExpression() {
	auto left = parseBitwiseXorExpression();
	while (lh(0)->type == TokenType::PIPE) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseBitwiseXorExpression());
	}
	return left;
}

Expression* Parser::parseBitwiseXorExpression() {
	auto left = parseBitwiseAndExpression();
	while (lh(0)->type == TokenType::CARET) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseBitwiseAndExpression());
	}
	return left;
}

Expression* Parser::parseBitwiseAndExpression() {
	auto left = parseEqualityExpression();
	while (lh(0)->type == TokenType::AMPERSAND) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parseEqualityExpression());
	}
	return left;
}

Expression* Parser::parseEqualityExpression() {
	auto left = parseRelationalExpression();
	while (isEqualityOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseRelationalExpression());
	}
	return left;
}

Expression* Parser::parseRelationalExpression() {
	auto left = parseNilCoalescingExpression();
	while (isRelationalOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseNilCoalescingExpression());
	}
	return left;
}

Expression* Parser::parseNilCoalescingExpression() {
	auto left = parseCastingExpression();
	while (lh(0)->type == TokenType::DOUBLE_QUESTION_MARK) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parseCastingExpression());
	}
	return left;
}

Expression* Parser::parseCastingExpression() {
	auto left = parseRangeExpression();
	while (isCastingOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new CastingExpression(op->meta, op, left, parseTypeRef());
	}
	return left;
}

Expression* Parser::parseRangeExpression() {
	auto left = parseBitshiftExpression();
	while (isRangeOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parseBitshiftExpression());
	}
	return left;
}

Expression* Parser::parseBitshiftExpression() {
	auto left = parseAdditiveExpression();
	while (isBitshiftOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parseAdditiveExpression());
	}
	return left;
}

Expression* Parser::parseAdditiveExpression() {
	auto left = parseMultiplicativeExpression();
	while (isAdditiveOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseMultiplicativeExpression());
	}
	return left;
}

Expression* Parser::parseMultiplicativeExpression() {
	auto left = parseExponentialExpression();
	while (isMultiplicativeOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		left = new BinaryExpression(op->meta, op, left,
									parseExponentialExpression());
	}
	return left;
}

Expression* Parser::parseExponentialExpression() {
	auto left = parsePrefixExpression();
	while (lh(0)->type == TokenType::DOUBLE_ASTERISK) {
		auto op = lh(0);
		advance();
		left =
			new BinaryExpression(op->meta, op, left, parsePrefixExpression());
	}
	return left;
}

Expression* Parser::parsePrefixExpression() {
	List<Token*> operators;
	while (isPrefixOperator(lh(0)->type)) {
		operators.push_back(lh(0));
		advance();
	}
	auto arg = parsePostfixExpression();
	while (operators.size() > 0) {
		auto op = operators.back();
		operators.pop_back();
		arg = new UnaryPrefixExpression(op->meta, op, arg);
	}
	return arg;
}

Expression* Parser::parsePostfixExpression() {
	auto arg = parseAccessCallExpression();
	while (isPostfixOperator(lh(0)->type)) {
		auto op = lh(0);
		advance();
		arg = new UnaryPostfixExpression(op->meta, op, arg);
	}
	return arg;
}

Expression* Parser::parseAccessCallExpression() {
	auto left = parsePrimaryExpression();
	while (isAccessOperator(lh(0)->type) || isCallOperator(lh(0)->type)) {
		if (isAccessOperator(lh(0)->type)) {
			auto op = lh(0);
			advance();
			left = new BinaryExpression(op->meta, op, left,
										parsePrimaryExpression());
		} else
			left = parseCallExpressionEnd(left);
	}
	return left;
}

Expression* Parser::parseCallExpressionEnd(Expression* caller) {
	if (lh(0)->type == TokenType::LPAREN) {
		auto meta = lh(0)->meta;
		advanceAndDelete();
		skipNewlines();
		List<Expression*> args;
		if (lh(0)->type != TokenType::RPAREN) parseExpressionList(args);
		matchAndDelete(TokenType::RPAREN);
		return new FunctionCallExpression(meta, caller, args);
	}

	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::LBRACKET);
	auto arg = parseExpression();
	matchAndDelete(TokenType::RBRACKET);
	return new SubscriptExpression(meta, caller, arg);
}

Expression* Parser::parsePrimaryExpression() {
	auto token = lh(0);

	if (isLiteral(token->type)) {
		advance();
		return new LiteralExpression(token);
	}
	if (token->type == TokenType::GLOBAL || token->type == TokenType::ID)
		return parseIdentifierExpression();
	if (token->type == TokenType::LBRACKET)
		return parseArrayOrMapLiteralExpression();

	auto meta = token->meta;
	matchAndDelete(TokenType::LPAREN);
	auto expr = parseExpression();
	if (lh(0)->type == TokenType::COMMA) {
		List<Expression*> args;
		args.push_back(expr);
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			args.push_back(parseExpression());
		}
		matchAndDelete(TokenType::RPAREN);
		return new TupleLiteralExpression(meta, args);
	}
	matchAndDelete(TokenType::RPAREN);
	return expr;
}

Expression* Parser::parseIdentifierExpression() {
	bool globalPrefix = false;
	if (lh(0)->type == TokenType::GLOBAL) {
		advanceAndDelete();
		matchAndDelete(TokenType::DOT);
		globalPrefix = true;
	}
	auto id = match(TokenType::ID);
	List<TypeRef*> generics;
	if (lh(0)->type == TokenType::LT) {
		mark();
		try {
			parseGenericImpl(generics);
			popMark();
		} catch (AcceleException& e) {
			resetToMark();
		}
	}
	return new IdentifierExpression(id, generics, globalPrefix);
}

Expression* Parser::parseArrayOrMapLiteralExpression() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::LBRACKET);
	skipNewlines();
	List<Expression*> args;
	List<Expression*> values;
	bool mapLiteral = false;

	if (lh(0)->type != TokenType::RBRACKET) {
		args.push_back(parseExpression());
		skipNewlines();

		if (lh(0)->type == TokenType::COLON) {
			advanceAndDelete();
			values.push_back(parseExpression());
			mapLiteral = true;
			skipNewlines();
		}
	}

	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		skipNewlines();
		args.push_back(parseExpression());
		skipNewlines();

		if (mapLiteral) {
			matchAndDelete(TokenType::COLON);
			skipNewlines();
			values.push_back(parseExpression());
			skipNewlines();
		}
	}

	matchAndDelete(TokenType::RBRACKET);

	if (mapLiteral) return new MapLiteralExpression(meta, args, values);

	return new ArrayLiteralExpression(meta, args);
}

void Parser::parseExpressionList(List<Expression*>& dest) {
	dest.push_back(parseExpression());
	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		dest.push_back(parseExpression());
	}
}

#ifndef __GNUC__
#pragma endregion
#endif

WarningMetaDeclaration* Parser::parseGlobalWarningMeta() {
	Token* t = lh(0);
	advance();
	matchAndDelete(TokenType::LPAREN);
	List<Token*> args;
	args.push_back(match(TokenType::STRING_LITERAL));
	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		args.push_back(match(TokenType::STRING_LITERAL));
	}
	matchAndDelete(TokenType::RPAREN);
	skipNewlines();
	return new WarningMetaDeclaration(t, args, parseGlobalContent());
}

Variable* Parser::parseNonClassVariable(const TokenType* modifiersArray,
										int modifiersLen) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::VAR);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	Expression* value = nullptr;
	if (lh(0)->type == TokenType::EQUALS) {
		advanceAndDelete();
		value = parseExpression();
	}
	parseNewlineEquiv();

	auto result = new Variable(modifiers, id, declaredType, value, false);
	currentScope->addSymbol(result);
	return result;
}

Variable* Parser::parseNonClassConstant(const TokenType* modifiersArray,
										int modifiersLen) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::CONST);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	matchAndDelete(TokenType::EQUALS);
	Expression* value = parseExpression();
	parseNewlineEquiv();

	auto result = new Variable(modifiers, id, declaredType, value, true);
	currentScope->addSymbol(result);
	return result;
}

Alias* Parser::parseAlias(const TokenType* modifiersArray, int modifiersLen) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::ALIAS);

	auto id = match(TokenType::ID);

	List<GenericType*> generics;
	if (lh(0)->type == TokenType::LT) {
		parseGenerics(generics);
	}

	matchAndDelete(TokenType::EQUALS);
	TypeRef* value = parseTypeRef();
	parseNewlineEquiv();

	auto result = new Alias(modifiers, id, generics, value, currentScope);
	currentScope->addSymbol(result);
	return result;
}

Class* Parser::parseClass(const TokenType* modifiersArray, int modifiersLen) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::CLASS);

	auto id = match(TokenType::ID);

	List<GenericType*> generics;
	if (lh(0)->type == TokenType::LT) {
		parseGenerics(generics);
	}

	skipNewlines();

	List<TypeRef*> declaredParentTypes;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		skipNewlines();
		declaredParentTypes.push_back(parseTypeRef());
		skipNewlines();
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			skipNewlines();
			declaredParentTypes.push_back(parseTypeRef());
			skipNewlines();
		}
	}

	matchAndDelete(TokenType::LBRACE);

	auto result = new Class(modifiers, id, generics, declaredParentTypes, {},
							currentScope);
	currentScope->addSymbol(result);
	currentScope = result;
	parseClassContent(result->content);
	popScope();
	matchAndDelete(TokenType::RBRACE);

	return result;
}

Struct* Parser::parseStruct(const TokenType* modifiersArray, int modifiersLen) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::STRUCT);

	auto id = match(TokenType::ID);

	List<GenericType*> generics;
	if (lh(0)->type == TokenType::LT) {
		parseGenerics(generics);
	}

	skipNewlines();

	List<TypeRef*> declaredParentTypes;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		skipNewlines();
		declaredParentTypes.push_back(parseTypeRef());
		skipNewlines();
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			skipNewlines();
			declaredParentTypes.push_back(parseTypeRef());
			skipNewlines();
		}
	}

	matchAndDelete(TokenType::LBRACE);

	auto result = new Struct(modifiers, id, generics, declaredParentTypes, {},
							 currentScope);
	currentScope->addSymbol(result);
	currentScope = result;
	parseClassContent(result->content);
	popScope();
	matchAndDelete(TokenType::RBRACE);

	return result;
}

Template* Parser::parseTemplate(const TokenType* modifiersArray,
								int modifiersLen) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::TEMPLATE);

	auto id = match(TokenType::ID);

	List<GenericType*> generics;
	if (lh(0)->type == TokenType::LT) {
		parseGenerics(generics);
	}

	skipNewlines();

	List<TypeRef*> declaredParentTypes;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		skipNewlines();
		declaredParentTypes.push_back(parseTypeRef());
		skipNewlines();
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			skipNewlines();
			declaredParentTypes.push_back(parseTypeRef());
			skipNewlines();
		}
	}

	matchAndDelete(TokenType::LBRACE);

	auto result = new Template(modifiers, id, generics, declaredParentTypes, {},
							   currentScope);
	currentScope->addSymbol(result);
	currentScope = result;
	parseTemplateContent(result->content);
	popScope();
	matchAndDelete(TokenType::RBRACE);

	return result;
}

Enum* Parser::parseEnum(const TokenType* modifiersArray, int modifiersLen) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::ENUM);

	auto id = match(TokenType::ID);

	List<GenericType*> generics;
	if (lh(0)->type == TokenType::LT) {
		parseGenerics(generics);
	}

	skipNewlines();

	List<TypeRef*> declaredParentTypes;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		skipNewlines();
		declaredParentTypes.push_back(parseTypeRef());
		skipNewlines();
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			skipNewlines();
			declaredParentTypes.push_back(parseTypeRef());
			skipNewlines();
		}
	}

	matchAndDelete(TokenType::LBRACE);

	auto result = new Enum(modifiers, id, generics, declaredParentTypes, {},
						   currentScope);
	currentScope->addSymbol(result);
	currentScope = result;
	parseEnumContent(result->content);
	popScope();
	matchAndDelete(TokenType::RBRACE);

	return result;
}

Namespace* Parser::parseNamespace(const TokenType* modifiersArray,
								  int modifiersLen) {
	List<Modifier*> modifiers;
	parseModifiers(modifiersArray, modifiersLen, modifiers);
	matchAndDelete(TokenType::NAMESPACE);

	auto id = match(TokenType::ID);

	List<GenericType*> generics;
	if (lh(0)->type == TokenType::LT) {
		parseGenerics(generics);
	}

	skipNewlines();

	matchAndDelete(TokenType::LBRACE);

	auto result = new Namespace(modifiers, id, generics, {}, currentScope);
	currentScope->addSymbol(result);
	currentScope = result;
	parseNamespaceContent(result->content);
	popScope();
	matchAndDelete(TokenType::RBRACE);

	return result;
}

void Parser::parseClassContent(List<Node*>& dest) {
	lexer.setRecoverySentinels({'}', '\r', '\n', ';'});
	skipNewlines(true);
	while (lh(0)->type != TokenType::RBRACE) {
		try {
			auto t = lh(0);
			int current = 0;
			while (isModifier(t->type) || t->type == TokenType::NL) {
				if (t->type == TokenType::META_ENABLEWARNING ||
					t->type == TokenType::META_DISABLEWARNING) {
					// 3 for keyword, lparen, and initial string literal
					current += 3;

					// 2 for comma and next string literal
					while (lh(current)->type == TokenType::COMMA) current += 2;

					t = lh(++current);
				} else
					t = lh(++current);
			}

			if (t->type == TokenType::VAR) {
				dest.push_back(parseClassVariable());
			} else if (t->type == TokenType::CONST) {
				dest.push_back(parseClassConstant());
			} else if (t->type == TokenType::ALIAS) {
				dest.push_back(parseAlias(CLASS_ALIAS_MODIFIERS,
										  CLASS_ALIAS_MODIFIERS_LEN));
			} else if (t->type == TokenType::CLASS) {
				dest.push_back(parseClass(CLASS_CLASS_MODIFIERS,
										  CLASS_CLASS_MODIFIERS_LEN));
			} else if (t->type == TokenType::STRUCT) {
				dest.push_back(parseStruct(CLASS_STRUCT_MODIFIERS,
										   CLASS_STRUCT_MODIFIERS_LEN));
			} else if (t->type == TokenType::TEMPLATE) {
				dest.push_back(parseTemplate(CLASS_TEMPLATE_MODIFIERS,
											 CLASS_TEMPLATE_MODIFIERS_LEN));
			} else if (t->type == TokenType::ENUM) {
				dest.push_back(
					parseEnum(CLASS_ENUM_MODIFIERS, CLASS_ENUM_MODIFIERS_LEN));
			} else if (t->type == TokenType::NAMESPACE) {
				dest.push_back(parseNamespace(CLASS_NAMESPACE_MODIFIERS,
											  CLASS_NAMESPACE_MODIFIERS_LEN));
			} else if (t->type == TokenType::FUN) {
				dest.push_back(parseFunction(CLASS_FUNCTION_MODIFIERS,
											 CLASS_FUNCTION_MODIFIERS_LEN,
											 true));
			} else if (t->type == TokenType::CONSTRUCT) {
				dest.push_back(parseConstructor());
			} else if (t->type == TokenType::DESTRUCT) {
				dest.push_back(parseDestructor());
			} else {
				if (!isSpeculating())
					diagnoser.diagnoseInvalidTokenWithMessage(
						"Invalid class content", t);
				panic();
			}
		} catch (DuplicateSymbolException& e) {
			if (!isSpeculating())
				diagnoser.diagnoseDuplicateSymbol(e.original, e.duplicate);
			panic();
		} catch (DuplicateImportException& e) {
			if (!isSpeculating())
				diagnoser.diagnoseDuplicateImport(e.original, e.duplicate);
		} catch (ParserPanicException& e) {
			panicking = false;
		} catch (AcceleException& e) {
			if (!isSpeculating()) {
				if (e.sourceMeta)
					diagnoser.diagnose(e.ec, *e.sourceMeta, e.highlightLength,
									   e.message);
				else
					diagnoser.diagnose(e.ec, e.message);
			}
			panic();
		}
		skipNewlines(true);
	}
}

void Parser::parseTemplateContent(List<Node*>& dest) {
	lexer.setRecoverySentinels({'}', '\r', '\n', ';'});
	skipNewlines(true);
	while (lh(0)->type != TokenType::RBRACE) {
		try {
			auto t = lh(0);
			int current = 0;
			while (isModifier(t->type) || t->type == TokenType::NL) {
				if (t->type == TokenType::META_ENABLEWARNING ||
					t->type == TokenType::META_DISABLEWARNING) {
					// 3 for keyword, lparen, and initial string literal
					current += 3;

					// 2 for comma and next string literal
					while (lh(current)->type == TokenType::COMMA) current += 2;

					t = lh(++current);
				} else
					t = lh(++current);
			}

			if (t->type == TokenType::VAR) {
				dest.push_back(parseTemplateVariable());
			} else if (t->type == TokenType::CONST) {
				dest.push_back(parseTemplateConstant());
			} else if (t->type == TokenType::ALIAS) {
				dest.push_back(parseAlias(CLASS_ALIAS_MODIFIERS,
										  CLASS_ALIAS_MODIFIERS_LEN));
			} else if (t->type == TokenType::CLASS) {
				dest.push_back(parseClass(CLASS_CLASS_MODIFIERS,
										  CLASS_CLASS_MODIFIERS_LEN));
			} else if (t->type == TokenType::STRUCT) {
				dest.push_back(parseStruct(CLASS_STRUCT_MODIFIERS,
										   CLASS_STRUCT_MODIFIERS_LEN));
			} else if (t->type == TokenType::TEMPLATE) {
				dest.push_back(parseTemplate(CLASS_TEMPLATE_MODIFIERS,
											 CLASS_TEMPLATE_MODIFIERS_LEN));
			} else if (t->type == TokenType::ENUM) {
				dest.push_back(
					parseEnum(CLASS_ENUM_MODIFIERS, CLASS_ENUM_MODIFIERS_LEN));
			} else if (t->type == TokenType::NAMESPACE) {
				dest.push_back(parseNamespace(CLASS_NAMESPACE_MODIFIERS,
											  CLASS_NAMESPACE_MODIFIERS_LEN));
			} else if (t->type == TokenType::FUN) {
				dest.push_back(parseFunction(TEMPLATE_FUNCTION_MODIFIERS,
											 TEMPLATE_FUNCTION_MODIFIERS_LEN,
											 true));
			} else {
				if (!isSpeculating())
					diagnoser.diagnoseInvalidTokenWithMessage(
						"Invalid template content", t);
				panic();
			}
		} catch (DuplicateSymbolException& e) {
			if (!isSpeculating())
				diagnoser.diagnoseDuplicateSymbol(e.original, e.duplicate);
			panic();
		} catch (DuplicateImportException& e) {
			if (!isSpeculating())
				diagnoser.diagnoseDuplicateImport(e.original, e.duplicate);
		} catch (ParserPanicException& e) {
			panicking = false;
		} catch (AcceleException& e) {
			if (!isSpeculating()) {
				if (e.sourceMeta)
					diagnoser.diagnose(e.ec, *e.sourceMeta, e.highlightLength,
									   e.message);
				else
					diagnoser.diagnose(e.ec, e.message);
			}
			panic();
		}
		skipNewlines(true);
	}
}

void Parser::parseEnumContent(List<Node*>& dest) {
	lexer.setRecoverySentinels({'}', '\r', '\n', ';'});
	skipNewlines(true);
	while (lh(0)->type != TokenType::RBRACE) {
		try {
			auto t = lh(0);
			int current = 0;
			while (isModifier(t->type) || t->type == TokenType::NL) {
				if (t->type == TokenType::META_ENABLEWARNING ||
					t->type == TokenType::META_DISABLEWARNING) {
					// 3 for keyword, lparen, and initial string literal
					current += 3;

					// 2 for comma and next string literal
					while (lh(current)->type == TokenType::COMMA) current += 2;

					t = lh(++current);
				} else
					t = lh(++current);
			}

			if (t->type == TokenType::VAR) {
				dest.push_back(parseClassVariable());
			} else if (t->type == TokenType::CONST) {
				dest.push_back(parseClassConstant());
			} else if (t->type == TokenType::ALIAS) {
				dest.push_back(parseAlias(NAMESPACE_ALIAS_MODIFIERS,
										  NAMESPACE_ALIAS_MODIFIERS_LEN));
			} else if (t->type == TokenType::CLASS) {
				dest.push_back(parseClass(NAMESPACE_CLASS_MODIFIERS,
										  NAMESPACE_CLASS_MODIFIERS_LEN));
			} else if (t->type == TokenType::STRUCT) {
				dest.push_back(parseStruct(NAMESPACE_STRUCT_MODIFIERS,
										   NAMESPACE_STRUCT_MODIFIERS_LEN));
			} else if (t->type == TokenType::TEMPLATE) {
				dest.push_back(parseTemplate(NAMESPACE_TEMPLATE_MODIFIERS,
											 NAMESPACE_TEMPLATE_MODIFIERS_LEN));
			} else if (t->type == TokenType::ENUM) {
				dest.push_back(parseEnum(NAMESPACE_ENUM_MODIFIERS,
										 NAMESPACE_ENUM_MODIFIERS_LEN));
			} else if (t->type == TokenType::NAMESPACE) {
				dest.push_back(
					parseNamespace(NAMESPACE_NAMESPACE_MODIFIERS,
								   NAMESPACE_NAMESPACE_MODIFIERS_LEN));
			} else if (t->type == TokenType::FUN) {
				dest.push_back(parseFunction(ENUM_FUNCTION_MODIFIERS,
											 ENUM_FUNCTION_MODIFIERS_LEN,
											 true));
			} else if (t->type == TokenType::CONSTRUCT) {
				dest.push_back(parseConstructor());
			} else if (t->type == TokenType::DESTRUCT) {
				dest.push_back(parseDestructor());
			} else if (t->type == TokenType::CASE) {
				dest.push_back(parseEnumCase());
			} else {
				if (!isSpeculating())
					diagnoser.diagnoseInvalidTokenWithMessage(
						"Invalid enum content", t);
				panic();
			}
		} catch (DuplicateSymbolException& e) {
			if (!isSpeculating())
				diagnoser.diagnoseDuplicateSymbol(e.original, e.duplicate);
			panic();
		} catch (DuplicateImportException& e) {
			if (!isSpeculating())
				diagnoser.diagnoseDuplicateImport(e.original, e.duplicate);
		} catch (ParserPanicException& e) {
			panicking = false;
		} catch (AcceleException& e) {
			if (!isSpeculating()) {
				if (e.sourceMeta)
					diagnoser.diagnose(e.ec, *e.sourceMeta, e.highlightLength,
									   e.message);
				else
					diagnoser.diagnose(e.ec, e.message);
			}
			panic();
		}
		skipNewlines(true);
	}
}

void Parser::parseNamespaceContent(List<Node*>& dest) {
	lexer.setRecoverySentinels({'}', '\r', '\n', ';'});
	skipNewlines(true);
	while (lh(0)->type != TokenType::RBRACE) {
		try {
			auto t = lh(0);
			int current = 0;
			while (isModifier(t->type) || t->type == TokenType::NL) {
				if (t->type == TokenType::META_ENABLEWARNING ||
					t->type == TokenType::META_DISABLEWARNING) {
					// 3 for keyword, lparen, and initial string literal
					current += 3;

					// 2 for comma and next string literal
					while (lh(current)->type == TokenType::COMMA) current += 2;

					t = lh(++current);
				} else
					t = lh(++current);
			}

			if (t->type == TokenType::VAR) {
				dest.push_back(
					parseNonClassVariable(NAMESPACE_VARIABLE_MODIFIERS,
										  NAMESPACE_VARIABLE_MODIFIERS_LEN));
			} else if (t->type == TokenType::CONST) {
				dest.push_back(
					parseNonClassConstant(NAMESPACE_VARIABLE_MODIFIERS,
										  NAMESPACE_VARIABLE_MODIFIERS_LEN));
			} else if (t->type == TokenType::ALIAS) {
				dest.push_back(parseAlias(NAMESPACE_ALIAS_MODIFIERS,
										  NAMESPACE_ALIAS_MODIFIERS_LEN));
			} else if (t->type == TokenType::CLASS) {
				dest.push_back(parseClass(NAMESPACE_CLASS_MODIFIERS,
										  NAMESPACE_CLASS_MODIFIERS_LEN));
			} else if (t->type == TokenType::STRUCT) {
				dest.push_back(parseStruct(NAMESPACE_STRUCT_MODIFIERS,
										   NAMESPACE_STRUCT_MODIFIERS_LEN));
			} else if (t->type == TokenType::TEMPLATE) {
				dest.push_back(parseTemplate(NAMESPACE_TEMPLATE_MODIFIERS,
											 NAMESPACE_TEMPLATE_MODIFIERS_LEN));
			} else if (t->type == TokenType::ENUM) {
				dest.push_back(parseEnum(NAMESPACE_TEMPLATE_MODIFIERS,
										 NAMESPACE_ENUM_MODIFIERS_LEN));
			} else if (t->type == TokenType::NAMESPACE) {
				dest.push_back(
					parseNamespace(NAMESPACE_NAMESPACE_MODIFIERS,
								   NAMESPACE_NAMESPACE_MODIFIERS_LEN));
			} else if (t->type == TokenType::FUN) {
				dest.push_back(parseFunction(NAMESPACE_FUNCTION_MODIFIERS,
											 NAMESPACE_FUNCTION_MODIFIERS_LEN,
											 false));
			} else {
				if (!isSpeculating())
					diagnoser.diagnoseInvalidTokenWithMessage(
						"Invalid namespace content", t);
				panic();
			}
		} catch (DuplicateSymbolException& e) {
			if (!isSpeculating())
				diagnoser.diagnoseDuplicateSymbol(e.original, e.duplicate);
			panic();
		} catch (DuplicateImportException& e) {
			if (!isSpeculating())
				diagnoser.diagnoseDuplicateImport(e.original, e.duplicate);
		} catch (ParserPanicException& e) {
			panicking = false;
		} catch (AcceleException& e) {
			if (!isSpeculating()) {
				if (e.sourceMeta)
					diagnoser.diagnose(e.ec, *e.sourceMeta, e.highlightLength,
									   e.message);
				else
					diagnoser.diagnose(e.ec, e.message);
			}
			panic();
		}
		skipNewlines(true);
	}
}

Variable* Parser::parseClassVariable() {
	List<Modifier*> modifiers;
	parseModifiers(CLASS_VARIABLE_MODIFIERS, CLASS_VARIABLE_MODIFIERS_LEN,
				   modifiers);
	matchAndDelete(TokenType::VAR);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	Node* value = nullptr;
	if (lh(0)->type == TokenType::EQUALS) {
		advanceAndDelete();
		value = parseExpression();
		parseNewlineEquiv();
	} else if (lh(0)->type == TokenType::LBRACE) {
		auto meta = lh(0)->meta;
		advanceAndDelete();
		skipNewlines();
		value = parseVariableBlock(meta);
		skipNewlines();
		matchAndDelete(TokenType::RBRACE);
	} else
		parseNewlineEquiv();

	auto result = new Variable(modifiers, id, declaredType, value, false);
	currentScope->addSymbol(result);
	return result;
}

Variable* Parser::parseClassConstant() {
	List<Modifier*> modifiers;
	parseModifiers(CLASS_VARIABLE_MODIFIERS, CLASS_VARIABLE_MODIFIERS_LEN,
				   modifiers);
	matchAndDelete(TokenType::CONST);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	Node* value = nullptr;
	if (lh(0)->type == TokenType::EQUALS) {
		advanceAndDelete();
		value = parseExpression();
		parseNewlineEquiv();
	} else if (lh(0)->type == TokenType::LBRACE) {
		auto meta = lh(0)->meta;
		advanceAndDelete();
		skipNewlines();
		value = parseVariableBlock(meta);
		skipNewlines();
		matchAndDelete(TokenType::RBRACE);
	} else
		parseNewlineEquiv();

	auto result = new Variable(modifiers, id, declaredType, value, true);
	currentScope->addSymbol(result);
	return result;
}

VariableBlock* Parser::parseVariableBlock(const SourceMeta& meta) {
	FunctionBlock* getBlock = nullptr;
	SetBlock* setBlock = nullptr;
	FunctionBlock* initBlock = nullptr;

	skipNewlines(true);
	while (lh(0)->type != TokenType::RBRACE) {
		auto t = lh(0);
		int current = 0;
		while (isModifier(t->type) || t->type == TokenType::NL) {
			if (t->type == TokenType::META_ENABLEWARNING ||
				t->type == TokenType::META_DISABLEWARNING) {
				// 3 for keyword, lparen, and initial string literal
				current += 3;

				// 2 for comma and next string literal
				while (lh(current)->type == TokenType::COMMA) current += 2;

				t = lh(++current);
			} else
				t = lh(++current);
		}

		if (t->type == TokenType::GET && getBlock) {
			if (!isSpeculating())
				diagnoser.diagnose(ec::DUPLICATE_VARIABLE_BLOCK, t->meta,
								   t->data.length(), "Duplicate get block");
			panic();
		} else if (t->type == TokenType::GET)
			getBlock = parseGetBlock();
		else if (t->type == TokenType::SET && setBlock) {
			if (!isSpeculating())
				diagnoser.diagnose(ec::DUPLICATE_VARIABLE_BLOCK, t->meta,
								   t->data.length(), "Duplicate set block");
			panic();
		} else if (t->type == TokenType::SET)
			setBlock = parseSetBlock();
		else if (t->type == TokenType::INIT && initBlock) {
			if (!isSpeculating())
				diagnoser.diagnose(ec::DUPLICATE_VARIABLE_BLOCK, t->meta,
								   t->data.length(), "Duplicate init block");
			panic();
		} else
			initBlock = parseInitBlock();

		skipNewlines(true);
	}

	return new VariableBlock(meta, getBlock, setBlock, initBlock);
}

FunctionBlock* Parser::parseGetBlock() {
	List<Modifier*> modifiers;
	parseModifiers(GET_BLOCK_MODIFIERS, GET_BLOCK_MODIFIERS_LEN, modifiers);
	skipNewlines();
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::GET);
	skipNewlines();
	FunctionBlock* block =
		new FunctionBlock(meta, modifiers, {}, currentScope, TokenType::GET);

	if (lh(0)->type == TokenType::LBRACE) {
		currentScope = block;
		matchAndDelete(TokenType::LBRACE);
		parseFunctionBlockContent(block->content);
		matchAndDelete(TokenType::RBRACE);
		popScope();
	}

	return block;
}

SetBlock* Parser::parseSetBlock() {
	List<Modifier*> modifiers;
	parseModifiers(SET_BLOCK_MODIFIERS, SET_BLOCK_MODIFIERS_LEN, modifiers);
	skipNewlines();
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::SET);
	skipNewlines();

	Parameter* param = nullptr;

	if (lh(0)->type == TokenType::LPAREN) {
		matchAndDelete(TokenType::LPAREN);
		skipNewlines();
		param = parseParameter();
		skipNewlines();
		matchAndDelete(TokenType::RPAREN);
		skipNewlines();
	}

	SetBlock* block = new SetBlock(meta, modifiers, param, {}, currentScope);

	if (param) {
		currentScope = block;
		matchAndDelete(TokenType::LBRACE);
		parseFunctionBlockContent(block->content);
		matchAndDelete(TokenType::RBRACE);
		popScope();
	}

	return block;
}

FunctionBlock* Parser::parseInitBlock() {
	List<Modifier*> modifiers;
	parseModifiers(INIT_BLOCK_MODIFIERS, INIT_BLOCK_MODIFIERS_LEN, modifiers);
	skipNewlines();
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::INIT);
	skipNewlines();
	FunctionBlock* block =
		new FunctionBlock(meta, modifiers, {}, currentScope, TokenType::INIT);

	currentScope = block;
	matchAndDelete(TokenType::LBRACE);
	parseFunctionBlockContent(block->content);
	matchAndDelete(TokenType::RBRACE);
	popScope();

	return block;
}

Constructor* Parser::parseConstructor() {
	List<Modifier*> modifiers;
	parseModifiers(CONSTRUCTOR_MODIFIERS, CONSTRUCTOR_MODIFIERS_LEN, modifiers);
	skipNewlines();
	auto id = match(TokenType::CONSTRUCT);
	skipNewlines();
	matchAndDelete(TokenType::LPAREN);
	List<Parameter*> parameters;
	parseParameters(parameters);
	matchAndDelete(TokenType::RPAREN);

	skipNewlines();

	Constructor* constructor =
		new Constructor(modifiers, id, parameters, {}, currentScope);
	currentScope->addSymbol(constructor);
	currentScope = constructor;

	matchAndDelete(TokenType::LBRACE);
	parseFunctionBlockContent(constructor->content);
	matchAndDelete(TokenType::RBRACE);

	popScope();

	return constructor;
}

Destructor* Parser::parseDestructor() {
	List<Modifier*> modifiers;
	parseModifiers(DESTRUCTOR_MODIFIERS, DESTRUCTOR_MODIFIERS_LEN, modifiers);
	skipNewlines();
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::DESTRUCT);
	skipNewlines();

	Destructor* destructor = new Destructor(meta, modifiers, {}, currentScope);
	currentScope = destructor;

	matchAndDelete(TokenType::LBRACE);
	parseFunctionBlockContent(destructor->content);
	matchAndDelete(TokenType::RBRACE);

	popScope();

	return destructor;
}

Variable* Parser::parseTemplateVariable() {
	List<Modifier*> modifiers;
	parseModifiers(CLASS_VARIABLE_MODIFIERS, CLASS_VARIABLE_MODIFIERS_LEN,
				   modifiers);

	bool hasStaticMod = false;
	for (const auto& mod : modifiers) {
		if (mod->content->type == TokenType::STATIC) {
			hasStaticMod = true;
			break;
		}
	}

	if (!hasStaticMod) {
		if (!isSpeculating())
			diagnoser.diagnose(ec::NONSTATIC_TEMPLATE_VARIABLE, lh(0)->meta,
							   lh(0)->data.length());
		panic();
	}

	matchAndDelete(TokenType::VAR);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	Expression* value = nullptr;
	if (lh(0)->type == TokenType::EQUALS) {
		advanceAndDelete();
		value = parseExpression();
	}
	parseNewlineEquiv();

	auto result = new Variable(modifiers, id, declaredType, value, false);
	currentScope->addSymbol(result);
	return result;
}

Variable* Parser::parseTemplateConstant() {
	List<Modifier*> modifiers;
	parseModifiers(CLASS_VARIABLE_MODIFIERS, CLASS_VARIABLE_MODIFIERS_LEN,
				   modifiers);

	bool hasStaticMod = false;
	for (const auto& mod : modifiers) {
		if (mod->content->type == TokenType::STATIC) {
			hasStaticMod = true;
			break;
		}
	}

	if (!hasStaticMod) {
		if (!isSpeculating())
			diagnoser.diagnose(ec::NONSTATIC_TEMPLATE_VARIABLE, lh(0)->meta,
							   lh(0)->data.length());
		panic();
	}

	matchAndDelete(TokenType::CONST);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	matchAndDelete(TokenType::EQUALS);
	Expression* value = parseExpression();
	parseNewlineEquiv();

	auto result = new Variable(modifiers, id, declaredType, value, true);
	currentScope->addSymbol(result);
	return result;
}

EnumCase* Parser::parseEnumCase() {
	List<Modifier*> modifiers;
	parseModifiers(ENUM_CASE_MODIFIERS, ENUM_CASE_MODIFIERS_LEN, modifiers);
	matchAndDelete(TokenType::CASE);

	auto id = match(TokenType::ID);

	skipNewlines();

	List<Expression*> args;
	if (lh(0)->type == TokenType::LPAREN) {
		advanceAndDelete();
		skipNewlines();
		if (lh(0)->type != TokenType::RPAREN) {
			parseExpressionList(args);
			skipNewlines();
		}
		matchAndDelete(TokenType::RPAREN);
	}

	parseNewlineEquiv();

	auto result =
		new EnumCase(modifiers, id, args, dynamic_cast<Enum*>(currentScope));
	currentScope->addSymbol(result);
	return result;
}

Import* Parser::parseImport() {
	matchAndDelete(TokenType::IMPORT);
	skipNewlines();
	auto t = lh(0);
	if (t->type == TokenType::LBRACE) {
		return parseFromImport();
	} else if (t->type == TokenType::STRING_LITERAL) {
		return parseStandardImport();
	}

	int current = 1;
	while (lh(current)->type == TokenType::NL) current++;

	t = lh(current);
	if (t->type == TokenType::COLON || t->type == TokenType::FROM) {
		return parseFromImport();
	}

	return parseStandardImport();
}

Import* Parser::parseStandardImport() {
	int numSkipped = 0;
	auto source = parseImportSource(numSkipped);
	if (numSkipped == 0) numSkipped = skipNewlines();
	Token* alias = nullptr;
	bool foundAs = false;
	if (lh(0)->type == TokenType::AS) {
		advanceAndDelete();
		alias = match(TokenType::ID);
		foundAs = true;
	}
	if (numSkipped == 0 && !foundAs) parseNewlineEquiv();
	return new Import(source, alias, {});
}

Import* Parser::parseFromImport() {
	List<ImportTarget*> targets;
	if (lh(0)->type == TokenType::LBRACE) {
		advanceAndDelete();
		skipNewlines();
		targets.push_back(parseImportTarget());
		skipNewlines();
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			skipNewlines();
			targets.push_back(parseImportTarget());
			skipNewlines();
		}
		matchAndDelete(TokenType::RBRACE);
		skipNewlines();
	} else {
		targets.push_back(parseImportTarget());
		skipNewlines();
	}

	matchAndDelete(TokenType::FROM);
	skipNewlines();

	int numSkipped = 0;
	auto source = parseImportSource(numSkipped);

	if (numSkipped == 0) parseNewlineEquiv();

	return new Import(source, nullptr, targets);
}

ImportSource* Parser::parseImportSource(int& numNewlinesSkipped) {
	auto t = lh(0);
	if (t->type == TokenType::STRING_LITERAL) {
		advance();
		return new ImportSource(t, nullptr, false);
	}

	ImportSource* result = nullptr;

	bool relative = false;

	while (t->type == TokenType::DOT || t->type == TokenType::DOUBLE_DOT ||
		   t->type == TokenType::TRIPLE_DOT) {
		if (t->type == TokenType::DOUBLE_DOT ||
			t->type == TokenType::TRIPLE_DOT) {
			t = relex();
		}

		relative = true;

		result = new ImportSource(t, result, relative);
		t = lh(0);
	}

	if (result && result->parent) {
		auto tmp = result->parent;
		result->parent = nullptr;
		delete result;
		result = tmp;
	} else if (result) {
		delete result;
		result = nullptr;
	}

	t = match(TokenType::ID);

	result = new ImportSource(t, result, relative);

	numNewlinesSkipped = skipNewlines();
	while (lh(0)->type == TokenType::DOT) {
		advance();
		skipNewlines();
		auto child = match(TokenType::ID);
		result = new ImportSource(child, result, relative);
		numNewlinesSkipped = skipNewlines();
	}

	return result;
}

ImportTarget* Parser::parseImportTarget() {
	auto id = match(TokenType::ID);
	skipNewlines();

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		skipNewlines();
		declaredType = parseTypeRef();
	}

	return new ImportTarget(id, declaredType);
}

MetaDeclaration* Parser::parseSourceLock(const List<Node*>& globalContent) {
	auto t = match(TokenType::META_SRCLOCK);
	if (!globalContent.empty() && ctx.warnings[ec::NONFRONTED_SOURCE_LOCK]) {
		if (!isSpeculating()) diagnoser.diagnoseSourceLock(t);
	}
	return new MetaDeclaration(t);
}

void Parser::parseParameters(List<Parameter*>& dest) {
	skipNewlines();
	if (lh(0)->type != TokenType::RPAREN) {
		dest.push_back(parseParameter());
		skipNewlines();
		while (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			skipNewlines();
			dest.push_back(parseParameter());
			skipNewlines();
		}
	}
}

Parameter* Parser::parseParameter() {
	List<Modifier*> modifiers;
	parseModifiers(PARAMETER_MODIFIERS, PARAMETER_MODIFIERS_LEN, modifiers);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	auto result = new Parameter(modifiers, id, declaredType);
	// Do not add symbol here; that is taken care of in the constructor of
	// whatever accepts the parameter
	return result;
}

void Parser::parseGenerics(List<GenericType*>& dest) {
	skipNewlines();
	matchAndDelete(TokenType::LT);
	skipNewlines();
	dest.push_back(parseGenericType());
	skipNewlines();
	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		skipNewlines();
		dest.push_back(parseGenericType());
		skipNewlines();
	}
	matchAndDelete(TokenType::GT);
}

GenericType* Parser::parseGenericType() {
	auto id = match(TokenType::ID);
	skipNewlines();
	TypeRef* declaredParentType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		skipNewlines();
		declaredParentType = parseTypeRef();
	}
	return new GenericType(id, declaredParentType);
}

void Parser::parseGenericImpl(List<TypeRef*>& dest) {
	auto t = lh(0);
	if (t->type != TokenType::LT) relex();
	matchAndDelete(TokenType::LT);
	skipNewlines();
	dest.push_back(parseTypeRef());
	skipNewlines();
	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		skipNewlines();
		dest.push_back(parseTypeRef());
		skipNewlines();
	}
	if (lh(0)->type != TokenType::GT) relex();
	matchAndDelete(TokenType::GT);
}

#ifndef __GNUC__
#pragma region FunctionBlockContent
#endif

FunctionBlock* Parser::parseFunctionBlock() {
	skipNewlines(true);
	List<Modifier*> modifiers;
	parseModifiers(FUNCTION_BLOCK_MODIFIERS, FUNCTION_BLOCK_MODIFIERS_LEN,
				   modifiers);
	skipNewlines();
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::LBRACE);
	FunctionBlock* result =
		new FunctionBlock(meta, modifiers, {}, currentScope, TokenType::LBRACE);
	currentScope = result;
	parseFunctionBlockContent(result->content);
	matchAndDelete(TokenType::RBRACE);
	popScope();
	return result;
}

void Parser::parseFunctionBlockContent(List<Node*>& dest) {
	lexer.setRecoverySentinels({'}', '\r', '\n', ';'});
	skipNewlines(true);
	while (lh(0)->type != TokenType::RBRACE) {
		try {
			dest.push_back(parseSingleFunctionBlockContent());
		} catch (ParserPanicException& e) {
			panicking = false;
		}
		skipNewlines(true);
	}
}

Node* Parser::parseSingleFunctionBlockContent() {
	panicTerminator = PanicTerminator::STATEMENT_END;
	try {
		auto t = lh(0);
		if (t->type == TokenType::IF) {
			return parseIfBlock();
		} else if (t->type == TokenType::WHILE) {
			return parseWhileBlock();
		} else if (t->type == TokenType::REPEAT) {
			return parseRepeatBlock();
		} else if (t->type == TokenType::FOR) {
			return parseForBlock();
		} else if (t->type == TokenType::SWITCH) {
			return parseSwitchBlock();
		} else if (t->type == TokenType::TRY) {
			return parseTryBlock();
		} else if (t->type == TokenType::LBRACE) {
			return parseFunctionBlock();
		} else if (t->type == TokenType::BREAK ||
				   t->type == TokenType::CONTINUE ||
				   t->type == TokenType::FALL) {
			advance();
			parseNewlineEquiv();
			return new SingleTokenStatement(t);
		} else if (t->type == TokenType::VAR) {
			return parseLocalVariable();
		} else if (t->type == TokenType::CONST) {
			return parseLocalConstant();
		} else if (t->type == TokenType::ALIAS) {
			return parseAlias(LOCAL_ALIAS_MODIFIERS, LOCAL_ALIAS_MODIFIERS_LEN);
		} else if (isModifier(t->type)) {
			// lookahead to var or const keyword
			int current = 0;
			while (isModifier(t->type) || t->type == TokenType::NL) {
				if (t->type == TokenType::META_ENABLEWARNING ||
					t->type == TokenType::META_DISABLEWARNING) {
					// 3 for keyword, lparen, and initial string literal
					current += 3;

					// 2 for comma and next string literal
					while (lh(current)->type == TokenType::COMMA) current += 2;

					t = lh(++current);
				} else
					t = lh(++current);
			}
			if (t->type == TokenType::CONST)
				return parseLocalConstant();
			else if (t->type == TokenType::VAR)
				return parseLocalVariable();
			else if (t->type == TokenType::ALIAS)
				return parseAlias(LOCAL_ALIAS_MODIFIERS,
								  LOCAL_ALIAS_MODIFIERS_LEN);
			return parseFunctionBlock();
		} else if (t->type == TokenType::THROW) {
			return parseThrowStatement();
		} else if (t->type == TokenType::RETURN) {
			return parseReturnStatement();
		} else if (t->type == TokenType::META_ENABLEWARNING ||
				   t->type == TokenType::META_DISABLEWARNING) {
			return parseLocalWarningMeta();
		}
		auto result = parseExpression();
		parseNewlineEquiv();
		return result;
	} catch (DuplicateSymbolException& e) {
		if (!isSpeculating())
			diagnoser.diagnoseDuplicateSymbol(e.original, e.duplicate);
		panic();
	} catch (DuplicateImportException& e) {
		if (!isSpeculating())
			diagnoser.diagnoseDuplicateImport(e.original, e.duplicate);
	} catch (ParserPanicException& e) {
		panicTerminator = PanicTerminator::STATEMENT_END;
		panic();
	} catch (AcceleException& e) {
		if (!isSpeculating()) {
			if (e.sourceMeta)
				diagnoser.diagnose(e.ec, *e.sourceMeta, e.highlightLength,
								   e.message);
			else
				diagnoser.diagnose(e.ec, e.message);
		}
		panic();
	}

	panic();
}

IfBlock* Parser::parseIfBlock() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::IF);
	auto condition = parseExpression();
	skipNewlines();
	FunctionBlock* block;
	if (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		skipNewlines();
		auto blockMeta = lh(0)->meta;
		block = new FunctionBlock(blockMeta, {}, {}, currentScope,
								  TokenType::LBRACE);
		currentScope = block;
		block->content.push_back(parseSingleFunctionBlockContent());
		popScope();
	} else {
		block = parseFunctionBlock();
	}

	skipNewlines();
	List<ConditionalBlock*> elifBlocks;
	while (lh(0)->type == TokenType::ELIF) {
		auto elifMeta = lh(0)->meta;
		advanceAndDelete();
		auto elifCondition = parseExpression();
		skipNewlines();
		FunctionBlock* elifBlock;
		if (lh(0)->type == TokenType::COMMA) {
			advanceAndDelete();
			skipNewlines();
			auto blockMeta = lh(0)->meta;
			elifBlock = new FunctionBlock(blockMeta, {}, {}, currentScope,
										  TokenType::LBRACE);
			currentScope = elifBlock;
			elifBlock->content.push_back(parseSingleFunctionBlockContent());
			popScope();
		} else {
			elifBlock = parseFunctionBlock();
		}

		skipNewlines();

		elifBlocks.push_back(
			new ConditionalBlock(elifMeta, elifCondition, elifBlock));
	}

	FunctionBlock* elseBlock = nullptr;
	if (lh(0)->type == TokenType::ELSE) {
		auto elseMeta = lh(0)->meta;
		advanceAndDelete();
		skipNewlines();
		elseBlock = new FunctionBlock(elseMeta, {}, {}, currentScope,
									  TokenType::LBRACE);
		currentScope = elseBlock;
		elseBlock->content.push_back(parseSingleFunctionBlockContent());
		popScope();
	}

	return new IfBlock(meta, condition, block, elifBlocks, elseBlock);
}

WhileBlock* Parser::parseWhileBlock() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::WHILE);
	auto condition = parseExpression();
	skipNewlines();
	FunctionBlock* block;
	if (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		skipNewlines();
		auto blockMeta = lh(0)->meta;
		block = new FunctionBlock(blockMeta, {}, {}, currentScope,
								  TokenType::LBRACE);
		currentScope = block;
		block->content.push_back(parseSingleFunctionBlockContent());
		popScope();
	} else {
		block = parseFunctionBlock();
	}

	return new WhileBlock(meta, condition, block);
}

RepeatBlock* Parser::parseRepeatBlock() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::REPEAT);
	skipNewlines();
	FunctionBlock* block =
		new FunctionBlock(lh(0)->meta, {}, {}, currentScope, TokenType::LBRACE);
	currentScope = block;
	block->content.push_back(parseSingleFunctionBlockContent());
	popScope();
	skipNewlines(true);
	matchAndDelete(TokenType::WHILE);
	auto condition = parseExpression();
	parseNewlineEquiv();
	return new RepeatBlock(meta, condition, block);
}

ForBlock* Parser::parseForBlock() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::FOR);
	skipNewlines();
	auto iterator = parseParameter();
	skipNewlines();
	matchAndDelete(TokenType::IN);
	skipNewlines();
	auto iteratee = parseExpression();
	skipNewlines();
	FunctionBlock* block;
	if (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		skipNewlines();
		auto blockMeta = lh(0)->meta;
		block = new FunctionBlock(blockMeta, {}, {}, currentScope,
								  TokenType::LBRACE);
		currentScope = block;
		block->content.push_back(parseSingleFunctionBlockContent());
		popScope();
	} else {
		block = parseFunctionBlock();
	}

	return new ForBlock(meta, iterator, iteratee, block);
}

SwitchBlock* Parser::parseSwitchBlock() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::SWITCH);
	skipNewlines();
	auto condition = parseExpression();
	List<SwitchCaseBlock*> cases;
	skipNewlines();
	matchAndDelete(TokenType::LBRACE);
	parseSwitchBlockCases(cases);
	matchAndDelete(TokenType::RBRACE);
	return new SwitchBlock(meta, condition, cases);
}

void Parser::parseSwitchBlockCases(List<SwitchCaseBlock*>& dest) {
	skipNewlines(true);
	bool foundDefault = false;
	while (lh(0)->type != TokenType::RBRACE) {
		if (lh(0)->type == TokenType::CASE) {
			auto t = match(TokenType::CASE);
			skipNewlines();
			auto condition = parseExpression();
			skipNewlines();
			matchAndDelete(TokenType::COLON);
			skipNewlines();
			FunctionBlock* block = new FunctionBlock(
				t->meta, {}, {}, currentScope, TokenType::LBRACE);
			currentScope = block;
			parseFunctionBlockContent(block->content);
			popScope();
			dest.push_back(new SwitchCaseBlock(t->meta, t, condition, block));
		} else if (lh(0)->type == TokenType::DEFAULT && foundDefault) {
			if (!isSpeculating())
				diagnoser.diagnose(ec::DUPLICATE_DEFAULT_CASE, lh(0)->meta,
								   lh(0)->data.length());
			panic();
		} else {
			auto t = match(TokenType::DEFAULT);
			skipNewlines();
			matchAndDelete(TokenType::COLON);
			skipNewlines();
			FunctionBlock* block = new FunctionBlock(
				t->meta, {}, {}, currentScope, TokenType::LBRACE);
			currentScope = block;
			parseFunctionBlockContent(block->content);
			popScope();
			dest.push_back(new SwitchCaseBlock(t->meta, t, nullptr, block));
			foundDefault = true;
		}
		skipNewlines(true);
	}
}

TryBlock* Parser::parseTryBlock() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::TRY);
	skipNewlines();
	FunctionBlock* block =
		new FunctionBlock(lh(0)->meta, {}, {}, currentScope, TokenType::LBRACE);
	currentScope = block;
	block->content.push_back(parseSingleFunctionBlockContent());
	popScope();
	skipNewlines(true);
	List<CatchBlock*> catchBlocks;
	while (lh(0)->type == TokenType::CATCH) {
		catchBlocks.push_back(parseCatchBlock());
		skipNewlines();
	}
	return new TryBlock(meta, block, catchBlocks);
}

CatchBlock* Parser::parseCatchBlock() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::CATCH);
	skipNewlines();
	auto exceptionVariable = parseParameter();
	skipNewlines();
	auto block = parseFunctionBlock();
	return new CatchBlock(meta, exceptionVariable, block);
}

Variable* Parser::parseLocalVariable() {
	List<Modifier*> modifiers;
	parseModifiers(LOCAL_VARIABLE_MODIFIERS, LOCAL_VARIABLE_MODIFIERS_LEN,
				   modifiers);
	matchAndDelete(TokenType::VAR);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	Expression* value = nullptr;
	if (lh(0)->type == TokenType::EQUALS) {
		advanceAndDelete();
		value = parseExpression();
	}
	parseNewlineEquiv();

	auto result = new Variable(modifiers, id, declaredType, value, false);
	currentScope->addSymbol(result);
	return result;
}

Variable* Parser::parseLocalConstant() {
	List<Modifier*> modifiers;
	parseModifiers(LOCAL_VARIABLE_MODIFIERS, LOCAL_VARIABLE_MODIFIERS_LEN,
				   modifiers);
	matchAndDelete(TokenType::CONST);

	auto id = match(TokenType::ID);

	TypeRef* declaredType = nullptr;
	if (lh(0)->type == TokenType::COLON) {
		advanceAndDelete();
		declaredType = parseTypeRef();
	}

	matchAndDelete(TokenType::EQUALS);
	Expression* value = parseExpression();
	parseNewlineEquiv();

	auto result = new Variable(modifiers, id, declaredType, value, true);
	currentScope->addSymbol(result);
	return result;
}

ThrowStatement* Parser::parseThrowStatement() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::THROW);
	skipNewlines();
	auto value = parseExpression();
	parseNewlineEquiv();
	return new ThrowStatement(meta, value);
}

ReturnStatement* Parser::parseReturnStatement() {
	auto meta = lh(0)->meta;
	matchAndDelete(TokenType::RETURN);
	Expression* value = nullptr;
	if (!isNewlineEquivalent(lh(0)->type) && lh(0)->type != TokenType::NL &&
		lh(0)->type != TokenType::SEMICOLON)
		value = parseExpression();
	parseNewlineEquiv();
	return new ReturnStatement(meta, value);
}

WarningMetaDeclaration* Parser::parseLocalWarningMeta() {
	Token* t = lh(0);
	advance();
	matchAndDelete(TokenType::LPAREN);
	List<Token*> args;
	args.push_back(match(TokenType::STRING_LITERAL));
	while (lh(0)->type == TokenType::COMMA) {
		advanceAndDelete();
		args.push_back(match(TokenType::STRING_LITERAL));
	}
	matchAndDelete(TokenType::RPAREN);
	skipNewlines();
	return new WarningMetaDeclaration(t, args,
									  parseSingleFunctionBlockContent());
}

#ifndef __GNUC__
#pragma endregion
#endif

#ifndef __GNUC__
#pragma region GlobalFunctions
#endif

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
		   type == TokenType::META_LAXTHROW ||
		   type == TokenType::META_NORETURN ||
		   type == TokenType::META_ENABLEWARNING ||
		   type == TokenType::META_DISABLEWARNING;
}

bool isNewlineEquivalent(TokenType type) {
	return type == TokenType::RBRACE || type == TokenType::RBRACKET ||
		   type == TokenType::RPAREN || type == TokenType::COMMA ||
		   type == TokenType::EOF_TOKEN;
}

bool isTypeSuffixStart(TokenType type) {
	return type == TokenType::LBRACKET || type == TokenType::TRIPLE_DOT ||
		   type == TokenType::MINUS_ARROW || type == TokenType::QUESTION_MARK ||
		   type == TokenType::DOUBLE_QUESTION_MARK ||
		   type == TokenType::QUESTION_MARK_DOT ||
		   type == TokenType::EXCLAMATION_POINT ||
		   type == TokenType::ASTERISK || type == TokenType::DOUBLE_ASTERISK ||
		   type == TokenType::ASTERISK_EQUALS ||
		   type == TokenType::DOUBLE_ASTERISK_EQUALS;
}

bool isGenericsStart(TokenType type) {
	return type == TokenType::LT || type == TokenType::DOUBLE_LT ||
		   type == TokenType::DOUBLE_LT_EQUALS || type == TokenType::LT_EQUALS;
}

bool isAssignmentOperator(TokenType type) {
	return type == TokenType::EQUALS || type == TokenType::PIPE_EQUALS ||
		   type == TokenType::PLUS_EQUALS || type == TokenType::CARET_EQUALS ||
		   type == TokenType::MINUS_EQUALS || type == TokenType::SLASH_EQUALS ||
		   type == TokenType::TILDE_EQUALS ||
		   type == TokenType::PERCENT_EQUALS ||
		   type == TokenType::ASTERISK_EQUALS ||
		   type == TokenType::AMPERSAND_EQUALS ||
		   type == TokenType::DOUBLE_GT_EQUALS ||
		   type == TokenType::DOUBLE_LT_EQUALS ||
		   type == TokenType::DOUBLE_ASTERISK_EQUALS;
}

bool isEqualityOperator(TokenType type) {
	return type == TokenType::DOUBLE_EQUALS ||
		   type == TokenType::TRIPLE_EQUALS ||
		   type == TokenType::EXCLAMATION_POINT_EQUALS ||
		   type == TokenType::EXCLAMATION_POINT_DOUBLE_EQUALS;
}

bool isRelationalOperator(TokenType type) {
	return type == TokenType::LT || type == TokenType::GT ||
		   type == TokenType::LT_EQUALS || type == TokenType::GT_EQUALS ||
		   type == TokenType::COMPARE;
}

bool isCastingOperator(TokenType type) {
	return type == TokenType::AS || type == TokenType::AS_OPTIONAL ||
		   type == TokenType::AS_UNWRAPPED || type == TokenType::IS;
}

bool isRangeOperator(TokenType type) {
	return type == TokenType::DOUBLE_DOT || type == TokenType::TRIPLE_DOT;
}

bool isBitshiftOperator(TokenType type) {
	return type == TokenType::DOUBLE_LT || type == TokenType::DOUBLE_GT;
}

bool isAdditiveOperator(TokenType type) {
	return type == TokenType::PLUS || type == TokenType::MINUS;
}

bool isMultiplicativeOperator(TokenType type) {
	return type == TokenType::ASTERISK || type == TokenType::SLASH ||
		   type == TokenType::PERCENT;
}

bool isPrefixOperator(TokenType type) {
	return type == TokenType::PLUS || type == TokenType::MINUS ||
		   type == TokenType::DOUBLE_PLUS || type == TokenType::DOUBLE_MINUS ||
		   type == TokenType::TILDE || type == TokenType::EXCLAMATION_POINT ||
		   type == TokenType::ASTERISK || type == TokenType::AMPERSAND ||
		   type == TokenType::RELEASE || type == TokenType::TRY_OPTIONAL ||
		   type == TokenType::TRY_UNWRAPPED || type == TokenType::AWAIT ||
		   type == TokenType::NOT;
}

bool isPostfixOperator(TokenType type) {
	return type == TokenType::DOUBLE_PLUS || type == TokenType::DOUBLE_MINUS ||
		   type == TokenType::EXCLAMATION_POINT;
}

bool isAccessOperator(TokenType type) {
	return type == TokenType::DOT || type == TokenType::QUESTION_MARK_DOT;
}

bool isCallOperator(TokenType type) {
	return type == TokenType::LPAREN || type == TokenType::LBRACKET;
}

bool isLiteral(TokenType type) {
	return type == TokenType::HEX_LITERAL || type == TokenType::NIL_LITERAL ||
		   type == TokenType::FLOAT_LITERAL ||
		   type == TokenType::OCTAL_LITERAL ||
		   type == TokenType::BINARY_LITERAL ||
		   type == TokenType::STRING_LITERAL ||
		   type == TokenType::BOOLEAN_LITERAL ||
		   type == TokenType::INTEGER_LITERAL || type == TokenType::SELF ||
		   type == TokenType::SUPER;
}

bool isFunctionOperator(TokenType type) {
	return type == TokenType::TILDE || type == TokenType::EXCLAMATION_POINT ||
		   type == TokenType::PERCENT || type == TokenType::CARET ||
		   type == TokenType::AMPERSAND || type == TokenType::ASTERISK ||
		   type == TokenType::MINUS || type == TokenType::PLUS ||
		   type == TokenType::PIPE || type == TokenType::LT ||
		   type == TokenType::GT || type == TokenType::SLASH ||
		   type == TokenType::DOUBLE_EQUALS ||
		   type == TokenType::EXCLAMATION_POINT_EQUALS ||
		   type == TokenType::NOT || type == TokenType::AS ||
		   type == TokenType::DOUBLE_ASTERISK ||
		   type == TokenType::DOUBLE_MINUS || type == TokenType::DOUBLE_PLUS ||
		   type == TokenType::DOUBLE_LT || type == TokenType::DOUBLE_GT ||
		   type == TokenType::DOUBLE_DOT || type == TokenType::TRIPLE_DOT ||
		   type == TokenType::COMPARE;
}

bool isLocalVariableModifier(TokenType type) {
	return type == TokenType::ATOMIC || type == TokenType::REF ||
		   type == TokenType::GREEDY || type == TokenType::STRONG ||
		   type == TokenType::WEAK || type == TokenType::META_ENABLEWARNING ||
		   type == TokenType::META_DISABLEWARNING;
}

#ifndef __GNUC__
#pragma endregion
#endif
}  // namespace acl