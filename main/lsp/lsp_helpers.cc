#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "lsp.h"

using namespace std;

namespace sorbet {
namespace realmain {
namespace lsp {

std::string LSPLoop::remoteName2Local(const absl::string_view uri) {
    ENFORCE(absl::StartsWith(uri, rootUri));
    const char *start = uri.data() + rootUri.length();
    if (*start == '/') {
        ++start;
    }
    return string(start, uri.end());
}

std::string LSPLoop::localName2Remote(const absl::string_view uri) {
    ENFORCE(!absl::StartsWith(uri, rootUri));
    return absl::StrCat(rootUri, "/", uri);
}

core::FileRef LSPLoop::uri2FileRef(const absl::string_view uri) {
    if (!absl::StartsWith(uri, rootUri)) {
        return core::FileRef();
    }
    auto needle = remoteName2Local(uri);
    return initialGS->findFileByPath(needle);
}

std::string LSPLoop::fileRef2Uri(core::FileRef file) {
    if (file.data(*finalGs).sourceType == core::File::Type::Payload) {
        return (string)file.data(*finalGs).path();
    } else {
        return localName2Remote((string)file.data(*finalGs).path());
    }
}
rapidjson::Value LSPLoop::loc2Range(core::Loc loc) {
    /**
       {
        start: { line: 5, character: 23 }
        end : { line 6, character : 0 }
        }
     */
    rapidjson::Value ret;
    ret.SetObject();
    rapidjson::Value start;
    start.SetObject();
    rapidjson::Value end;
    end.SetObject();
    if (!loc.file().exists()) {
        start.AddMember("line", 1, alloc);
        start.AddMember("character", 1, alloc);
        end.AddMember("line", 2, alloc);
        end.AddMember("character", 0, alloc);

        ret.AddMember("start", start, alloc);
        ret.AddMember("end", end, alloc);
        return ret;
    }

    auto pair = loc.position(*finalGs);
    // All LSP numbers are zero-based, ours are 1-based.
    start.AddMember("line", pair.first.line - 1, alloc);
    start.AddMember("character", pair.first.column - 1, alloc);
    end.AddMember("line", pair.second.line - 1, alloc);
    end.AddMember("character", pair.second.column - 1, alloc);

    ret.AddMember("start", start, alloc);
    ret.AddMember("end", end, alloc);
    return ret;
}

rapidjson::Value LSPLoop::loc2Location(core::Loc loc) {
    string uri;
    //  interface Location {
    //      uri: DocumentUri;
    //      range: Range;
    //  }
    rapidjson::Value ret;
    ret.SetObject();

    if (!loc.file().exists()) {
        uri = localName2Remote("???");
    } else {
        auto &messageFile = loc.file().data(*finalGs);
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
            uri = fmt::format("{}#L{}", (string)messageFile.path(),
                              std::to_string((int)(loc.position(*finalGs).first.line)));
        } else {
            uri = fileRef2Uri(loc.file());
        }
    }

    ret.AddMember("uri", uri, alloc);
    ret.AddMember("range", loc2Range(loc), alloc);

