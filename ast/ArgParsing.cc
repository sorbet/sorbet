#include "ast/ArgParsing.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/hashing/hashing.h"

using namespace std;

namespace sorbet::ast {

namespace {
core::ParsedArg parseArg(const ast::ExpressionPtr &arg) {
    core::ParsedArg parsedArg;
    auto *cursor = &arg;

    while (cursor != nullptr) {
        typecase(
            *cursor,
            [&](const ast::RestArg &rest) {
                parsedArg.flags.isRepeated = true;
                cursor = &rest.expr;
            },
            [&](const ast::KeywordArg &kw) {
                parsedArg.flags.isKeyword = true;
                cursor = &kw.expr;
            },
            [&](const ast::OptionalArg &opt) {
                parsedArg.flags.isDefault = true;
                cursor = &opt.expr;
            },
            [&](const ast::BlockArg &blk) {
                parsedArg.flags.isBlock = true;
                cursor = &blk.expr;
            },
            [&](const ast::ShadowArg &shadow) {
                parsedArg.flags.isShadow = true;
                cursor = &shadow.expr;
            },
            [&](const ast::Local &local) {
                parsedArg.local = local.localVariable;
                parsedArg.loc = local.loc;
                cursor = nullptr;
            });
    }

    return parsedArg;
}

ExpressionPtr getDefaultValue(ExpressionPtr arg) {
    auto *cursor = &arg;
    bool done = false;
    while (!done) {
        typecase(
            *cursor, [&](ast::RestArg &rest) { cursor = &rest.expr; }, [&](ast::KeywordArg &kw) { cursor = &kw.expr; },
            [&](ast::OptionalArg &opt) {
                cursor = &opt.default_;
                done = true;
            },
            [&](ast::BlockArg &blk) { cursor = &blk.expr; }, [&](ast::ShadowArg &shadow) { cursor = &shadow.expr; },
            [&](ast::Local &local) {
                ENFORCE(false, "shouldn't reach a local variable for arg");
                done = true;
                // No default.
            });
    }
    ENFORCE(cursor != &arg);
    return std::move(*cursor);
}

} // namespace

vector<core::ParsedArg> ArgParsing::parseArgs(const ast::MethodDef::PARAMS_store &args) {
    vector<core::ParsedArg> parsedArgs;
    for (auto &arg : args) {
        if (!ast::isa_reference(arg)) {
            Exception::raise("Must be a reference!");
        }
        parsedArgs.emplace_back(parseArg(arg));
    }

    return parsedArgs;
}

// This has to match the implementation of Method::methodArityHash
core::ArityHash ArgParsing::hashArgs(core::Context ctx, const vector<core::ParsedArg> &args) {
    uint32_t result = 0;
    result = core::mix(result, args.size());
    for (const auto &e : args) {
        if (e.flags.isKeyword) {
            if (e.flags.isRepeated && e.local._name != core::Names::fwdKwargs()) {
                auto name = core::Names::kwargs();
                result = core::mix(result, core::_hash(name.shortName(ctx)));
            } else {
                result = core::mix(result, core::_hash(e.local._name.shortName(ctx)));
            }
        }

        result = core::mix(result, e.flags.toU1());
    }
    return core::ArityHash(result);
}

ExpressionPtr ArgParsing::getDefault(const core::ParsedArg &parsedArg, ExpressionPtr arg) {
    if (!parsedArg.flags.isDefault) {
        return nullptr;
    }
    return getDefaultValue(move(arg));
}

} // namespace sorbet::ast
