#include "common.hpp"

#include "ast.hpp"
#include "diagnoser.hpp"

namespace acl {
CompilerContext::CompilerContext() {
	warnings[ec::NONFRONTED_SOURCE_LOCK] = true;
}

Module::Module(const ModuleInfo& moduleInfo, Ast* ast,
			   const List<String>& source)
	: moduleInfo(moduleInfo), ast(ast), source(source) {}

Module::~Module() { /*delete ast;*/
}
}  // namespace acl