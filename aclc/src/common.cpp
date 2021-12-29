#include "common.hpp"

#include "exceptions.hpp"

namespace acl {
CompilerContext::CompilerContext() {
	warnings[ASP_SOURCE_LOCK_FRONTING] = true;
}
}  // namespace acl