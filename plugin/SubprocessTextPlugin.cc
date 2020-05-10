#include "plugin/SubprocessTextPlugin.h"
#include "absl/strings/str_replace.h"
#include "ast/treemap/treemap.h"
#include "common/Subprocess.h"
#include "core/errors/plugin.h"

using namespace std;

namespace sorbet::plugin {

struct Namespace {
    enum class Type { Class, Module };
    Type type;
    InlinedVector<core::NameRef, 3> components;

    Namespace(ast::TreePtr &tree) {
        auto *klass = ast::cast_tree<ast::ClassDef>(tree);
        type = (klass->kind == ast::ClassDef::Kind::Module ? Namespace::Type::Module : Namespace::Type::Class);
        fillComponents(klass->name);
    }
    ~Namespace() = default;
    Namespace(Namespace &&) = default;

private:
    void fillComponents(ast::TreePtr &constant) {
        auto *cursor = &constant;
        while (cursor) {
            if (auto unresolved = ast::cast_tree<ast::UnresolvedConstantLit>(*cursor)) {
                components.push_back(unresolved->cnst);
                cursor = &unresolved->scope;
            } else if (auto ident = ast::cast_tree<ast::UnresolvedIdent>(*cursor)) {
                ENFORCE(ident->name == core::Names::singleton());
                components.push_back(core::Names::singleton());
                break;
            } else if (auto constLit = ast::cast_tree<ast::ConstantLit>(*cursor)) {
                ENFORCE(constLit->symbol == core::Symbols::root());
                components.push_back(core::Names::Constants::Root());
                break;
            } else {
                break;
            }
        }
    }
};

struct SpawningWalker {
    vector<shared_ptr<core::File>> subprocessResults;
    InlinedVector<Namespace, 5> nesting;

    SpawningWalker() {}

    ast::TreePtr preTransformClassDef(core::Context ctx, ast::TreePtr tree) {
        auto *klass = ast::cast_tree<ast::ClassDef>(tree);
        if (klass->symbol == core::Symbols::root()) {
            return tree;
        }
        nesting.emplace_back(tree);
        for (auto &statement : klass->rhs) {
            auto send = ast::cast_tree<ast::Send>(statement);
            if (!send) {
                continue;
            }
            auto command = ctx.state.findDslPlugin(send->fun);
            if (!command) {
                continue;
            }

            optional<string> output;
            {
                string className = core::Loc(ctx.file, klass->name->loc).source(ctx);
                string_view shortName = send->fun.data(ctx)->shortName(ctx);
                string sendSource = core::Loc(ctx.file, send->loc).source(ctx);

                vector<string> args(ctx.state.dslRubyExtraArgs);
                args.emplace_back(*command);
                args.emplace_back("--class");
                args.emplace_back(move(className));
                args.emplace_back("--method");
                args.emplace_back(shortName);
                args.emplace_back("--source");
                args.emplace_back(move(sendSource));

                output = Subprocess::spawn("ruby", move(args));
            }

            if (output) {
                fmt::memory_buffer generatedSource;
                for (auto &n : nesting) {
                    if (!n.components.empty() && n.components.back() == core::Names::singleton()) {
                        format_to(generatedSource, "{}", "class << self;");
                    } else {
                        bool first = true;
                        format_to(generatedSource, "{}", (n.type == Namespace::Type::Class ? "class " : "module "));
                        for (auto it = n.components.rbegin(); it != n.components.rend(); it++) {
                            if (!first) {
                                format_to(generatedSource, "{}", "::");
                            }
                            first = false;
                            if (auto &name = *it; name != core::Names::Constants::Root()) {
                                format_to(generatedSource, "{}", name.data(ctx)->shortName(ctx));
                            }
                        }
                        format_to(generatedSource, "{}", ';');
                    }
                }
                format_to(generatedSource, "\n{}", *output);
                for (int i = 0; i < nesting.size(); i++) {
                    format_to(generatedSource, "{}", "end;");
                }

                auto path =
                    fmt::format("{}//plugin-generated|{}.rbi", ctx.file.data(ctx).path(), subprocessResults.size());
                auto file =
                    make_shared<core::File>(move(path), fmt::to_string(generatedSource), core::File::Type::Normal);
                file->pluginGenerated = true;
                subprocessResults.emplace_back(move(file));
            } else {
                if (auto e = ctx.beginError(send->loc, core::errors::Plugin::SubProcessError)) {
                    e.setHeader("Error while executing subprocess plugin `{}`", *command);
                }
            }
        }
        return tree;
    }

    ast::TreePtr postTransformClassDef(core::Context ctx, ast::TreePtr klass) {
        if (ast::cast_tree<ast::ClassDef>(klass)->symbol != core::Symbols::root()) {
            nesting.pop_back();
        }
        return klass;
    }
};

pair<ast::TreePtr, vector<shared_ptr<core::File>>> SubprocessTextPlugin::run(core::Context ctx, ast::TreePtr tree) {
    if (!ctx.state.hasAnyDslPlugin()) {
        vector<shared_ptr<core::File>> empty;
        return {move(tree), empty};
    }
    SpawningWalker walker;
    return {ast::TreeMap::apply(ctx, walker, move(tree)), move(walker.subprocessResults)};
}

}; // namespace sorbet::plugin
