#pragma once

#include "common.hpp"
#include "lexer.hpp"

namespace acl {
struct Symbol;
struct Type;

struct Scope {
	Scope* parentScope;
	List<Symbol*> symbols;
	List<Type*> types;

	Scope(Scope* parentScope);
	virtual ~Scope();
	virtual void addSymbol(Symbol* symbol);
	virtual void addType(Type* type);
	virtual Symbol* resolveSymbol(const Token* id, Type* targetType,
								  const List<Type*>& generics);
	virtual Type* resolveType(const Token* id, const List<Type*>& generics);
};

struct Node {
	SourceMeta sourceMeta;
	Node(const SourceMeta& sourceMeta);
	virtual ~Node();
	virtual void toJson(StringBuffer& dest) const = 0;
};

struct Modifier : public Node {
	Token* content;
	Modifier(Token* content);
	virtual ~Modifier();
	virtual void toJson(StringBuffer& dest) const override;
};

struct MetaDeclaration : public Modifier {
	MetaDeclaration(Token* content);
	virtual ~MetaDeclaration();
	virtual void toJson(StringBuffer& dest) const override;
};

struct WarningMetaDeclaration : public MetaDeclaration {
	List<Token*> args;
	Node* target;
	WarningMetaDeclaration(Token* content, const List<Token*>& args,
						   Node* target);
	virtual ~WarningMetaDeclaration();
	virtual void toJson(StringBuffer& dest) const override;
};

struct GlobalScope : public Node, public Scope {
	List<Node*> content;
	GlobalScope(const SourceMeta& sourceMeta, const List<Node*>& content);
	virtual ~GlobalScope();
	virtual void toJson(StringBuffer& dest) const override;
};

struct TypeRef : public Node {
	TypeRef(const SourceMeta& sourceMeta);
	virtual ~TypeRef();
};

struct SimpleTypeRef : public TypeRef {
	SimpleTypeRef* parent;
	Token* id;
	List<TypeRef*> generics;
	SimpleTypeRef(const SourceMeta& sourceMeta, Token* id,
				  const List<TypeRef*>& generics, SimpleTypeRef* parent);
	virtual ~SimpleTypeRef();
	virtual void toJson(StringBuffer& dest) const override;
};

struct SuffixTypeRef : public TypeRef {
	TypeRef* type;
	Token* suffixSymbol;
	SuffixTypeRef(const SourceMeta& sourceMeta, TypeRef* type,
				  Token* suffixSymbol);
	virtual ~SuffixTypeRef();
	virtual void toJson(StringBuffer& dest) const override;
};

struct TupleTypeRef : public TypeRef {
	List<TypeRef*> elementTypes;

