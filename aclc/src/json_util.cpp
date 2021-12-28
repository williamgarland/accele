#include "json_util.hpp"

namespace acl {
namespace json {
void appendToken(StringBuffer& dest, const Token* token) {
	if (const auto* t = dynamic_cast<const StringToken*>(token)) {
		appendStringToken(dest, t);
	} else if (token->type == TokenType::INTEGER_LITERAL ||
			   token->type == TokenType::FLOAT_LITERAL) {
		dest << token->data;
	} else {
		dest << "\"" << token->data << "\"";
	}
}

void appendStringToken(StringBuffer& dest, const StringToken* token) {
	dest << "\"";
	for (char c : token->data) {
		if (c == '\\') dest << "\\";
		dest << c;
	}
	dest << "\"";
}

void appendBool(StringBuffer& dest, bool value) {
	if (value)
		dest << "true";
	else
		dest << "false";
}
}  // namespace json
}  // namespace acl