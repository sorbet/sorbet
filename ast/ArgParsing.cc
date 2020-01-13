#include "ast/ArgParsing.h"
#include "common/typecase.h"
#include "core/Context.h"
#include "core/Hashing.h"

using namespace std;

namespace sorbet::ast {

ParsedArg ArgParsing::parseArg(core::Context ctx, unique_ptr<ast::Reference> arg) {
    ParsedArg parsedArg;

    typecase(
        arg.get(), [&](ast::UnresolvedIdent *nm) { Exception::raise("Unexpected unresolved name in arg!"); },
        [&](ast::RestArg *rest) {
            parsedArg = parseArg(ctx, move(rest->expr));
            parsedArg.repeated = true;
        },
        [&](ast::KeywordArg *kw) {
            parsedArg = parseArg(ctx, move(kw->expr));
            parsedArg.keyword = true;
        },
        [&](ast::OptionalArg *opt) {
            parsedArg = parseArg(ctx, move(opt->expr));
            parsedArg.default_ = move(opt->default_);
        },
        [&](ast::BlockArg *blk) {
            parsedArg = parseArg(ctx, move(blk->expr));
            parsedArg.block = true;
        },
        [&](ast::ShadowArg *shadow) {
            parsedArg = parseArg(ctx, move(shadow->expr));
            parsedArg.shadow = true;
        },
        [&](ast::Local *local) {
            parsedArg.local = local->localVariable;
            parsedArg.loc = local->loc;
        });

    return parsedArg;
}

vector<ParsedArg> ArgParsing::parseArgs(core::Context ctx, ast::MethodDef::ARGS_store &args) {
    vector<ParsedArg> parsedArgs;
    for (auto &arg : args) {
        auto *refExp = ast::cast_tree<ast::Reference>(arg.get());
        if (!refExp) {
            Exception::raise("Must be a reference!");
        }
        unique_ptr<ast::Reference> refExpImpl(refExp);
        arg.release();
        parsedArgs.emplace_back(parseArg(ctx, move(refExpImpl)));
    }

    return parsedArgs;
}

std::vector<u4> ArgParsing::hashArgs(core::Context ctx, std::vector<ParsedArg> &args) {
    std::vector<u4> result;
    result.reserve(args.size());
    for (const auto &e : args) {
        u4 arg = 0;
        u1 flags = 0;
        if (e.keyword) {
            arg = core::mix(arg, core::_hash(e.local._name.data(ctx)->shortName(ctx)));
            flags += 1;
        }
        if (e.repeated) {
            flags += 2;
        }
        if (e.default_) {
            flags += 4;
        }
        if (e.shadow) {
            flags += 8;
        }
        if (e.block) {
            flags += 16;
        }

        result.push_back(core::mix(arg, flags));
    }
    return result;
}

} // namespace sorbet::ast
