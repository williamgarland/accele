#include "ast.hpp"

namespace acl {
Node::Node(const SourceMeta& sourceMeta) {}

Node::~Node() {}

TypeRef::TypeRef(const SourceMeta& sourceMeta) : Node(sourceMeta) {}

TypeRef::~TypeRef() {}

SimpleTypeRef::SimpleTypeRef(const SourceMeta& sourceMeta, Token* id,
							 const List<TypeRef*>& generics,
							 SimpleTypeRef* parent)
	: TypeRef(sourceMeta), id(id), generics(generics), parent(parent) {}

SimpleTypeRef::~SimpleTypeRef() {}

SuffixTypeRef::SuffixTypeRef(const SourceMeta& sourceMeta, TypeRef* type,
							 Token* suffixSymbol)
	: TypeRef(sourceMeta), type(type), suffixSymbol(suffixSymbol) {}

SuffixTypeRef::~SuffixTypeRef() {}

TupleTypeRef::TupleTypeRef(const SourceMeta& sourceMeta,
						   const List<TypeRef*>& elementTypes)
	: TypeRef(sourceMeta), elementTypes(elementTypes) {}

TupleTypeRef::~TupleTypeRef() {}

MapTypeRef::MapTypeRef(const SourceMeta& sourceMeta, TypeRef* keyType,
					   TypeRef* valueType)
	: TypeRef(sourceMeta), keyType(keyType), valueType(valueType) {}

MapTypeRef::~MapTypeRef() {}

ArrayTypeRef::ArrayTypeRef(const SourceMeta& sourceMeta, TypeRef* elementType)
	: TypeRef(sourceMeta), elementType(elementType) {}

ArrayTypeRef::~ArrayTypeRef() {}

FunctionTypeRef::FunctionTypeRef(const SourceMeta& sourceMeta,
								 const List<TypeRef*>& paramTypes,
								 TypeRef* returnType)
	: TypeRef(sourceMeta), paramTypes(paramTypes), returnType(returnType) {}

FunctionTypeRef::~FunctionTypeRef() {}

Expression::Expression(const SourceMeta& sourceMeta) : Node(sourceMeta) {}

Expression::~Expression() {}

TernaryExpression::TernaryExpression(const SourceMeta& sourceMeta,
									 Expression* arg0, Expression* arg1,
									 Expression* arg2)
	: Expression(sourceMeta), arg0(arg0), arg1(arg1), arg2(arg2) {}

TernaryExpression::~TernaryExpression() {}

BinaryExpression::BinaryExpression(const SourceMeta& sourceMeta, Token* op,
								   Expression* left, Expression* right)
	: Expression(sourceMeta), op(op), left(left), right(right) {}

BinaryExpression::~BinaryExpression() {}

UnaryPrefixExpression::UnaryPrefixExpression(const SourceMeta& sourceMeta,
											 Token* op, Expression* arg)
	: Expression(sourceMeta), op(op), arg(arg) {}

UnaryPrefixExpression::~UnaryPrefixExpression() {}

UnaryPostfixExpression::UnaryPostfixExpression(const SourceMeta& sourceMeta,
											   Token* op, Expression* arg)
	: Expression(sourceMeta), op(op), arg(arg) {}

UnaryPostfixExpression::~UnaryPostfixExpression() {}

FunctionCallExpression::FunctionCallExpression(const SourceMeta& sourceMeta,
											   Expression* caller,
											   List<Expression*> args)
	: Expression(sourceMeta), caller(caller), args(args) {}

FunctionCallExpression::~FunctionCallExpression() {}

SubscriptExpression::SubscriptExpression(const SourceMeta& sourceMeta,
										 Expression* target, Expression* index)
	: Expression(sourceMeta), target(target), index(index) {}

SubscriptExpression::~SubscriptExpression() {}

CastingExpression::CastingExpression(const SourceMeta& sourceMeta, Token* op,
									 Expression* left, TypeRef* right)
	: Expression(sourceMeta), op(op), left(left), right(right) {}

CastingExpression::~CastingExpression() {}

MapLiteralExpression::MapLiteralExpression(const SourceMeta& sourceMeta,
										   const List<Expression*>& keys,
										   const List<Expression*>& values)
	: Expression(sourceMeta), keys(keys), values(values) {}

MapLiteralExpression::~MapLiteralExpression() {}

ArrayLiteralExpression::ArrayLiteralExpression(
	const SourceMeta& sourceMeta, const List<Expression*>& elements)
	: Expression(sourceMeta), elements(elements) {}

ArrayLiteralExpression::~ArrayLiteralExpression() {}

TupleLiteralExpression::TupleLiteralExpression(
	const SourceMeta& sourceMeta, const List<Expression*>& elements)
	: Expression(sourceMeta), elements(elements) {}

TupleLiteralExpression::~TupleLiteralExpression() {}

LiteralExpression::LiteralExpression(Token* value)
	: Expression(value->meta), value(value) {}

LiteralExpression::~LiteralExpression() {}

LambdaExpression::LambdaExpression(const SourceMeta& sourceMeta,
								   const List<Token*>& modifiers,
								   const List<Parameter*>& parameters,
								   FunctionBlock* block)
	: Expression(sourceMeta),
	  modifiers(modifiers),
	  parameters(parameters),
	  block(block) {}

LambdaExpression::~LambdaExpression() {}

Symbol::Symbol(Token* id) : Node(id->meta), id(id) {}

Symbol::~Symbol() {}

Parameter::Parameter(const List<Token*>& modifiers, Token* id,
					 TypeRef* declaredType)
	: Symbol(id), modifiers(modifiers), declaredType(declaredType) {}

Parameter::~Parameter() {}

FunctionBlock::FunctionBlock(const SourceMeta& sourceMeta,
							 const List<Token*>& modifiers,
							 const List<Node*>& content)
	: Node(sourceMeta), modifiers(modifiers), content(content) {}

FunctionBlock::~FunctionBlock() {}

Function::Function(const List<Token*>& modifiers, Token* id,
				   const List<GenericType*>& generics,
				   const List<Parameter*>& parameters,
				   TypeRef* declaredReturnType, FunctionBlock* block)
	: Symbol(id),
	  modifiers(modifiers),
	  generics(generics),
	  parameters(parameters),
	  declaredReturnType(declaredReturnType),
	  block(block) {}

Function::~Function() {}

Variable::Variable(const List<Token*>& modifiers, Token* id,
				   TypeRef* declaredType, Node* value, bool constant)
	: Symbol(id),
	  modifiers(modifiers),
	  declaredType(declaredType),
	  value(value),
	  constant(constant) {}

Variable::~Variable() {}

ConditionalBlock::ConditionalBlock(const SourceMeta& sourceMeta,
								   Expression* condition, FunctionBlock* block)
	: Node(sourceMeta), condition(condition), block(block) {}

ConditionalBlock::~ConditionalBlock() {}

IfBlock::IfBlock(const SourceMeta& sourceMeta, Expression* condition,
				 FunctionBlock* block,
				 const List<ConditionalBlock*>& elifBlocks,
				 FunctionBlock* elseBlock)
	: ConditionalBlock(sourceMeta, condition, block),
	  elifBlocks(elifBlocks),
	  elseBlock(elseBlock) {}

IfBlock::~IfBlock() {}

WhileBlock::WhileBlock(const SourceMeta& sourceMeta, Expression* condition,
					   FunctionBlock* block)
	: ConditionalBlock(sourceMeta, condition, block) {}

WhileBlock::~WhileBlock() {}

RepeatBlock::RepeatBlock(const SourceMeta& sourceMeta, Expression* condition,
						 FunctionBlock* block)
	: ConditionalBlock(sourceMeta, condition, block) {}

RepeatBlock::~RepeatBlock() {}

CatchBlock::CatchBlock(const SourceMeta& sourceMeta,
					   Parameter* exceptionVariable, FunctionBlock* block)
	: Node(sourceMeta), exceptionVariable(exceptionVariable), block(block) {}

CatchBlock::~CatchBlock() {}

TryBlock::TryBlock(const SourceMeta& sourceMeta, FunctionBlock* block,
				   const List<CatchBlock*>& catchBlocks)
	: Node(sourceMeta), block(block), catchBlocks(catchBlocks) {}

TryBlock::~TryBlock() {}

SwitchCaseBlock::SwitchCaseBlock(const SourceMeta& sourceMeta, Token* caseType,
								 FunctionBlock* block)
	: Node(sourceMeta), caseType(caseType), block(block) {}

SwitchCaseBlock::~SwitchCaseBlock() {}

SwitchBlock::SwitchBlock(const SourceMeta& sourceMeta, Expression* condition,
						 const List<SwitchCaseBlock*>& cases)
	: Node(sourceMeta), condition(condition), cases(cases) {}

SwitchBlock::~SwitchBlock() {}

ReturnStatement::ReturnStatement(const SourceMeta& sourceMeta,
								 Expression* value)
	: Node(sourceMeta), value(value) {}

ReturnStatement::~ReturnStatement() {}

ThrowStatement::ThrowStatement(const SourceMeta& sourceMeta, Expression* value)
	: Node(sourceMeta), value(value) {}

ThrowStatement::~ThrowStatement() {}

SingleTokenStatement::SingleTokenStatement(Token* content)
	: Node(content->meta), content(content) {}

SingleTokenStatement::~SingleTokenStatement() {}

Type::Type(Token* id, const List<GenericType*>& generics)
	: Symbol(id), generics(generics) {}

Type::~Type() {}

GenericType::GenericType(Token* id, TypeRef* declaredParentType)
	: Type(id, {}), declaredParentType(declaredParentType) {}

GenericType::~GenericType() {}

Alias::Alias(const List<Token*>& modifiers, Token* id,
			 const List<GenericType*>& generics, TypeRef* value)
	: Type(id, generics), modifiers(modifiers), value(value) {}

Alias::~Alias() {}

SetBlock::SetBlock(const SourceMeta& sourceMeta, const List<Token*>& modifiers,
				   Parameter* parameter, FunctionBlock* block)
	: Node(sourceMeta),
	  modifiers(modifiers),
	  parameter(parameter),
	  block(block) {}

SetBlock::~SetBlock() {}

VariableBlock::VariableBlock(const SourceMeta& sourceMeta,
							 FunctionBlock* getBlock, SetBlock* setBlock,
							 FunctionBlock* initBlock)
	: Node(sourceMeta),
	  getBlock(getBlock),
	  setBlock(setBlock),
	  initBlock(initBlock) {}

VariableBlock::~VariableBlock() {}

Class::Class(const List<Token*>& modifiers, Token* id,
			 const List<GenericType*>& generics, TypeRef* declaredParentType,
			 const List<TypeRef*>& usedTemplates, const List<Node*>& content)
	: Type(id, generics),
	  modifiers(modifiers),
	  declaredParentType(declaredParentType),
	  usedTemplates(usedTemplates),
	  content(content) {}

Class::~Class() {}

Struct::Struct(const List<Token*>& modifiers, Token* id,
			   const List<GenericType*>& generics, TypeRef* declaredParentType,
			   const List<TypeRef*>& usedTemplates, const List<Node*>& content)
	: Type(id, generics),
	  modifiers(modifiers),
	  declaredParentType(declaredParentType),
	  usedTemplates(usedTemplates),
	  content(content) {}

Struct::~Struct() {}

Template::Template(const List<Token*>& modifiers, Token* id,
				   const List<GenericType*>& generics,
				   const List<TypeRef*>& declaredParentTypes,
				   const List<Node*>& content)
	: Type(id, generics),
	  modifiers(modifiers),
	  declaredParentTypes(declaredParentTypes),
	  content(content) {}

Template::~Template() {}

Enum::Enum(const List<Token*>& modifiers, Token* id,
		   const List<GenericType*>& generics,
		   const List<TypeRef*>& usedTemplates, const List<Node*>& content)
	: Type(id, generics),
	  modifiers(modifiers),
	  usedTemplates(usedTemplates),
	  content(content) {}

Enum::~Enum() {}

Namespace::Namespace(const List<Token*> modifiers, Token* id,
					 const List<GenericType*>& generics,
					 const List<Node*>& content)
	: Symbol(id), modifiers(modifiers), generics(generics), content(content) {}

Namespace::~Namespace() {}

Constructor::Constructor(const List<Token*>& modifiers, Token* id,
						 const List<Parameter*>& parameters,
						 FunctionBlock* block)
	: Symbol(id), modifiers(modifiers), parameters(parameters), block(block) {}

Constructor::~Constructor() {}

Destructor::Destructor(const SourceMeta& sourceMeta, List<Token*>& modifiers,
					   FunctionBlock* block)
	: Node(sourceMeta), modifiers(modifiers), block(block) {}

Destructor::~Destructor() {}

EnumCase::EnumCase(const List<Token*>& modifiers, Token* id,
				   const List<Expression*>& args)
	: Symbol(id), modifiers(modifiers), args(args) {}

EnumCase::~EnumCase() {}

ImportTarget::ImportTarget(Token* id, TypeRef* declaredType)
	: Node(id->meta), id(id), declaredType(declaredType) {}

ImportTarget::~ImportTarget() {}

Import::Import(Token* source, Token* alias, const List<ImportTarget*>& targets)
	: Node(source->meta), source(source), alias(alias), targets(targets) {}

Import::~Import() {}

MetaDeclaration::MetaDeclaration(Token* content)
	: Node(content->meta), content(content) {}

MetaDeclaration::~MetaDeclaration() {}

}  // namespace acl