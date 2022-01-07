#include "type_builder.hpp"

#include "invariant_types.hpp"

namespace acl {
namespace tb {
TypeRef* base(Type* referent, const List<TypeRef*>& generics,
			  const SourceMeta& meta) {
	auto result = new SimpleTypeRef(
		meta,
		new Token(referent->id->type, referent->id->data, referent->id->meta),
		generics, nullptr);
	result->referent = referent;
	result->actualType = referent;
	result->actualGenerics = generics;
	return result;
}

TypeRef* optional(TypeRef* content) {
	auto result = new SuffixTypeRef(
		content->sourceMeta, content,
		new Token(TokenType::QUESTION_MARK, "?", content->sourceMeta));

	result->actualType = const_cast<bt::InvariantType*>(bt::OPTIONAL);
	result->actualGenerics.push_back(content);

	return result;
}

TypeRef* unwrappedOptional(TypeRef* content) {
	auto result = new SuffixTypeRef(
		content->sourceMeta, content,
		new Token(TokenType::EXCLAMATION_POINT, "!", content->sourceMeta));

	result->actualType = const_cast<bt::InvariantType*>(bt::UNWRAPPED_OPTIONAL);
	result->actualGenerics.push_back(content);

	return result;
}

TypeRef* pointer(TypeRef* content) {
	auto result = new SuffixTypeRef(
		content->sourceMeta, content,
		new Token(TokenType::ASTERISK, "*", content->sourceMeta));

	result->actualType = const_cast<bt::InvariantType*>(bt::POINTER);
	result->actualGenerics.push_back(content);

	return result;
}

TypeRef* array(TypeRef* content) {
	auto result = new ArrayTypeRef(content->sourceMeta, content);

	result->actualType = const_cast<bt::InvariantType*>(bt::ARRAY);
	result->actualGenerics.push_back(content);

	return result;
}

TypeRef* map(TypeRef* key, TypeRef* value) {
	auto result = new MapTypeRef(key->sourceMeta, key, value);

	result->actualType = const_cast<bt::InvariantType*>(bt::MAP);
	result->actualGenerics.push_back(key);
	result->actualGenerics.push_back(value);

	return result;
}

TypeRef* tuple(std::initializer_list<TypeRef*> types) {
	List<TypeRef*> elements;
	elements.insert(elements.end(), types.begin(), types.end());
	return tuple(elements);
}

TypeRef* tuple(const List<TypeRef*>& types) {
	auto result = new TupleTypeRef(types[0]->sourceMeta, types);

	result->actualType = const_cast<bt::InvariantType*>(bt::TUPLE);
	result->actualGenerics.insert(result->actualGenerics.end(), types.begin(),
								  types.end());

	return result;
}

FunctionTypeRef* function(std::initializer_list<TypeRef*> paramTypes,
						  TypeRef* returnType) {
	List<TypeRef*> params;
	params.insert(params.end(), paramTypes.begin(), paramTypes.end());
	return function(params, returnType);
}

FunctionTypeRef* function(const List<TypeRef*>& paramTypes,
						  TypeRef* returnType) {
	auto result =
		new FunctionTypeRef(returnType->sourceMeta, paramTypes, returnType);

	result->actualType = const_cast<bt::InvariantType*>(bt::FUNCTION);
	result->actualGenerics.push_back(returnType);
	result->actualGenerics.insert(result->actualGenerics.end(),
								  paramTypes.begin(), paramTypes.end());

	return result;
}
}  // namespace tb
}  // namespace acl