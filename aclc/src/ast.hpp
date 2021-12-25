#pragma once

#include "common.hpp"
#include "lexer.hpp"

namespace acl {
struct Node {
	SourceMeta sourceMeta;
	Node(const SourceMeta& sourceMeta);
	virtual ~Node();
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
};

struct SuffixTypeRef : public TypeRef {
	TypeRef* type;
	Token* suffixSymbol;
	SuffixTypeRef(const SourceMeta& sourceMeta, TypeRef* type,
				  Token* suffixSymbol);
	virtual ~SuffixTypeRef();
};

struct TupleTypeRef : public TypeRef {
	List<TypeRef*> elementTypes;
	TupleTypeRef(const SourceMeta& sourceMeta,
				 const List<TypeRef*>& elementTypes);
	virtual ~TupleTypeRef();
};

struct MapTypeRef : public TypeRef {
	TypeRef* keyType;
	TypeRef* valueType;
	MapTypeRef(const SourceMeta& sourceMeta, TypeRef* keyType,
			   TypeRef* valueType);
	virtual ~MapTypeRef();
};

struct ArrayTypeRef : public TypeRef {
	TypeRef* elementType;
	ArrayTypeRef(const SourceMeta& sourceMeta, TypeRef* elementType);
	virtual ~ArrayTypeRef();
};

struct FunctionTypeRef : public TypeRef {
	List<TypeRef*> paramTypes;
	TypeRef* returnType;
	FunctionTypeRef(const SourceMeta& sourceMeta,
					const List<TypeRef*>& paramTypes, TypeRef* returnType);
	virtual ~FunctionTypeRef();
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
};

struct BinaryExpression : public Expression {
	Token* op;
	Expression* left;
	Expression* right;
	BinaryExpression(const SourceMeta& sourceMeta, Token* op, Expression* left,
					 Expression* right);
	virtual ~BinaryExpression();
};

struct UnaryPrefixExpression : public Expression {
	Token* op;
	Expression* arg;
	UnaryPrefixExpression(const SourceMeta& sourceMeta, Token* op,
						  Expression* arg);
	virtual ~UnaryPrefixExpression();
};

struct UnaryPostfixExpression : public Expression {
	Token* op;
	Expression* arg;
	UnaryPostfixExpression(const SourceMeta& sourceMeta, Token* op,
						   Expression* arg);
	virtual ~UnaryPostfixExpression();
};

struct FunctionCallExpression : public Expression {
	Expression* caller;
	List<Expression*> args;
	FunctionCallExpression(const SourceMeta& sourceMeta, Expression* caller,
						   List<Expression*> args);
	virtual ~FunctionCallExpression();
};

struct SubscriptExpression : public Expression {
	Expression* target;
	Expression* index;
	SubscriptExpression(const SourceMeta& sourceMeta, Expression* target,
						Expression* index);
	virtual ~SubscriptExpression();
};

struct CastingExpression : public Expression {
	Token* op;
	Expression* left;
	TypeRef* right;
	CastingExpression(const SourceMeta& sourceMeta, Token* op, Expression* left,
					  TypeRef* right);
	virtual ~CastingExpression();
};

struct MapLiteralExpression : public Expression {
	List<Expression*> keys;
	List<Expression*> values;
	MapLiteralExpression(const SourceMeta& sourceMeta,
						 const List<Expression*>& keys,
						 const List<Expression*>& values);
	virtual ~MapLiteralExpression();
};

struct ArrayLiteralExpression : public Expression {
	List<Expression*> elements;
	ArrayLiteralExpression(const SourceMeta& sourceMeta,
						   const List<Expression*>& elements);
	virtual ~ArrayLiteralExpression();
};

struct TupleLiteralExpression : public Expression {
	List<Expression*> elements;
	TupleLiteralExpression(const SourceMeta& sourceMeta,
						   const List<Expression*>& elements);
	virtual ~TupleLiteralExpression();
};

struct LiteralExpression : public Expression {
	Token* value;
	LiteralExpression(Token* value);
	virtual ~LiteralExpression();
};

struct Parameter;
struct FunctionBlock;

struct LambdaExpression : public Expression {
	List<Token*> modifiers;	 // For things like "async"
	List<Parameter*> parameters;
	FunctionBlock* block;
	LambdaExpression(const SourceMeta& sourceMeta,
					 const List<Token*>& modifiers,
					 const List<Parameter*>& parameters, FunctionBlock* block);
	virtual ~LambdaExpression();
};

struct Symbol : public Node {
	Token* id;
	Symbol(Token* id);
	virtual ~Symbol();
};

struct Parameter : public Symbol {
	List<Token*> modifiers;
	TypeRef* declaredType;
	Parameter(const List<Token*>& modifiers, Token* id, TypeRef* declaredType);
	virtual ~Parameter();
};

struct FunctionBlock : public Node {
	List<Token*> modifiers;	 // For things like "unsafe" blocks
	List<Node*> content;
	FunctionBlock(const SourceMeta& sourceMeta, const List<Token*>& modifiers,
				  const List<Node*>& content);
	virtual ~FunctionBlock();
};

struct GenericType;

struct Function : public Symbol {
	List<Token*> modifiers;
	List<GenericType*> generics;
	List<Parameter*> parameters;
	TypeRef* declaredReturnType;
	FunctionBlock* block;
	Function(const List<Token*>& modifiers, Token* id,
			 const List<GenericType*>& generics,
			 const List<Parameter*>& parameters, TypeRef* declaredReturnType,
			 FunctionBlock* block);
	virtual ~Function();
};

struct Variable : public Symbol {
	List<Token*> modifiers;
	bool constant;
	TypeRef* declaredType;
	Node* value;  // This is a Node and not an Expression because this could be
				  // a VariableBlock instead of an Expression
	Variable(const List<Token*>& modifiers, Token* id, TypeRef* declaredType,
			 Node* value, bool constant);
	virtual ~Variable();
};

struct ConditionalBlock : public Node {
	Expression* condition;
	FunctionBlock* block;
	ConditionalBlock(const SourceMeta& sourceMeta, Expression* condition,
					 FunctionBlock* block);
	virtual ~ConditionalBlock();
};

struct IfBlock : public ConditionalBlock {
	List<ConditionalBlock*> elifBlocks;
	FunctionBlock* elseBlock;
	IfBlock(const SourceMeta& sourceMeta, Expression* condition,
			FunctionBlock* block, const List<ConditionalBlock*>& elifBlocks,
			FunctionBlock* elseBlock);
	virtual ~IfBlock();
};

struct WhileBlock : public ConditionalBlock {
	WhileBlock(const SourceMeta& sourceMeta, Expression* condition,
			   FunctionBlock* block);
	virtual ~WhileBlock();
};

struct RepeatBlock : public ConditionalBlock {
	RepeatBlock(const SourceMeta& sourceMeta, Expression* condition,
				FunctionBlock* block);
	virtual ~RepeatBlock();
};

struct CatchBlock : public Node {
	Parameter* exceptionVariable;
	FunctionBlock* block;
	CatchBlock(const SourceMeta& sourceMeta, Parameter* exceptionVariable,
			   FunctionBlock* block);
	virtual ~CatchBlock();
};

struct TryBlock : public Node {
	FunctionBlock* block;
	List<CatchBlock*> catchBlocks;
	TryBlock(const SourceMeta& sourceMeta, FunctionBlock* block,
			 const List<CatchBlock*>& catchBlocks);
	virtual ~TryBlock();
};

struct SwitchCaseBlock : public Node {
	Token* caseType;  // "case" or "default"
	FunctionBlock* block;
	SwitchCaseBlock(const SourceMeta& sourceMeta, Token* caseType,
					FunctionBlock* block);
	virtual ~SwitchCaseBlock();
};

struct SwitchBlock : public Node {
	Expression* condition;
	List<SwitchCaseBlock*> cases;
	SwitchBlock(const SourceMeta& sourceMeta, Expression* condition,
				const List<SwitchCaseBlock*>& cases);
	virtual ~SwitchBlock();
};

struct ReturnStatement : public Node {
	Expression* value;
	ReturnStatement(const SourceMeta& sourceMeta, Expression* value);
	virtual ~ReturnStatement();
};

struct ThrowStatement : public Node {
	Expression* value;
	ThrowStatement(const SourceMeta& sourceMeta, Expression* value);
	virtual ~ThrowStatement();
};

struct SingleTokenStatement : public Node {
	Token* content;
	SingleTokenStatement(Token* content);
	virtual ~SingleTokenStatement();
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
};

struct Alias : public Type {
	List<Token*> modifiers;
	TypeRef* value;
	Alias(const List<Token*>& modifiers, Token* id,
		  const List<GenericType*>& generics, TypeRef* value);
	virtual ~Alias();
};

struct SetBlock : public Node {
	List<Token*> modifiers;
	Parameter* parameter;
	FunctionBlock* block;
	SetBlock(const SourceMeta& sourceMeta, const List<Token*>& modifiers,
			 Parameter* parameter, FunctionBlock* block);
	virtual ~SetBlock();
};

struct VariableBlock : public Node {
	FunctionBlock* getBlock;
	SetBlock* setBlock;
	FunctionBlock* initBlock;
	VariableBlock(const SourceMeta& sourceMeta, FunctionBlock* getBlock,
				  SetBlock* setBlock, FunctionBlock* initBlock);
	virtual ~VariableBlock();
};

struct Class : public Type {
	List<Token*> modifiers;
	TypeRef* declaredParentType;
	List<TypeRef*> usedTemplates;
	List<Node*> content;
	Class(const List<Token*>& modifiers, Token* id,
		  const List<GenericType*>& generics, TypeRef* declaredParentType,
		  const List<TypeRef*>& usedTemplates, const List<Node*>& content);
	virtual ~Class();
};

struct Struct : public Type {
	List<Token*> modifiers;
	TypeRef* declaredParentType;
	List<TypeRef*> usedTemplates;
	List<Node*> content;
	Struct(const List<Token*>& modifiers, Token* id,
		   const List<GenericType*>& generics, TypeRef* declaredParentType,
		   const List<TypeRef*>& usedTemplates, const List<Node*>& content);
	virtual ~Struct();
};

struct Template : public Type {
	List<Token*> modifiers;
	List<TypeRef*> declaredParentTypes;
	List<Node*> content;
	Template(const List<Token*>& modifiers, Token* id,
			 const List<GenericType*>& generics,
			 const List<TypeRef*>& declaredParentTypes,
			 const List<Node*>& content);
	virtual ~Template();
};

struct Enum : public Type {
	List<Token*> modifiers;
	List<TypeRef*> usedTemplates;
	List<Node*> content;
	Enum(const List<Token*>& modifiers, Token* id,
		 const List<GenericType*>& generics,
		 const List<TypeRef*>& usedTemplates, const List<Node*>& content);
	virtual ~Enum();
};

struct Namespace : public Symbol {
	List<GenericType*> generics;
	List<Token*> modifiers;
	List<Node*> content;
	Namespace(const List<Token*> modifiers, Token* id,
			  const List<GenericType*>& generics, const List<Node*>& content);
	virtual ~Namespace();
};

struct Constructor : public Symbol {
	List<Token*> modifiers;
	List<Parameter*> parameters;
	FunctionBlock* block;
	Constructor(const List<Token*>& modifiers, Token* id,
				const List<Parameter*>& parameters, FunctionBlock* block);
	virtual ~Constructor();
};

struct Destructor : public Node {
	List<Token*> modifiers;
	FunctionBlock* block;
	Destructor(const SourceMeta& sourceMeta, List<Token*>& modifiers,
			   FunctionBlock* block);
	virtual ~Destructor();
};

struct EnumCase : public Symbol {
	List<Token*> modifiers;
	List<Expression*> args;
	EnumCase(const List<Token*>& modifiers, Token* id,
			 const List<Expression*>& args);
	virtual ~EnumCase();
};

struct ImportTarget : public Node {
	Token* id;
	TypeRef* declaredType;
	ImportTarget(Token* id, TypeRef* declaredType);
	virtual ~ImportTarget();
};

struct Import : public Node {
	Token* source;
	Token* alias;
	List<ImportTarget*> targets;
	Import(Token* source, Token* alias, const List<ImportTarget*>& targets);
	virtual ~Import();
};

struct MetaDeclaration : public Node {
	Token* content;
	MetaDeclaration(Token* content);
	virtual ~MetaDeclaration();
};
}  // namespace acl