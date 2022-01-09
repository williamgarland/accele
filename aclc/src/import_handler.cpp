#include "import_handler.hpp"

#include <fstream>

#include "exceptions.hpp"
#include "parser.hpp"

namespace acl {
ImportHandler::ImportHandler(CompilerContext& ctx, Ast* ast)
	: ctx(ctx), ast(ast) {}

void ImportHandler::resolveImports() {
	for (auto& i : ast->globalScope->imports) {
		resolveImport(i);
	}
}

void ImportHandler::resolveImport(Import* i) {
	resolveImportSource(i->source);

	// TODO: Handle import targets and import aliases
}

void ImportHandler::getPossibleBaseDirs(bool declaredRelative,
										List<std::filesystem::path>& dest) {
	if (declaredRelative) {
		dest.push_back(ast->moduleInfo.dir);
		return;
	}

	for (auto& dir : ctx.additionalImportDirs) dest.push_back(dir);

	dest.push_back(ctx.globalImportDir);
}

std::filesystem::path resolveImportSourceParent(
	ImportSource* parent, const std::filesystem::path& currentDir) {
	auto p = currentDir;
	if (parent->parent) p = resolveImportSourceParent(parent->parent, p);

	if (parent->content->type == TokenType::DOT) return p.parent_path();

	return p / parent->content->data;
}

Ast* ImportHandler::resolveImportSource(ImportSource* src) {
	if (src->content->type == TokenType::STRING_LITERAL)
		return resolveImportSourcePath(src->content->data, src->sourceMeta);

	List<std::filesystem::path> possibleBaseDirs;
	getPossibleBaseDirs(src->declaredRelative, possibleBaseDirs);

	for (auto& dir : possibleBaseDirs) {
		if (src->parent) {
			dir = resolveImportSourceParent(src->parent, dir);
		}

		try {
			return resolveImportSourcePath(
				dir / (src->content->data + ".accele"), src->sourceMeta);
		} catch (UnresolvedImportException& e) {
			try {
				return resolveImportSourcePath(
					dir / (src->content->data + ".acldef"), src->sourceMeta);
			} catch (UnresolvedImportException& e2) {
			}
		}
	}

	throw UnresolvedImportException(ASP_CORE_UNKNOWN, src->sourceMeta,
									"Unresolved import");
}

Ast* ImportHandler::resolveImportSourcePath(const std::filesystem::path& path,
											const SourceMeta& meta) {
	if (!std::filesystem::exists(path))
		throw UnresolvedImportException(ASP_CORE_UNKNOWN, meta,
										"The specified module does not exist");
	if (std::filesystem::is_directory(path))
		throw UnresolvedImportException(ASP_CORE_UNKNOWN, meta,
										"The specified module is not a file");

	for (const auto& m : ctx.modules) {
		if (m->moduleInfo.path == path) return m;
	}

	return compileImport(path, meta);
}

Ast* ImportHandler::compileImport(const std::filesystem::path& path,
								  const SourceMeta& meta) {
	std::ifstream ifs(path);

	if (ifs) {
		StringBuffer buf;
		buf << ifs.rdbuf();
		ifs.close();

		Parser parser(ctx, Lexer(path.string(), buf));

		auto ast = parser.parse();

		ctx.modules.push_back(ast);

		return ast;
	} else
		throw UnresolvedImportException(ASP_CORE_UNKNOWN, meta,
										"Unresolved import");
}
}  // namespace acl