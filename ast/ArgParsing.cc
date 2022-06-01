#include "ast/ArgParsing.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/hashing/hashing.h"

using namespace std;

namespace sorbet::ast {

namespace {
core::ParsedArg parseArg(const ast::ExpressionPtr &arg) {
    core::ParsedArg parsedArg;

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

vector<core::ParsedArg> ArgParsing::parseArgs(const ast::MethodDef::ARGS_store &args) {
    vector<core::ParsedArg> parsedArgs;
    for (auto &arg : args) {
        if (!ast::isa_reference(arg)) {
            Exception::raise("Must be a reference!");
        }
        parsedArgs.emplace_back(parseArg(arg));
    }

    return parsedArgs;
}

std::vector<uint32_t> ArgParsing::hashArgs(core::Context ctx, const std::vector<core::ParsedArg> &args) {
    std::vector<uint32_t> result;
    result.reserve(args.size());
    for (const auto &e : args) {
        uint32_t arg = 0;
        if (e.flags.isKeyword) {
            if (e.flags.isRepeated && e.local._name != core::Names::fwdKwargs()) {
                auto name = core::Names::kwargs();
                arg = core::mix(arg, core::_hash(name.shortName(ctx)));
            } else {
                arg = core::mix(arg, core::_hash(e.local._name.shortName(ctx)));
            }
        }

        result.push_back(core::mix(arg, e.flags.toU1()));
    }
    return result;
}

ExpressionPtr ArgParsing::getDefault(const core::ParsedArg &parsedArg, ExpressionPtr arg) {
    if (!parsedArg.flags.isDefault) {
        return nullptr;
    }
    return getDefaultValue(move(arg));
}

} // namespace sorbet::ast
