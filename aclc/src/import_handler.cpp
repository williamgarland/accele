#include "import_handler.hpp"

#include <fstream>

#include "exceptions.hpp"
#include "parser.hpp"
#include "resolver.hpp"

namespace acl {
ImportHandler::ImportHandler(CompilerContext& ctx, Module* mod)
	: ctx(ctx), mod(mod) {}

void ImportHandler::resolveImports() {
	for (auto& i : mod->ast->globalScope->imports) {
		resolveImport(i);
	}
}

void ImportHandler::resolveImport(Import* i) {
	auto ast = resolveImportSource(i->source);

	for (auto& imp : this->mod->ast->globalScope->imports) {
		if (imp->referent == ast)
			throw AclException(ASP_CORE_UNKNOWN, i->sourceMeta,
							   "The target of this import has already been "
							   "imported by a previous import statement");
	}

	i->referent = ast;

	for (auto& t : i->targets) {
		resolveImportTarget(ast, t, i->targets);
	}
}

void ImportHandler::resolveImportTarget(Ast* searchTarget, ImportTarget* target,
										const List<ImportTarget*>& allTargets) {
	for (auto& s : searchTarget->globalScope->symbols) {
		if (target->id->data == s->id->data) {
			const Token* destToken = nullptr;
			if (getSymbolVisibility(mod->ast->globalScope, s, false,
									&destToken) == TokenType::INTERNAL)
				throw UnresolvedSymbolException(target->id);
			for (auto& t : allTargets) {
				for (auto& r : t->referents)
					if (r == s)
						throw AclException(ASP_CORE_UNKNOWN, target->sourceMeta,
										   "Duplicate symbol import");
			}

			target->referents.push_back(s);
		}
	}

	if (target->referents.empty()) throw UnresolvedSymbolException(target->id);
}

void ImportHandler::getPossibleBaseDirs(bool declaredRelative,
										List<std::filesystem::path>& dest) {
	dest.push_back(mod->moduleInfo.dir);
	if (declaredRelative) {
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
		if (m->moduleInfo.path == path) return m->ast;
	}

	return compileImport(path, meta);
}

static acl::String getModuleDir(const std::filesystem::path& path) {
	return path.parent_path();
}

static acl::String getModuleName(const std::filesystem::path& path) {
	return path.stem();
}

static acl::ModuleInfo getModuleInfo(const std::filesystem::path& path) {
	auto p = std::filesystem::absolute(path);
	return {getModuleDir(p), p, getModuleName(p)};
}

Ast* ImportHandler::compileImport(const std::filesystem::path& path,
								  const SourceMeta& meta) {
	std::ifstream ifs(path);

	if (ifs) {
		acl::StringBuffer sb;
		sb << ifs.rdbuf();
		ifs.close();

		acl::String str = sb.str();

		acl::StringBuffer lexerBuf;
		lexerBuf << str;

		// TODO: A .accele file and a .acldef file should be parsed and resolved
		// differently. Also, .acldef files cannot be translated to C++ source
		// or OBJ files. They can only be used to reference a library.

		auto moduleInfo = getModuleInfo(path);

		acl::StringBuffer linesBuf;
		linesBuf << str;

		acl::List<acl::String> lines;
		while (linesBuf) {
			acl::String line;
			std::getline(linesBuf, line);
			lines.push_back(line);
		}

		auto m = new acl::Module{moduleInfo, nullptr, lines};

		ctx.modules.push_back(m);

		acl::Parser parser =
			acl::Parser(ctx, acl::Lexer(ctx, m->moduleInfo, lexerBuf));
		auto ast = parser.parse();

		m->ast = ast;

		Resolver resolver = Resolver(ctx, m, ResolutionStage::INTERNAL_ALL);
		resolver.resolve();

		return ast;
	} else
		throw UnresolvedImportException(ASP_CORE_UNKNOWN, meta,
										"Unresolved import");
}
}  // namespace acl