    return ret;
}
bool LSPLoop::hideSymbol(core::SymbolRef sym) {
    if (!sym.exists() || sym == core::Symbols::root()) {
        return true;
    }
    auto &data = sym.data(*finalGs);
    if (data.isClass() && data.attachedClass(*finalGs).exists()) {
        return true;
    }
    if (data.isClass() && data.superClass == core::Symbols::StubClass()) {
        return true;
    }
    if (data.isMethodArgument() && data.isBlockArgument()) {
        return true;
    }
    if (data.name.data(*finalGs).kind == core::NameKind::UNIQUE &&
        data.name.data(*finalGs).unique.original == core::Names::staticInit()) {
        return true;
    }
    if (data.name.data(*finalGs).kind == core::NameKind::UNIQUE &&
        data.name.data(*finalGs).unique.original == core::Names::blockTemp()) {
        return true;
    }
    return false;
}

bool LSPLoop::hasSimilarName(core::GlobalState &gs, core::NameRef name, const absl::string_view &pattern) {
    absl::string_view view = name.data(gs).shortName(gs);
    auto fnd = view.find(pattern);
    return fnd != absl::string_view::npos;
}

string LSPLoop::methodDetail(core::SymbolRef method, shared_ptr<core::Type> receiver, shared_ptr<core::Type> retType,
                             shared_ptr<core::TypeConstraint> constraint) {
    ENFORCE(method.exists());
    // handle this case anyways so that we don't crash in prod when this method is mis-used
    if (!method.exists()) {
        return "";
    }

    string ret;
    if (!retType) {
        retType = getResultType(method, receiver, constraint);
    }
    string methodReturnType = (retType == core::Types::void_()) ? "void" : "returns(" + retType->show(*finalGs) + ")";
    std::vector<string> typeAndArgNames;

    if (method.data(*finalGs).isMethod()) {
        for (auto &argSym : method.data(*finalGs).arguments()) {
            typeAndArgNames.push_back(argSym.data(*finalGs).name.show(*finalGs) + ": " +
                                      getResultType(argSym, receiver, constraint)->show(*finalGs));
        }
    }

    std::stringstream ss;
    for (size_t i = 0; i < typeAndArgNames.size(); ++i) {
        if (i != 0) {
            ss << ", ";
        }
        ss << typeAndArgNames[i];
    }
    std::string joinedTypeAndArgNames = ss.str();

    return fmt::format("sig({}).{}", joinedTypeAndArgNames, methodReturnType);
}

shared_ptr<core::Type> LSPLoop::getResultType(core::SymbolRef ofWhat, shared_ptr<core::Type> receiver,
                                              shared_ptr<core::TypeConstraint> constr) {
    core::Context ctx(*finalGs, core::Symbols::root());
    auto resultType = ofWhat.data(*finalGs).resultType;
    if (auto *proxy = core::cast_type<core::ProxyType>(receiver.get())) {
        receiver = proxy->underlying;
    }
    if (auto *applied = core::cast_type<core::AppliedType>(receiver.get())) {
        /* instantiate generic classes */
        resultType = core::Types::resultTypeAsSeenFrom(ctx, ofWhat, applied->klass, applied->targs);
    }
    if (!resultType) {
        resultType = core::Types::untyped();
    }

    resultType = core::Types::replaceSelfType(ctx, resultType, receiver); // instantiate self types
    if (constr) {
        resultType = core::Types::instantiate(ctx, resultType, *constr); // instantiate generic methods
    }
    return resultType;
}

int LSPLoop::symbolRef2SymbolKind(core::SymbolRef symbol) {
    auto &sym = symbol.data(*finalGs);
    /**
     * A symbol kind.
     *
     *      export namespace SymbolKind {
     *          export const File = 1;
     *          export const Module = 2;
     *          export const Namespace = 3;
     *          export const Package = 4;
     *          export const Class = 5;
     *          export const Method = 6;
     *          export const Property = 7;
     *          export const Field = 8;
     *          export const Constructor = 9;
     *          export const Enum = 10;
     *          export const Interface = 11;
     *          export const Function = 12;
     *          export const Variable = 13;
     *          export const Constant = 14;
     *          export const String = 15;
     *          export const Number = 16;
     *          export const Boolean = 17;
     *          export const Array = 18;
     *          export const Object = 19;
     *          export const Key = 20;
     *          export const Null = 21;
     *          export const EnumMember = 22;
     *          export const Struct = 23;
     *          export const Event = 24;
     *          export const Operator = 25;
     *          export const TypeParameter = 26;
     *      }
     **/
    if (sym.isClass()) {
        if (sym.isClassModule()) {
            return 2;
        }
        if (sym.isClassClass()) {
            return 5;
        }
    } else if (sym.isMethod()) {
        if (sym.name == core::Names::initialize()) {
            return 9;
        } else {
            return 6;
        }
    } else if (sym.isField()) {
        return 8;
    } else if (sym.isStaticField()) {
        return 14;
    } else if (sym.isMethodArgument()) {
        return 13;
    } else if (sym.isTypeMember()) {
        return 26;
    } else if (sym.isTypeArgument()) {
        return 26;
    }
    return 0;
}

} // namespace lsp
} // namespace realmain
} // namespace sorbet