	// This is only used when the parser needs to convert a tuple type ref to a
	// list of type refs in the parameters of a function type
	bool deleteElementTypes;
	TupleTypeRef(const SourceMeta& sourceMeta,
				 const List<TypeRef*>& elementTypes);
	virtual ~TupleTypeRef();
	virtual void toJson(StringBuffer& dest) const override;
};

struct MapTypeRef : public TypeRef {
	TypeRef* keyType;
	TypeRef* valueType;
	MapTypeRef(const SourceMeta& sourceMeta, TypeRef* keyType,
			   TypeRef* valueType);
	virtual ~MapTypeRef();
	virtual void toJson(StringBuffer& dest) const override;
};

struct ArrayTypeRef : public TypeRef {
	TypeRef* elementType;
	ArrayTypeRef(const SourceMeta& sourceMeta, TypeRef* elementType);
	virtual ~ArrayTypeRef();
	virtual void toJson(StringBuffer& dest) const override;
};

struct FunctionTypeRef : public TypeRef {
	List<TypeRef*> paramTypes;
	TypeRef* returnType;
	FunctionTypeRef(const SourceMeta& sourceMeta,
					const List<TypeRef*>& paramTypes, TypeRef* returnType);
	virtual ~FunctionTypeRef();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Expression : public Node {
	Expression(const SourceMeta& sourceMeta);
	virtual ~Expression();
};

struct TernaryExpression : public Expression {
	Expression* arg0;
	Expression* arg1;
	Expression* arg2;
	TernaryExpression(const SourceMeta& sourceMeta, Expression* arg0,
					  Expression* arg1, Expression* arg2);
	virtual ~TernaryExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct BinaryExpression : public Expression {
	Token* op;
	Expression* left;
	Expression* right;
	BinaryExpression(const SourceMeta& sourceMeta, Token* op, Expression* left,
					 Expression* right);
	virtual ~BinaryExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct UnaryPrefixExpression : public Expression {
	Token* op;
	Expression* arg;
	UnaryPrefixExpression(const SourceMeta& sourceMeta, Token* op,
						  Expression* arg);
	virtual ~UnaryPrefixExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct UnaryPostfixExpression : public Expression {
	Token* op;
	Expression* arg;
	UnaryPostfixExpression(const SourceMeta& sourceMeta, Token* op,
						   Expression* arg);
	virtual ~UnaryPostfixExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct FunctionCallExpression : public Expression {
	Expression* caller;
	List<Expression*> args;
	FunctionCallExpression(const SourceMeta& sourceMeta, Expression* caller,
						   List<Expression*> args);
	virtual ~FunctionCallExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct SubscriptExpression : public Expression {
	Expression* target;
	Expression* index;
	SubscriptExpression(const SourceMeta& sourceMeta, Expression* target,
						Expression* index);
	virtual ~SubscriptExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct CastingExpression : public Expression {
	Token* op;
	Expression* left;
	TypeRef* right;
	CastingExpression(const SourceMeta& sourceMeta, Token* op, Expression* left,
					  TypeRef* right);
	virtual ~CastingExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct MapLiteralExpression : public Expression {
	List<Expression*> keys;
	List<Expression*> values;
	MapLiteralExpression(const SourceMeta& sourceMeta,
						 const List<Expression*>& keys,
						 const List<Expression*>& values);
	virtual ~MapLiteralExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct ArrayLiteralExpression : public Expression {
	List<Expression*> elements;
	ArrayLiteralExpression(const SourceMeta& sourceMeta,
						   const List<Expression*>& elements);
	virtual ~ArrayLiteralExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct TupleLiteralExpression : public Expression {
	List<Expression*> elements;
	TupleLiteralExpression(const SourceMeta& sourceMeta,
						   const List<Expression*>& elements);
	virtual ~TupleLiteralExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct LiteralExpression : public Expression {
	Token* value;
	LiteralExpression(Token* value);
	virtual ~LiteralExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct IdentifierExpression : public Expression {
	Token* value;
	List<TypeRef*> generics;
	bool globalPrefix;
	IdentifierExpression(Token* value, const List<TypeRef*>& generics,
						 bool globalPrefix);
	virtual ~IdentifierExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Parameter;
struct FunctionBlock;

struct LambdaExpression : public Expression, public Scope {
	List<Modifier*> modifiers;	// For things like "async"
	List<Parameter*> parameters;
	List<Node*> content;
	LambdaExpression(const SourceMeta& sourceMeta,
					 const List<Modifier*>& modifiers,
					 const List<Parameter*>& parameters,
					 const List<Node*>& content, Scope* parentScope);
	virtual ~LambdaExpression();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Symbol : public Node {
	Token* id;
	Symbol(Token* id);
	virtual ~Symbol();
};

struct Parameter : public Symbol {
	List<Modifier*> modifiers;
	TypeRef* declaredType;
	Parameter(const List<Modifier*>& modifiers, Token* id,
			  TypeRef* declaredType);
	virtual ~Parameter();
	virtual void toJson(StringBuffer& dest) const override;
};

struct FunctionBlock : public Node, public Scope {
	List<Modifier*> modifiers;	// For things like "unsafe" blocks
	List<Node*> content;
	FunctionBlock(const SourceMeta& sourceMeta,
				  const List<Modifier*>& modifiers, const List<Node*>& content,
				  Scope* parentScope);
	virtual ~FunctionBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct GenericType;

struct Function : public Symbol, public Scope {
	List<Modifier*> modifiers;
	List<GenericType*> generics;
	List<Parameter*> parameters;
	TypeRef* declaredReturnType;
	List<Node*> content;
	Function(const List<Modifier*>& modifiers, Token* id,
			 const List<GenericType*>& generics,
			 const List<Parameter*>& parameters, TypeRef* declaredReturnType,
			 const List<Node*>& content, Scope* parentScope);
	virtual ~Function();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Variable : public Symbol {
	List<Modifier*> modifiers;
	bool constant;
	TypeRef* declaredType;
	Node* value;  // This is a Node and not an Expression because this could be
				  // a VariableBlock instead of an Expression
	Variable(const List<Modifier*>& modifiers, Token* id, TypeRef* declaredType,
			 Node* value, bool constant);
	virtual ~Variable();
	virtual void toJson(StringBuffer& dest) const override;
};

struct ConditionalBlock : public Node {
	Expression* condition;
	FunctionBlock* block;
	ConditionalBlock(const SourceMeta& sourceMeta, Expression* condition,
					 FunctionBlock* block);
	virtual ~ConditionalBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct IfBlock : public ConditionalBlock {
	List<ConditionalBlock*> elifBlocks;
	FunctionBlock* elseBlock;
	IfBlock(const SourceMeta& sourceMeta, Expression* condition,
			FunctionBlock* block, const List<ConditionalBlock*>& elifBlocks,
			FunctionBlock* elseBlock);
	virtual ~IfBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct WhileBlock : public ConditionalBlock {
	WhileBlock(const SourceMeta& sourceMeta, Expression* condition,
			   FunctionBlock* block);
	virtual ~WhileBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct RepeatBlock : public ConditionalBlock {
	RepeatBlock(const SourceMeta& sourceMeta, Expression* condition,
				FunctionBlock* block);
	virtual ~RepeatBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct CatchBlock : public Node {
	Parameter* exceptionVariable;
	FunctionBlock* block;
	CatchBlock(const SourceMeta& sourceMeta, Parameter* exceptionVariable,
			   FunctionBlock* block);
	virtual ~CatchBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct TryBlock : public Node {
	FunctionBlock* block;
	List<CatchBlock*> catchBlocks;
	TryBlock(const SourceMeta& sourceMeta, FunctionBlock* block,
			 const List<CatchBlock*>& catchBlocks);
	virtual ~TryBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct SwitchCaseBlock : public Node {
	Token* caseType;  // "case" or "default"
	FunctionBlock* block;
	SwitchCaseBlock(const SourceMeta& sourceMeta, Token* caseType,
					FunctionBlock* block);
	virtual ~SwitchCaseBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct SwitchBlock : public Node {
	Expression* condition;
	List<SwitchCaseBlock*> cases;
	SwitchBlock(const SourceMeta& sourceMeta, Expression* condition,
				const List<SwitchCaseBlock*>& cases);
	virtual ~SwitchBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct ReturnStatement : public Node {
	Expression* value;
	ReturnStatement(const SourceMeta& sourceMeta, Expression* value);
	virtual ~ReturnStatement();
	virtual void toJson(StringBuffer& dest) const override;
};

struct ThrowStatement : public Node {
	Expression* value;
	ThrowStatement(const SourceMeta& sourceMeta, Expression* value);
	virtual ~ThrowStatement();
	virtual void toJson(StringBuffer& dest) const override;
};

struct SingleTokenStatement : public Node {
	Token* content;
	SingleTokenStatement(Token* content);
	virtual ~SingleTokenStatement();
	virtual void toJson(StringBuffer& dest) const override;
};

struct GenericType;

struct Type : public Symbol {
	List<GenericType*> generics;
	Type(Token* id, const List<GenericType*>& generics);
	virtual ~Type();
};

struct GenericType : public Type {
	TypeRef* declaredParentType;

	// A generic type cannot itself declare generics,
	// therefore it does not accept the list of generic types
	GenericType(Token* id, TypeRef* declaredParentType);
	virtual ~GenericType();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Alias : public Type {
	List<Modifier*> modifiers;
	TypeRef* value;
	Alias(const List<Modifier*>& modifiers, Token* id,
		  const List<GenericType*>& generics, TypeRef* value);
	virtual ~Alias();
	virtual void toJson(StringBuffer& dest) const override;
};

struct SetBlock : public Node, public Scope {
	List<Modifier*> modifiers;
	Parameter* parameter;
	List<Node*> content;
	SetBlock(const SourceMeta& sourceMeta, const List<Modifier*>& modifiers,
			 Parameter* parameter, const List<Node*>& content,
			 Scope* parentScope);
	virtual ~SetBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct VariableBlock : public Node {
	FunctionBlock* getBlock;
	SetBlock* setBlock;
	FunctionBlock* initBlock;
	VariableBlock(const SourceMeta& sourceMeta, FunctionBlock* getBlock,
				  SetBlock* setBlock, FunctionBlock* initBlock);
	virtual ~VariableBlock();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Class : public Type, public Scope {
	List<Modifier*> modifiers;
	TypeRef* declaredParentType;
	List<TypeRef*> usedTemplates;
	List<Node*> content;
	Class(const List<Modifier*>& modifiers, Token* id,
		  const List<GenericType*>& generics, TypeRef* declaredParentType,
		  const List<TypeRef*>& usedTemplates, const List<Node*>& content,
		  Scope* parentScope);
	virtual ~Class();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Struct : public Type, public Scope {
	List<Modifier*> modifiers;
	TypeRef* declaredParentType;
	List<TypeRef*> usedTemplates;
	List<Node*> content;
	Struct(const List<Modifier*>& modifiers, Token* id,
		   const List<GenericType*>& generics, TypeRef* declaredParentType,
		   const List<TypeRef*>& usedTemplates, const List<Node*>& content,
		   Scope* parentScope);
	virtual ~Struct();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Template : public Type, public Scope {
	List<Modifier*> modifiers;
	List<TypeRef*> declaredParentTypes;
	List<Node*> content;
	Template(const List<Modifier*>& modifiers, Token* id,
			 const List<GenericType*>& generics,
			 const List<TypeRef*>& declaredParentTypes,
			 const List<Node*>& content, Scope* parentScope);
	virtual ~Template();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Enum : public Type, public Scope {
	List<Modifier*> modifiers;
	List<TypeRef*> usedTemplates;
	List<Node*> content;
	Enum(const List<Modifier*>& modifiers, Token* id,
		 const List<GenericType*>& generics,
		 const List<TypeRef*>& usedTemplates, const List<Node*>& content,
		 Scope* parentScope);
	virtual ~Enum();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Namespace : public Symbol, public Scope {
	List<GenericType*> generics;
	List<Modifier*> modifiers;
	List<Node*> content;
	Namespace(const List<Modifier*>& modifiers, Token* id,
			  const List<GenericType*>& generics, const List<Node*>& content,
			  Scope* parentScope);
	virtual ~Namespace();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Constructor : public Symbol, public Scope {
	List<Modifier*> modifiers;
	List<Parameter*> parameters;
	List<Node*> content;
	Constructor(const List<Modifier*>& modifiers, Token* id,
				const List<Parameter*>& parameters, const List<Node*>& content,
				Scope* parentScope);
	virtual ~Constructor();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Destructor : public Node, public Scope {
	List<Modifier*> modifiers;
	List<Node*> content;
	Destructor(const SourceMeta& sourceMeta, List<Modifier*>& modifiers,
			   const List<Node*>& content, Scope* parentScope);
	virtual ~Destructor();
	virtual void toJson(StringBuffer& dest) const override;
};

struct EnumCase : public Symbol {
	List<Modifier*> modifiers;
	List<Expression*> args;
	EnumCase(const List<Modifier*>& modifiers, Token* id,
			 const List<Expression*>& args);
	virtual ~EnumCase();
	virtual void toJson(StringBuffer& dest) const override;
};

struct ImportTarget : public Node {
	Token* id;
	TypeRef* declaredType;
	ImportTarget(Token* id, TypeRef* declaredType);
	virtual ~ImportTarget();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Import : public Node {
	Token* source;
	Token* alias;
	List<ImportTarget*> targets;
	Import(Token* source, Token* alias, const List<ImportTarget*>& targets);
	virtual ~Import();
	virtual void toJson(StringBuffer& dest) const override;
};

struct Ast {
	GlobalScope* globalScope;
	Ast(GlobalScope* globalScope);
	~Ast();
};
}  // namespace acl