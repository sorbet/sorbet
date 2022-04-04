#ifndef AUTOGEN_CONSTANT_HASH_H
#define AUTOGEN_CONSTANT_HASH_H

#include <string>
#include <vector>

#include "core/GlobalState.h"

namespace sorbet::parser {
class Node;
} // namespace sorbet::parser

namespace sorbet::autogen {

// This computes a "constant hash", i.e. a hash entirely of the
// constant structure of the file. This hash is specifically defined
// with respect to "what could cause an autogen run to produce
// different output", and might accordingly change: in particular,
// right now it cares about "require" statements because they are
// important to the autoloader output, but we might remove that if we
// change the structure of the autoloader. Changes to method bodies,
// class bodies that don't add or remove other constant definitions,
// and so forth should produce an identical constant hash.
//
// The end-goal is that if a file produces the same constant hash,
// then we do not need to re-run autogen-related logic on that file,
// because no changes have happened which could possibly affect what
// autogen produces.
unsigned int constantHashNode(core::GlobalState &gs, const std::unique_ptr<parser::Node> &what);

} // namespace sorbet::autogen
#endif // AUTOGEN_CONSTANT_HASH_H
