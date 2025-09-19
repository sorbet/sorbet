#include "ast/ParamParsing.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/hashing/hashing.h"

using namespace std;

namespace sorbet::ast {

namespace {
core::ParsedParam parseParam(const ast::ExpressionPtr &param) {
    core::ParsedParam parsedParam;
    auto *cursor = &param;

    while (cursor != nullptr) {
        typecase(
            *cursor,
            [&](const ast::RestParam &rest) {
                parsedParam.flags.isRepeated = true;
                cursor = &rest.expr;
            },
            [&](const ast::KeywordArg &kw) {
                parsedParam.flags.isKeyword = true;
                cursor = &kw.expr;
            },
            [&](const ast::OptionalParam &opt) {
                parsedParam.flags.isDefault = true;
                cursor = &opt.expr;
            },
            [&](const ast::BlockParam &blk) {
                parsedParam.flags.isBlock = true;
                cursor = &blk.expr;
            },
            [&](const ast::ShadowArg &shadow) {
                parsedParam.flags.isShadow = true;
                cursor = &shadow.expr;
            },
            [&](const ast::Local &local) {
                parsedParam.local = local.localVariable;
                parsedParam.loc = local.loc;
                cursor = nullptr;
            });
    }

    return parsedParam;
}

ExpressionPtr getDefaultValue(ExpressionPtr param) {
    auto *cursor = &param;
    bool done = false;
    while (!done) {
        typecase(
            *cursor, [&](ast::RestParam &rest) { cursor = &rest.expr; },
            [&](ast::KeywordArg &kw) { cursor = &kw.expr; },
            [&](ast::OptionalParam &opt) {
                cursor = &opt.default_;
                done = true;
            },
            [&](ast::BlockParam &blk) { cursor = &blk.expr; }, [&](ast::ShadowArg &shadow) { cursor = &shadow.expr; },
            [&](ast::Local &local) {
                ENFORCE(false, "shouldn't reach a local variable for arg");
                done = true;
                // No default.
            });
    }
    ENFORCE(cursor != &param);
    return std::move(*cursor);
}

} // namespace

vector<core::ParsedParam> ParamParsing::parseParams(const ast::MethodDef::PARAMS_store &params) {
    vector<core::ParsedParam> parsedParams;
    for (auto &param : params) {
        if (!ast::isa_reference(param)) {
            Exception::raise("Must be a reference!");
        }
        parsedParams.emplace_back(parseParam(param));
    }

    return parsedParams;
}

// This has to match the implementation of Method::methodArityHash
core::ArityHash ParamParsing::hashParams(core::Context ctx, const vector<core::ParsedParam> &params) {
    uint32_t result = 0;
    result = core::mix(result, params.size());
    for (const auto &e : params) {
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

ExpressionPtr ParamParsing::getDefault(const core::ParsedParam &parsedArg, ExpressionPtr arg) {
    if (!parsedArg.flags.isDefault) {
        return nullptr;
    }
    return getDefaultValue(move(arg));
}

} // namespace sorbet::ast
