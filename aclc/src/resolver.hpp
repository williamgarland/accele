#pragma once

#include <deque>

#include "ast.hpp"

namespace acl {
class Resolver {
	Ast* ast;
	Scope* currentScope;
	std::deque<Scope*> scopeStack;

	void pushScope(Scope* newScope);
	void popScope();

   public:
	Resolver(Ast* ast);
	void resolve();
	void resolveGlobalScope(GlobalScope* scope);
	void resolveNonLocalContent(Node* content);
	void resolveLocalContent(Node* content);
	void resolveImport(Import* i);
	void resolveAlias(Alias* alias);
	void resolveGenericType(GenericType* type);
	void resolveClass(Class* c);
	void resolveStruct(Struct* s);
	void resolveTemplate(Template* t);
	void resolveEnum(Enum* e);
	void resolveNamespace(Namespace* n);
	void resolveFunctionScope(LambdaExpression* scope);
	void resolveFunctionScope(FunctionBlock* scope);
	void resolveFunctionScope(Function* scope);
	void resolveFunctionScope(SetBlock* scope);
	void resolveFunctionScope(Constructor* scope);
	void resolveFunctionScope(Destructor* scope);
	void resolveParameter(Parameter* parameter);
	void resolveVariable(Variable* variable);
	void resolveTypeRef(TypeRef* typeRef);
	void resolveSimpleTypeRef(SimpleTypeRef* typeRef);

	void resolveExpression(Expression* expression,
						   int symbolSearchFlags = ResolveFlag::RECURSIVE |
												   ResolveFlag::LEXICAL |
												   ResolveFlag::TYPE_HIERARCHY);
	void resolveBinaryExpression(BinaryExpression* expression,
								 int symbolSearchFlags);
	void resolvePrefixExpression(UnaryPrefixExpression* expression);
	void resolvePostfixExpression(UnaryPostfixExpression* expression);
	void resolveTernaryExpression(TernaryExpression* expression);
	void resolveFunctionCallExpression(FunctionCallExpression* expression);
	void resolveSubscriptExpression(SubscriptExpression* expression);
	void resolveCastingExpression(CastingExpression* expression);
	void resolveMapLiteralExpression(MapLiteralExpression* expression);
	void resolveArrayLiteralExpression(ArrayLiteralExpression* expression);
	void resolveTupleLiteralExpression(TupleLiteralExpression* expression);
	void resolveLiteralExpression(LiteralExpression* expression);
	void resolveIdentifierExpression(IdentifierExpression* expression,
									 int symbolSearchFlags);
	void resolveLambdaExpression(LambdaExpression* expression);

	void resolveExpressionExpectSymbols(
		IdentifierExpression** dest, Expression* expression,
		int symbolSearchFlags = ResolveFlag::RECURSIVE | ResolveFlag::LEXICAL |
								ResolveFlag::TYPE_HIERARCHY);
	void resolveBinaryExpressionExpectSymbols(IdentifierExpression** dest,
											  BinaryExpression* expression,
											  int symbolSearchFlags);
	void resolveIdentifierExpressionExpectSymbols(
		IdentifierExpression** dest, IdentifierExpression* expression,
		int symbolSearchFlags);

	Symbol* resolveFunctionCallSymbols(FunctionCallExpression* expression,
									   const List<TypeRef*>& argTypes,
									   const List<Symbol*>& symbols);
};

Scope* getScopeForIdentifierExpression(IdentifierExpression* expression);
Scope* getScopeForTypeRef(TypeRef* t);
int getFunctionTypeScoreForArgs(FunctionTypeRef* t,
								const List<TypeRef*>& argTypes);
bool isVariadicType(TypeRef* t);

// WARNING: This will heap-allocate a new FunctionTypeRef!
FunctionTypeRef* getFunctionTypeForFunction(Function* f);
}  // namespace acl