#include "ast.hpp"

namespace acl {
Node::Node(const SourceMeta& sourceMeta) : sourceMeta(sourceMeta) {}

Node::~Node() {}

Ast::Ast(const List<Node*>& content) : content(content) {}

Ast::~Ast() {
	for (auto& c : content) delete c;
}

TypeRef::TypeRef(const SourceMeta& sourceMeta) : Node(sourceMeta) {}

TypeRef::~TypeRef() {}

SimpleTypeRef::SimpleTypeRef(const SourceMeta& sourceMeta, Token* id,
							 const List<TypeRef*>& generics,
							 SimpleTypeRef* parent)
	: TypeRef(sourceMeta), id(id), generics(generics), parent(parent) {}

SimpleTypeRef::~SimpleTypeRef() {
	delete id;
	for (auto& c : generics) delete c;
	delete parent;
}

SuffixTypeRef::SuffixTypeRef(const SourceMeta& sourceMeta, TypeRef* type,
							 Token* suffixSymbol)
	: TypeRef(sourceMeta), type(type), suffixSymbol(suffixSymbol) {}

SuffixTypeRef::~SuffixTypeRef() {
	delete type;
	delete suffixSymbol;
}

TupleTypeRef::TupleTypeRef(const SourceMeta& sourceMeta,
						   const List<TypeRef*>& elementTypes)
	: TypeRef(sourceMeta), elementTypes(elementTypes) {}

TupleTypeRef::~TupleTypeRef() {
	for (auto& c : elementTypes) delete c;
}

MapTypeRef::MapTypeRef(const SourceMeta& sourceMeta, TypeRef* keyType,
					   TypeRef* valueType)
	: TypeRef(sourceMeta), keyType(keyType), valueType(valueType) {}

MapTypeRef::~MapTypeRef() {
	delete keyType;
	delete valueType;
}

ArrayTypeRef::ArrayTypeRef(const SourceMeta& sourceMeta, TypeRef* elementType)
	: TypeRef(sourceMeta), elementType(elementType) {}

ArrayTypeRef::~ArrayTypeRef() { delete elementType; }

FunctionTypeRef::FunctionTypeRef(const SourceMeta& sourceMeta,
								 const List<TypeRef*>& paramTypes,
								 TypeRef* returnType)
	: TypeRef(sourceMeta), paramTypes(paramTypes), returnType(returnType) {}

FunctionTypeRef::~FunctionTypeRef() {
	for (auto& c : paramTypes) delete c;
	delete returnType;
}

Expression::Expression(const SourceMeta& sourceMeta) : Node(sourceMeta) {}

Expression::~Expression() {}

TernaryExpression::TernaryExpression(const SourceMeta& sourceMeta,
									 Expression* arg0, Expression* arg1,
									 Expression* arg2)
	: Expression(sourceMeta), arg0(arg0), arg1(arg1), arg2(arg2) {}

TernaryExpression::~TernaryExpression() {
	delete arg0;
	delete arg1;
	delete arg2;
}

BinaryExpression::BinaryExpression(const SourceMeta& sourceMeta, Token* op,
								   Expression* left, Expression* right)
	: Expression(sourceMeta), op(op), left(left), right(right) {}

BinaryExpression::~BinaryExpression() {
	delete op;
	delete left;
	delete right;
}

UnaryPrefixExpression::UnaryPrefixExpression(const SourceMeta& sourceMeta,
											 Token* op, Expression* arg)
	: Expression(sourceMeta), op(op), arg(arg) {}

UnaryPrefixExpression::~UnaryPrefixExpression() {
	delete op;
	delete arg;
}

UnaryPostfixExpression::UnaryPostfixExpression(const SourceMeta& sourceMeta,
											   Token* op, Expression* arg)
	: Expression(sourceMeta), op(op), arg(arg) {}

UnaryPostfixExpression::~UnaryPostfixExpression() {
	delete op;
	delete arg;
}

FunctionCallExpression::FunctionCallExpression(const SourceMeta& sourceMeta,
											   Expression* caller,
											   List<Expression*> args)
	: Expression(sourceMeta), caller(caller), args(args) {}

FunctionCallExpression::~FunctionCallExpression() {
	delete caller;
	for (auto& c : args) delete c;
}

SubscriptExpression::SubscriptExpression(const SourceMeta& sourceMeta,
										 Expression* target, Expression* index)
	: Expression(sourceMeta), target(target), index(index) {}

SubscriptExpression::~SubscriptExpression() {
	delete target;
	delete index;
}

CastingExpression::CastingExpression(const SourceMeta& sourceMeta, Token* op,
									 Expression* left, TypeRef* right)
	: Expression(sourceMeta), op(op), left(left), right(right) {}

CastingExpression::~CastingExpression() {
	delete op;
	delete left;
	delete right;
}

MapLiteralExpression::MapLiteralExpression(const SourceMeta& sourceMeta,
										   const List<Expression*>& keys,
										   const List<Expression*>& values)
	: Expression(sourceMeta), keys(keys), values(values) {}

MapLiteralExpression::~MapLiteralExpression() {
	for (auto& c : keys) delete c;
	for (auto& c : values) delete c;
}

ArrayLiteralExpression::ArrayLiteralExpression(
	const SourceMeta& sourceMeta, const List<Expression*>& elements)
	: Expression(sourceMeta), elements(elements) {}

ArrayLiteralExpression::~ArrayLiteralExpression() {
	for (auto& c : elements) delete c;
}

TupleLiteralExpression::TupleLiteralExpression(
	const SourceMeta& sourceMeta, const List<Expression*>& elements)
	: Expression(sourceMeta), elements(elements) {}

TupleLiteralExpression::~TupleLiteralExpression() {
	for (auto& c : elements) delete c;
}

LiteralExpression::LiteralExpression(Token* value)
	: Expression(value->meta), value(value) {}

LiteralExpression::~LiteralExpression() { delete value; }

LambdaExpression::LambdaExpression(const SourceMeta& sourceMeta,
								   const List<Token*>& modifiers,
								   const List<Parameter*>& parameters,
								   FunctionBlock* block)
	: Expression(sourceMeta),
	  modifiers(modifiers),
	  parameters(parameters),
	  block(block) {}

LambdaExpression::~LambdaExpression() {
	for (auto& c : modifiers) delete c;
	for (auto& c : parameters) delete c;
	delete block;
}

Symbol::Symbol(Token* id) : Node(id->meta), id(id) {}

Symbol::~Symbol() { delete id; }

Parameter::Parameter(const List<Token*>& modifiers, Token* id,
					 TypeRef* declaredType)
	: Symbol(id), modifiers(modifiers), declaredType(declaredType) {}

Parameter::~Parameter() {
	for (auto& c : modifiers) delete c;
	delete declaredType;
}

FunctionBlock::FunctionBlock(const SourceMeta& sourceMeta,
							 const List<Token*>& modifiers,
							 const List<Node*>& content)
	: Node(sourceMeta), modifiers(modifiers), content(content) {}

FunctionBlock::~FunctionBlock() {
	for (auto& c : modifiers) delete c;
	for (auto& c : content) delete c;
}

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

Function::~Function() {
	for (auto& c : modifiers) delete c;
	for (auto& c : generics) delete c;
	for (auto& c : parameters) delete c;
	delete declaredReturnType;
	delete block;
}

Variable::Variable(const List<Token*>& modifiers, Token* id,
				   TypeRef* declaredType, Node* value, bool constant)
	: Symbol(id),
	  modifiers(modifiers),
	  declaredType(declaredType),
	  value(value),
	  constant(constant) {}

Variable::~Variable() {
	for (auto& c : modifiers) delete c;
	delete declaredType;
	delete value;
}

ConditionalBlock::ConditionalBlock(const SourceMeta& sourceMeta,
								   Expression* condition, FunctionBlock* block)
	: Node(sourceMeta), condition(condition), block(block) {}

ConditionalBlock::~ConditionalBlock() {
	delete condition;
	delete block;
}

IfBlock::IfBlock(const SourceMeta& sourceMeta, Expression* condition,
				 FunctionBlock* block,
				 const List<ConditionalBlock*>& elifBlocks,
				 FunctionBlock* elseBlock)
	: ConditionalBlock(sourceMeta, condition, block),
	  elifBlocks(elifBlocks),
	  elseBlock(elseBlock) {}

IfBlock::~IfBlock() {
	for (auto& c : elifBlocks) delete c;
	delete elseBlock;
}

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

CatchBlock::~CatchBlock() {
	delete exceptionVariable;
	delete block;
}

TryBlock::TryBlock(const SourceMeta& sourceMeta, FunctionBlock* block,
				   const List<CatchBlock*>& catchBlocks)
	: Node(sourceMeta), block(block), catchBlocks(catchBlocks) {}

TryBlock::~TryBlock() {
	delete block;
	for (auto& c : catchBlocks) delete c;
}

SwitchCaseBlock::SwitchCaseBlock(const SourceMeta& sourceMeta, Token* caseType,
								 FunctionBlock* block)
	: Node(sourceMeta), caseType(caseType), block(block) {}

SwitchCaseBlock::~SwitchCaseBlock() {
	delete caseType;
	delete block;
}

SwitchBlock::SwitchBlock(const SourceMeta& sourceMeta, Expression* condition,
						 const List<SwitchCaseBlock*>& cases)
	: Node(sourceMeta), condition(condition), cases(cases) {}

SwitchBlock::~SwitchBlock() {
	delete condition;
	for (auto& c : cases) delete c;
}

ReturnStatement::ReturnStatement(const SourceMeta& sourceMeta,
								 Expression* value)
	: Node(sourceMeta), value(value) {}

ReturnStatement::~ReturnStatement() { delete value; }

ThrowStatement::ThrowStatement(const SourceMeta& sourceMeta, Expression* value)
	: Node(sourceMeta), value(value) {}

ThrowStatement::~ThrowStatement() { delete value; }

SingleTokenStatement::SingleTokenStatement(Token* content)
	: Node(content->meta), content(content) {}

SingleTokenStatement::~SingleTokenStatement() { delete content; }

Type::Type(Token* id, const List<GenericType*>& generics)
	: Symbol(id), generics(generics) {}

Type::~Type() {
	for (auto& c : generics) delete c;
}

GenericType::GenericType(Token* id, TypeRef* declaredParentType)
	: Type(id, {}), declaredParentType(declaredParentType) {}

GenericType::~GenericType() { delete declaredParentType; }

Alias::Alias(const List<Token*>& modifiers, Token* id,
			 const List<GenericType*>& generics, TypeRef* value)
	: Type(id, generics), modifiers(modifiers), value(value) {}

Alias::~Alias() {
	for (auto& c : modifiers) delete c;
	delete value;
}

SetBlock::SetBlock(const SourceMeta& sourceMeta, const List<Token*>& modifiers,
				   Parameter* parameter, FunctionBlock* block)
	: Node(sourceMeta),
	  modifiers(modifiers),
	  parameter(parameter),
	  block(block) {}

SetBlock::~SetBlock() {
	for (auto& c : modifiers) delete c;
	delete parameter;
	delete block;
}

VariableBlock::VariableBlock(const SourceMeta& sourceMeta,
							 FunctionBlock* getBlock, SetBlock* setBlock,
							 FunctionBlock* initBlock)
	: Node(sourceMeta),
	  getBlock(getBlock),
	  setBlock(setBlock),
	  initBlock(initBlock) {}

VariableBlock::~VariableBlock() {
	delete getBlock;
	delete setBlock;
	delete initBlock;
}

Class::Class(const List<Token*>& modifiers, Token* id,
			 const List<GenericType*>& generics, TypeRef* declaredParentType,
			 const List<TypeRef*>& usedTemplates, const List<Node*>& content)
	: Type(id, generics),
	  modifiers(modifiers),
	  declaredParentType(declaredParentType),
	  usedTemplates(usedTemplates),
	  content(content) {}

Class::~Class() {
	for (auto& c : modifiers) delete c;
	delete declaredParentType;
	for (auto& c : usedTemplates) delete c;
	for (auto& c : content) delete c;
}

Struct::Struct(const List<Token*>& modifiers, Token* id,
			   const List<GenericType*>& generics, TypeRef* declaredParentType,
			   const List<TypeRef*>& usedTemplates, const List<Node*>& content)
	: Type(id, generics),
	  modifiers(modifiers),
	  declaredParentType(declaredParentType),
	  usedTemplates(usedTemplates),
	  content(content) {}

Struct::~Struct() {
	for (auto& c : modifiers) delete c;
	delete declaredParentType;
	for (auto& c : usedTemplates) delete c;
	for (auto& c : content) delete c;
}

Template::Template(const List<Token*>& modifiers, Token* id,
				   const List<GenericType*>& generics,
				   const List<TypeRef*>& declaredParentTypes,
				   const List<Node*>& content)
	: Type(id, generics),
	  modifiers(modifiers),
	  declaredParentTypes(declaredParentTypes),
	  content(content) {}

Template::~Template() {
	for (auto& c : modifiers) delete c;
	for (auto& c : declaredParentTypes) delete c;
	for (auto& c : content) delete c;
}

Enum::Enum(const List<Token*>& modifiers, Token* id,
		   const List<GenericType*>& generics,
		   const List<TypeRef*>& usedTemplates, const List<Node*>& content)
	: Type(id, generics),
	  modifiers(modifiers),
	  usedTemplates(usedTemplates),
	  content(content) {}

Enum::~Enum() {
	for (auto& c : modifiers) delete c;
	for (auto& c : usedTemplates) delete c;
	for (auto& c : content) delete c;
}

Namespace::Namespace(const List<Token*> modifiers, Token* id,
					 const List<GenericType*>& generics,
					 const List<Node*>& content)
	: Symbol(id), modifiers(modifiers), generics(generics), content(content) {}

Namespace::~Namespace() {
	for (auto& c : modifiers) delete c;
	for (auto& c : generics) delete c;
	for (auto& c : content) delete c;
}

Constructor::Constructor(const List<Token*>& modifiers, Token* id,
						 const List<Parameter*>& parameters,
						 FunctionBlock* block)
	: Symbol(id), modifiers(modifiers), parameters(parameters), block(block) {}

Constructor::~Constructor() {
	for (auto& c : modifiers) delete c;
	for (auto& c : parameters) delete c;
	delete block;
}

Destructor::Destructor(const SourceMeta& sourceMeta, List<Token*>& modifiers,
					   FunctionBlock* block)
	: Node(sourceMeta), modifiers(modifiers), block(block) {}

Destructor::~Destructor() {
	for (auto& c : modifiers) delete c;
	delete block;
}

EnumCase::EnumCase(const List<Token*>& modifiers, Token* id,
				   const List<Expression*>& args)
	: Symbol(id), modifiers(modifiers), args(args) {}

EnumCase::~EnumCase() {
	for (auto& c : modifiers) delete c;
	for (auto& c : args) delete c;
}

ImportTarget::ImportTarget(Token* id, TypeRef* declaredType)
	: Node(id->meta), id(id), declaredType(declaredType) {}

ImportTarget::~ImportTarget() {
	delete id;
	delete declaredType;
}

Import::Import(Token* source, Token* alias, const List<ImportTarget*>& targets)
	: Node(source->meta), source(source), alias(alias), targets(targets) {}

Import::~Import() {
	delete source;
	delete alias;
	for (auto& c : targets) delete c;
}

MetaDeclaration::MetaDeclaration(Token* content)
	: Node(content->meta), content(content) {}

MetaDeclaration::~MetaDeclaration() { delete content; }

}  // namespace acl