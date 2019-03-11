#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "lsp.h"

using namespace std;

namespace sorbet::realmain::lsp {

string LSPLoop::remoteName2Local(string_view uri) {
    ENFORCE(absl::StartsWith(uri, rootUri));
    const char *start = uri.data() + rootUri.length();
    if (*start == '/') {
        ++start;
    }
    return string(start, uri.end());
}

string LSPLoop::localName2Remote(string_view uri) {
    ENFORCE(!absl::StartsWith(uri, rootUri));
    return absl::StrCat(rootUri, "/", uri);
}

core::FileRef LSPLoop::uri2FileRef(string_view uri) {
    if (!absl::StartsWith(uri, rootUri)) {
        return core::FileRef();
    }
    auto needle = remoteName2Local(uri);
    return initialGS->findFileByPath(needle);
}

string LSPLoop::fileRef2Uri(const core::GlobalState &gs, core::FileRef file) {
    if (file.data(gs).sourceType == core::File::Type::Payload) {
        return string(file.data(gs).path());
    } else {
        return localName2Remote(string(file.data(gs).path()));
    }
}
unique_ptr<Range> loc2Range(const core::GlobalState &gs, core::Loc loc) {
    unique_ptr<Position> start;
    unique_ptr<Position> end;
    if (!loc.file().exists()) {
        start = make_unique<Position>(1, 1);
        end = make_unique<Position>(2, 0);
    } else {
        auto pair = loc.position(gs);
        // All LSP numbers are zero-based, ours are 1-based.
        start = make_unique<Position>(pair.first.line - 1, pair.first.column - 1);
        end = make_unique<Position>(pair.second.line - 1, pair.second.column - 1);
    }
    return make_unique<Range>(move(start), move(end));
}

unique_ptr<Location> LSPLoop::loc2Location(const core::GlobalState &gs, core::Loc loc) {
    string uri;
    if (!loc.file().exists()) {
        uri = localName2Remote("???");
    } else {
        auto &messageFile = loc.file().data(gs);
        if (messageFile.sourceType == core::File::Type::Payload) {
            // This is hacky because VSCode appends #4,3 (or whatever the position is of the
            // error) to the uri before it shows it in the UI since this is the format that
            // VSCode uses to denote which location to jump to. However, if you append #L4
            // to the end of the uri, this will work on github (it will ignore the #4,3)
            //
            // As an example, in VSCode, on hover you might see
            //
            // string.rbi(18,7): Method `+` has specified type of argument `arg0` as `String`
            //
            // When you click on the link, in the browser it appears as
            // https://git.corp.stripe.com/stripe-internal/ruby-typer/tree/master/rbi/core/string.rbi#L18%2318,7
            // but shows you the same thing as
            // https://git.corp.stripe.com/stripe-internal/ruby-typer/tree/master/rbi/core/string.rbi#L18
            uri = fmt::format("{}#L{}", messageFile.path(), loc.position(gs).first.line);
        } else {
            uri = fileRef2Uri(gs, loc.file());
        }
    }
    return make_unique<Location>(uri, loc2Range(gs, loc));
}
bool hideSymbol(const core::GlobalState &gs, core::SymbolRef sym) {
    if (!sym.exists() || sym == core::Symbols::root()) {
        return true;
    }
    auto data = sym.data(gs);
    if (data->isClass() && data->attachedClass(gs).exists()) {
        return true;
    }
    if (data->isClass() && data->superClass() == core::Symbols::StubClass()) {
        return true;
    }
    if (data->isMethodArgument() && data->isBlockArgument()) {
        return true;
    }
    if (data->name.data(gs)->kind == core::NameKind::UNIQUE &&
        data->name.data(gs)->unique.original == core::Names::staticInit()) {
        return true;
    }
    if (data->name.data(gs)->kind == core::NameKind::UNIQUE &&
        data->name.data(gs)->unique.original == core::Names::blockTemp()) {
        return true;
    }
    return false;
}

bool hasSimilarName(const core::GlobalState &gs, core::NameRef name, string_view pattern) {
    string_view view = name.data(gs)->shortName(gs);
    auto fnd = view.find(pattern);
    return fnd != string_view::npos;
}

string methodDetail(const core::GlobalState &gs, core::SymbolRef method, core::TypePtr receiver, core::TypePtr retType,
                    shared_ptr<core::TypeConstraint> constraint) {
    ENFORCE(method.exists());
    // handle this case anyways so that we don't crash in prod when this method is mis-used
    if (!method.exists()) {
        return "";
    }

    if (!retType) {
        retType = getResultType(gs, method, receiver, constraint);
    }
    string methodReturnType =
        (retType == core::Types::void_()) ? "void" : absl::StrCat("returns(", retType->show(gs), ")");
    vector<string> typeAndArgNames;

    if (method.data(gs)->isMethod()) {
        for (auto &argSym : method.data(gs)->arguments()) {
            typeAndArgNames.emplace_back(absl::StrCat(argSym.data(gs)->argumentName(gs), ": ",
                                                      getResultType(gs, argSym, receiver, constraint)->show(gs)));
        }
    }

    return fmt::format("sig {{params({}).{}}}", fmt::join(typeAndArgNames, ", "), methodReturnType);
}

core::TypePtr getResultType(const core::GlobalState &gs, core::SymbolRef ofWhat, core::TypePtr receiver,
                            shared_ptr<core::TypeConstraint> constr) {
    core::Context ctx(gs, core::Symbols::root());
    auto resultType = ofWhat.data(gs)->resultType;
    if (auto *proxy = core::cast_type<core::ProxyType>(receiver.get())) {
        receiver = proxy->underlying();
    }
    if (auto *applied = core::cast_type<core::AppliedType>(receiver.get())) {
        /* instantiate generic classes */
        resultType = core::Types::resultTypeAsSeenFrom(ctx, ofWhat, applied->klass, applied->targs);
    }
    if (!resultType) {
        resultType = core::Types::untypedUntracked();
    }

    resultType = core::Types::replaceSelfType(ctx, resultType, receiver); // instantiate self types
    if (constr) {
        resultType = core::Types::instantiate(ctx, resultType, *constr); // instantiate generic methods
    }
    return resultType;
}

SymbolKind symbolRef2SymbolKind(const core::GlobalState &gs, core::SymbolRef symbol) {
    auto sym = symbol.data(gs);
    if (sym->isClass()) {
        if (sym->isClassModule()) {
            return SymbolKind::Module;
        }
        if (sym->isClassClass()) {
            return SymbolKind::Class;
        }
    } else if (sym->isMethod()) {
        if (sym->name == core::Names::initialize()) {
            return SymbolKind::Constructor;
        } else {
            return SymbolKind::Method;
        }
    } else if (sym->isField()) {
        return SymbolKind::Field;
    } else if (sym->isStaticField()) {
        return SymbolKind::Constant;
    } else if (sym->isMethodArgument()) {
        return SymbolKind::Variable;
    } else if (sym->isTypeMember()) {
        return SymbolKind::TypeParameter;
    } else if (sym->isTypeArgument()) {
        return SymbolKind::TypeParameter;
    }
    return SymbolKind::Unknown;
}
} // namespace sorbet::realmain::lsp
