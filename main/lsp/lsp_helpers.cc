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
    // Special case: Folder is '' (current directory).
    if (rootPath.length() > 0) {
        return absl::StrCat(rootPath, "/", string(start, uri.end()));
    } else {
        return string(start, uri.end());
    }
}

string LSPLoop::localName2Remote(string_view uri) {
    ENFORCE(absl::StartsWith(uri, rootPath));
    string_view relativeUri = uri.substr(rootPath.length());
    if (relativeUri.at(0) == '/') {
        relativeUri = relativeUri.substr(1);
    }

    // Special case: Root uri is '' (happens in Monaco)
    if (rootUri.length() == 0) {
        return string(relativeUri);
    }

    return absl::StrCat(rootUri, "/", relativeUri);
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

int cmpPositions(const Position &a, const Position &b) {
    const int line = a.line - b.line;
    if (line != 0) {
        return line;
    }
    return a.character - b.character;
}

int cmpLocations(const Location &a, const Location &b) {
    const int fileCmp = a.uri.compare(b.uri);
    if (fileCmp != 0) {
        return fileCmp;
    }
    const int startCmp = cmpPositions(*a.range->start, *b.range->start);
    if (startCmp != 0) {
        return startCmp;
    }
    return cmpPositions(*a.range->end, *b.range->end);
}

bool cmpRanges(const Range &a, const Range &b) {
    return a.start->character == b.start->character && a.start->line == b.start->line &&
           a.end->character == b.end->character && a.end->line == b.end->line;
}

vector<unique_ptr<Location>>
LSPLoop::extractLocations(const core::GlobalState &gs,
                          const vector<unique_ptr<core::lsp::QueryResponse>> &queryResponses,
                          vector<unique_ptr<Location>> locations) {
    for (auto &q : queryResponses) {
        core::Loc loc = q->getLoc();
        if (loc.exists() && loc.file().exists()) {
            auto fileIsTyped = loc.file().data(gs).strictLevel >= core::StrictLevel::True;
            // If file is untyped, only support responses involving constants and definitions.
            if (fileIsTyped || q->isConstant() || q->isDefinition()) {
                addLocIfExists(gs, locations, loc);
            }
        }
    }
    // Dedupe locations
    fast_sort(locations, [](const unique_ptr<Location> &a, const unique_ptr<Location> &b) -> bool {
        return cmpLocations(*a, *b) < 0;
    });
    locations.resize(std::distance(
        locations.begin(), std::unique(locations.begin(), locations.end(),
                                       [](const unique_ptr<Location> &a, const unique_ptr<Location> &b) -> bool {
                                           return cmpLocations(*a, *b) == 0;
                                       })));
    return locations;
}

bool hideSymbol(const core::GlobalState &gs, core::SymbolRef sym) {
    if (!sym.exists() || sym == core::Symbols::root()) {
        return true;
    }
    auto data = sym.data(gs);
    if (data->isClass() && data->attachedClass(gs).exists()) {
        return true;
    }
    if (data->isClass() && data->superClass() == core::Symbols::StubModule()) {
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
                    const unique_ptr<core::TypeConstraint> &constraint) {
    ENFORCE(method.exists());
    // handle this case anyways so that we don't crash in prod when this method is mis-used
    if (!method.exists()) {
        return "";
    }

    if (!retType) {
        retType = getResultType(gs, method.data(gs)->resultType, method, receiver, constraint);
    }
    string methodReturnType =
        (retType == core::Types::void_()) ? "void" : absl::StrCat("returns(", retType->show(gs), ")");
    vector<string> typeAndArgNames;

    vector<string> flags;
    const core::SymbolData &sym = method.data(gs);
    string accessFlagString = "";
    if (sym->isMethod()) {
        if (sym->hasGeneratedSig()) {
            flags.emplace_back("generated");
        }
        if (sym->isAbstract()) {
            flags.emplace_back("abstract");
        }
        if (sym->isOverridable()) {
            flags.emplace_back("overridable");
        }
        if (sym->isOverride()) {
            flags.emplace_back("override");
        }
        if (sym->isImplementation()) {
            flags.emplace_back("implementation");
        }
        if (sym->isPrivate()) {
            accessFlagString = "private ";
        }
        if (sym->isProtected()) {
            accessFlagString = "protected ";
        }
        for (auto &argSym : method.data(gs)->arguments()) {
            // Don't display synthetic arguments (like blk).
            if (!argSym.isSyntheticBlockArgument()) {
                typeAndArgNames.emplace_back(
                    absl::StrCat(argSym.argumentName(gs), ": ",
                                 getResultType(gs, argSym.type, method, receiver, constraint)->show(gs)));
            }
        }
    }

    string flagString = "";
    if (!flags.empty()) {
        flagString = fmt::format("{}.", fmt::join(flags, "."));
    }
    string paramsString = "";
    if (!typeAndArgNames.empty()) {
        paramsString = fmt::format("params({}).", fmt::join(typeAndArgNames, ", "));
    }
    return fmt::format("{}sig {{{}{}{}}}", accessFlagString, flagString, paramsString, methodReturnType);
}

core::TypePtr getResultType(const core::GlobalState &gs, core::TypePtr type, core::SymbolRef inWhat,
                            core::TypePtr receiver, const unique_ptr<core::TypeConstraint> &constr) {
    core::Context ctx(gs, inWhat);
    auto resultType = type;
    if (auto *proxy = core::cast_type<core::ProxyType>(receiver.get())) {
        receiver = proxy->underlying();
    }
    if (auto *applied = core::cast_type<core::AppliedType>(receiver.get())) {
        /* instantiate generic classes */
        resultType = core::Types::resultTypeAsSeenFrom(ctx, resultType, inWhat.data(ctx)->enclosingClass(ctx),
                                                       applied->klass, applied->targs);
    }
    if (!resultType) {
        resultType = core::Types::untypedUntracked();
    }
    if (receiver) {
        resultType = core::Types::replaceSelfType(ctx, resultType, receiver); // instantiate self types
    }
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
    } else if (sym->isTypeMember()) {
        return SymbolKind::TypeParameter;
    } else if (sym->isTypeArgument()) {
        return SymbolKind::TypeParameter;
    }
    return SymbolKind::Unknown;
}

} // namespace sorbet::realmain::lsp
