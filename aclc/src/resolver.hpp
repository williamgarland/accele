#pragma once

#include <deque>

#include "ast.hpp"
#include "common.hpp"

namespace acl {
enum class SearchTarget { VARIABLE, TYPE, NAMESPACE };

struct SearchCriteria {
	bool recursive;
	bool allowExternal;
	List<SearchTarget> targets;
	bool requireExactMatch;

	// For when you're searching for an lvalue as opposed to an rvalue
	bool modifiable;
};

class Resolver {
	std::deque<Scope*> scopes;
	std::deque<Scope*> lexicalScopes;
	Module* mod;
	CompilerContext& ctx;
	ResolutionStage maxStage;
	List<Symbol*> symbolStack;
	Diagnoser diagnoser;

	void pushSymbol(Symbol* symbol);
	void popSymbol();
	bool stackContainsSymbol(Symbol* symbol);

	void pushScope(Scope* scope, bool isLexicalScope);
	Scope* peekScope();
	Scope* popScope();
	Scope* getLexicalScope();

   public:
	Resolver(CompilerContext& ctx, Module* mod);
	Resolver(CompilerContext& ctx, Module* mod, ResolutionStage maxStage);
	void resolve();

   private:
	// ----- General ----- //
	void resolveGlobalScope();
	void resolveNonLocalContent(Node* n);
	void resolveClass(Class* n);
	void resolveStruct(Struct* n);
	void resolveTemplate(Template* n);
	void resolveEnum(Enum* n);
	void resolveNamespace(Namespace* n);
	void resolveAlias(Alias* n);
	void resolveConstructor(Constructor* n);
	void resolveVariable(Variable* n);
	void resolveEnumCase(EnumCase* n);
	void resolveFunction(Function* n);
	void resolveGenericType(GenericType* n);

	// "intendedType" is for things like for-loops
	void resolveParameter(Parameter* n, TypeRef* intendedType);
	void resolveLocalContent(Node* n, TypeRef** destReturnType);
	void resolveFunctionBlock(FunctionBlock* n, TypeRef** destReturnType);
	void resolveIfBlock(IfBlock* n, TypeRef** destReturnType);
	void resolveWhileBlock(WhileBlock* n, TypeRef** destReturnType);
	void resolveRepeatBlock(RepeatBlock* n, TypeRef** destReturnType);
	void resolveForBlock(ForBlock* n, TypeRef** destReturnType);
	void resolveSwitchBlock(SwitchBlock* n, TypeRef** destReturnType);
	void resolveTryBlock(TryBlock* n, TypeRef** destReturnType);
	void resolveReturnStatement(ReturnStatement* n, TypeRef** destReturnType);
	void resolveThrowStatement(ThrowStatement* n);

	// ----- TypeRef ----- //
	void resolveTypeRef(TypeRef* n);
	void resolveSimpleTypeRef(SimpleTypeRef* n);
	Scope* resolveSimpleTypeRefParent(SimpleTypeRef* n);
	void resolveArrayTypeRef(ArrayTypeRef* n);
	void resolveMapTypeRef(MapTypeRef* n);
	void resolveTupleTypeRef(TupleTypeRef* n);
	void resolveFunctionTypeRef(FunctionTypeRef* n);
	void resolveSuffixTypeRef(SuffixTypeRef* n);

	// ----- Expression ----- //
	void resolveExpression(Expression* n);
	void resolveExpression0(Expression* n, const SearchCriteria* searchCriteria,
							IdentifierExpression** dest);
	void resolveFunctionCallExpression(FunctionCallExpression* n,
									   const SearchCriteria* searchCriteria);
	void resolveTernaryExpression(TernaryExpression* n);
	void resolveBinaryExpression(BinaryExpression* n,
								 const SearchCriteria* searchCriteria,
								 IdentifierExpression** dest);
	void resolveAccessExpression(BinaryExpression* n,
								 const SearchCriteria* searchCriteria,
								 IdentifierExpression** dest);
	void resolvePrefixExpression(UnaryPrefixExpression* n);
	void resolvePostfixExpression(UnaryPostfixExpression* n);
	void resolveSubscriptExpression(SubscriptExpression* n);
	void resolveIdentifierExpression(IdentifierExpression* n,
									 const SearchCriteria* searchCriteria,
									 IdentifierExpression** dest);
	void resolveArrayLiteralExpression(ArrayLiteralExpression* n);
	void resolveMapLiteralExpression(MapLiteralExpression* n);
	void resolveTupleLiteralExpression(TupleLiteralExpression* n);
	void resolveLiteralExpression(LiteralExpression* n);
	void resolveLambdaExpression(LambdaExpression* n);
	void resolveCastingExpression(CastingExpression* n);

   private:
	TypeRef* getSymbolReturnType(Symbol* symbol, const SourceMeta& refererMeta);
	Symbol* getBestCallerForArgs(IdentifierExpression* idexpr,
								 const List<TypeRef*>& args,
								 const SourceMeta& callerMeta,
								 Scope* lexicalScope,
								 const SearchCriteria& searchCriteria,
								 TypeRef** destReturnType);
	void getFunctionCallCandidateTypes(
		const List<resolve::SearchResult>& candidates,
		List<List<std::pair<Symbol*, FunctionTypeRef*>>>& candidateTypes,
		const SourceMeta& callerMeta);
	void getFunctionCallCandidateType(
		Symbol* symbol, List<std::pair<Symbol*, FunctionTypeRef*>>& refs,
		const SourceMeta& callerMeta);
	void getFcctForType(Type* type,
						List<std::pair<Symbol*, FunctionTypeRef*>>& refs,
						const SourceMeta& callerMeta);
};
}  // namespace acl