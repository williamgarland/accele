#include <cstring>
#include <fstream>
#include <iostream>

#include "lexer.hpp"
#include "parser.hpp"

void generateAstJson(const acl::Ast* ast, const acl::String& dest) {
	std::ofstream ofs(dest);
	if (ofs) {
		acl::StringBuffer json;
		ast->globalScope->toJson(json);
		acl::String asString = json.str();
		ofs << asString;
	} else
		throw "Expected output destination following AST generation flag";
}

void checkForAstOutputFlag(int argc, char* argv[], const acl::Ast* ast) {
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--genAstJson") == 0) {
			if (i + 1 >= argc)
				throw "Expected output destination following AST generation flag";
			generateAstJson(ast, argv[i + 1]);
		}
	}
}

int main(int argc, char* argv[]) {
	using namespace acl;

	if (argc < 2) return 1;

	String file = String(argv[1]);

	std::ifstream ifs(file);

	if (ifs) {
		StringBuffer buf;
		buf << ifs.rdbuf();
		ifs.close();

		CompilerContext ctx;
		Parser parser(ctx, Lexer(file, buf));

		auto ast = parser.parse();

		checkForAstOutputFlag(argc, argv, ast);

		delete ast;
	}

	return 0;
}