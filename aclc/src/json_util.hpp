#pragma once

#include <functional>

#include "common.hpp"
#include "lexer.hpp"

namespace acl {
namespace json {
template <typename T>
using ListAppendFunc = std::function<void(StringBuffer&, const T&)>;

template <typename T>
void appendList(StringBuffer& dest, const List<T>& list,
				ListAppendFunc<T> appendFunc) {
	dest << "[";
	bool first = true;
	for (const auto& e : list) {
		if (!first) dest << ",";
		dest << "\n";
		appendFunc(dest, e);
		first = false;
	}
	if (!list.empty()) dest << "\n";
	dest << "]";
}

void appendToken(StringBuffer& dest, const Token* token);
void appendStringToken(StringBuffer& dest, const StringToken* token);
void appendBool(StringBuffer& dest, bool value);
}  // namespace json
}  // namespace acl