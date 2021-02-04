#include "ast/ArgParsing.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/hashing/hashing.h"

using namespace std;

namespace sorbet::ast {

namespace {
ParsedArg parseArg(const ast::ExpressionPtr &arg) {
    ParsedArg parsedArg;

    typecase(
        arg,
        [&](const ast::RestArg &rest) {
            parsedArg = parseArg(rest.expr);
            parsedArg.flags.isRepeated = true;
        },
        [&](const ast::KeywordArg &kw) {
            parsedArg = parseArg(kw.expr);
            parsedArg.flags.isKeyword = true;
        },
        [&](const ast::OptionalArg &opt) {
            parsedArg = parseArg(opt.expr);
            parsedArg.flags.isDefault = true;
        },
        [&](const ast::BlockArg &blk) {
            parsedArg = parseArg(blk.expr);
            parsedArg.flags.isBlock = true;
        },
        [&](const ast::ShadowArg &shadow) {
            parsedArg = parseArg(shadow.expr);
            parsedArg.flags.isShadow = true;
        },
        [&](const ast::Local &local) {
            parsedArg.local = local.localVariable;
            parsedArg.loc = local.loc;
        });

    return parsedArg;
}

ExpressionPtr getDefaultValue(ExpressionPtr arg) {
    ExpressionPtr default_;
    typecase(
        arg, [&](ast::RestArg &rest) { default_ = getDefaultValue(move(rest.expr)); },
        [&](ast::KeywordArg &kw) { default_ = getDefaultValue(move(kw.expr)); },
        [&](ast::OptionalArg &opt) { default_ = move(opt.default_); },
        [&](ast::BlockArg &blk) { default_ = getDefaultValue(move(blk.expr)); },
        [&](ast::ShadowArg &shadow) { default_ = getDefaultValue(move(shadow.expr)); },
        [&](ast::Local &local) {
            // No default.
        });
    ENFORCE(default_ != nullptr);
    return default_;
}

} // namespace

vector<ParsedArg> ArgParsing::parseArgs(const ast::MethodDef::ARGS_store &args) {
    vector<ParsedArg> parsedArgs;
    for (auto &arg : args) {
        if (!ast::isa_reference(arg)) {
            Exception::raise("Must be a reference!");
        }
        parsedArgs.emplace_back(parseArg(arg));
    }

    return parsedArgs;
}

std::vector<u4> ArgParsing::hashArgs(core::Context ctx, const std::vector<ParsedArg> &args) {
    std::vector<u4> result;
    result.reserve(args.size());
    core::Hasher hasher;
    for (const auto &e : args) {
        u1 flags = 0;
        if (e.flags.isKeyword) {
            hasher.mixString(e.local._name.shortName(ctx));
            flags += 1;
        }
        if (e.flags.isRepeated) {
            flags += 2;
        }
        if (e.flags.isDefault) {
            flags += 4;
        }
        if (e.flags.isShadow) {
            flags += 8;
        }
        if (e.flags.isBlock) {
            flags += 16;
        }

        hasher.mixUint(flags);

        result.push_back(hasher.digest());
        hasher.reset();
    }
    return result;
}

ExpressionPtr ArgParsing::getDefault(const ParsedArg &parsedArg, ExpressionPtr arg) {
    if (!parsedArg.flags.isDefault) {
        return nullptr;
    }
    return getDefaultValue(move(arg));
}

} // namespace sorbet::ast
