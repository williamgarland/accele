#include "type_builder.hpp"

namespace acl {
namespace tb {
TypeRef* base(Type* referent, const List<TypeRef*>& generics,
			  const SourceMeta& meta) {
	auto result = new SimpleTypeRef(
		meta,
		new Token(referent->id->type, referent->id->data, referent->id->meta),
		generics, nullptr);
	result->referent = referent;
	return result;
}

TypeRef* optional(TypeRef* content) {
	return new SuffixTypeRef(
		content->sourceMeta, content,
		new Token(TokenType::QUESTION_MARK, "?", content->sourceMeta));
}

TypeRef* unwrappedOptional(TypeRef* content) {
	return new SuffixTypeRef(
		content->sourceMeta, content,
		new Token(TokenType::EXCLAMATION_POINT, "!", content->sourceMeta));
}

TypeRef* pointer(TypeRef* content) {
	return new SuffixTypeRef(
		content->sourceMeta, content,
		new Token(TokenType::ASTERISK, "*", content->sourceMeta));
}

TypeRef* array(TypeRef* content) {
	return new ArrayTypeRef(content->sourceMeta, content);
}

TypeRef* map(TypeRef* key, TypeRef* value) {
	return new MapTypeRef(key->sourceMeta, key, value);
}

TypeRef* tuple(std::initializer_list<TypeRef*> types) {
	List<TypeRef*> elements;
	elements.insert(elements.end(), types.begin(), types.end());
	return new TupleTypeRef(elements[0]->sourceMeta, elements);
}

FunctionTypeRef* function(std::initializer_list<TypeRef*> paramTypes,
						  TypeRef* returnType) {
	List<TypeRef*> params;
	params.insert(params.end(), paramTypes.begin(), paramTypes.end());
	return function(params, returnType);
}

FunctionTypeRef* function(const List<TypeRef*>& paramTypes,
						  TypeRef* returnType) {
	return new FunctionTypeRef(returnType->sourceMeta, paramTypes, returnType);
}
}  // namespace tb
}  // namespace acl