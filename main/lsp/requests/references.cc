#include "main/lsp/requests/references.h"
#include "core/lsp/QueryResponse.h"
#include "main/lsp/LSPLoop.h"
#include "main/lsp/LSPOutput.h"
#include "main/lsp/LSPQuery.h"
#include "main/lsp/ShowOperation.h"
#include "main/lsp/json_types.h"

using namespace std;

namespace sorbet::realmain::lsp {

ReferencesTask::ReferencesTask(const LSPConfiguration &config, MessageId id, unique_ptr<ReferenceParams> params,
                               bool hierarchyReferences)
    : LSPRequestTask(config, move(id), LSPMethod::TextDocumentReferences), params(move(params)),
      hierarchyReferences(hierarchyReferences) {}

bool ReferencesTask::needsMultithreading(const LSPIndexer &indexer) const {
    return true;
}

namespace {

core::SymbolRef findSym(const core::GlobalState &gs, const vector<core::NameRef> &fullName,
                        core::SymbolRef underNamespace) {
    core::SymbolRef symToCheck = underNamespace;
    for (auto part = fullName.rbegin(); part != fullName.rend(); ++part) {
        symToCheck = symToCheck.asClassOrModuleRef().data(gs)->findMember(gs, *part);
        if (!symToCheck.exists()) {
            return symToCheck;
        }
    }

    return symToCheck;
}

core::lsp::Query::Symbol::STORAGE getSymsToCheckWithinPackage(const core::GlobalState &gs, core::SymbolRef symInPackage,
                                                              core::packages::MangledName packageName) {
    vector<core::NameRef> fullName;

    auto sym = symInPackage;
    while (sym.exists() && sym != core::Symbols::PackageSpecRegistry() && sym != core::Symbols::root()) {
        fullName.emplace_back(sym.name(gs));
        sym = sym.owner(gs);
    }

    core::lsp::Query::Symbol::STORAGE result;
    vector<core::SymbolRef> namespacesToCheck = {
        core::Symbols::root(),
        core::Symbols::root().data(gs)->findMember(gs, core::packages::PackageDB::TEST_NAMESPACE),
    };

    for (auto &namespaceToCheck : namespacesToCheck) {
        if (!namespaceToCheck.exists()) {
            continue;
        }

        auto symFound = findSym(gs, fullName, namespaceToCheck);
        // Do nothing if the symbol is not found or is from the same package -- i.e. for class ... < PackageSpec
        // declarations
        if (symFound.exists() && symFound.enclosingClass(gs).data(gs)->package != packageName) {
            result.emplace_back(std::move(symFound));
        }
    }

    return result;
}

// This will find *all* classes which own a member with the given name, in any ancestor.
//
// That might find "too many" methods. For example, if you want to find all references to a method
// called `load` that you've defined in some module of yours, it will *also* find references to
// `Kernel#load`.
//
// Maybe we could be smarter about this, maybe looking at whether either the parent or child method
// had `override` or `abstract` or something, and only show references like that. But then again,
// there are cases where you're not currently required to put any annotation (e.g., if you don't have
// a sig but you override a parent method). So for now, I'm punting on this and returning all
// references. Maybe at some point we could bound the search somewhere, but hopefully in practice
// people only use this for compatibly-overridden methods.
void parentsWithMember(const core::GlobalState &gs, core::ClassOrModuleRef owner, core::NameRef name,
                       vector<core::ClassOrModuleRef> &result) {
    auto ownerData = owner.data(gs);
    ENFORCE(ownerData->flags.isLinearizationComputed, "This algorithm does not look through mixins transitively");
    for (const auto mixin : ownerData->mixins()) {
        // No dealias: show the definition of the alias, but not calls to the new method.
        auto parentMember = mixin.data(gs)->findMemberNoDealias(name);
        if (parentMember.exists()) {
            result.emplace_back(mixin);
        }
    }

    // BasicObject has no superClass, so this terminates
    auto superClass = ownerData->superClass();
    if (superClass.exists()) {
        auto parentMember = superClass.data(gs)->findMemberNoDealias(name);
        if (parentMember.exists()) {
            result.emplace_back(superClass);
        }

        parentsWithMember(gs, superClass, name, result);
    }
}

// TODO(jez) This method is an example of a place where we could possibly end up with lots of symbols
// We're currently searching through a vector of symbols for a query match--if we see this being
// slow, we might want to change the STORAGE to an UnorderedSet instead of a vector.
void addHierarchyRelatedSymbols(const core::GlobalState &gs, core::lsp::Query::Symbol::STORAGE &symbols) {
    auto names = vector<core::NameRef>{};
    auto parentClasses = vector<core::ClassOrModuleRef>{};
    for (const auto sym : symbols) {
        auto name = sym.name(gs);
        names.emplace_back(name);
        auto owner = sym.enclosingClass(gs);
        parentClasses.emplace_back(owner);
        parentsWithMember(gs, owner, name, parentClasses);
    }

    // There may be duplicate parents accumulated into parentClasses, but they will be deduplicated as
    // the first step of getSubclassesSlowMulti, so it's fine to skip de-duplicating on our own.
    for (const auto subclass : getSubclassesSlowMulti(gs, parentClasses)) {
        for (auto name : names) {
            auto subclassMember = subclass.data(gs)->findMemberNoDealias(name);
            if (subclassMember.exists()) {
                symbols.emplace_back(subclassMember);
                // This could be a `prop` or `attr_*`, which have multiple associated symbols.
                if (subclassMember.isMethod() || subclassMember.isField(gs)) {
                    addOtherAccessorSymbols(gs, subclassMember, symbols);
                }
            }
        }
    }
}

} // namespace

unique_ptr<ResponseMessage> ReferencesTask::runRequest(LSPTypecheckerDelegate &typechecker) {
    auto response = make_unique<ResponseMessage>("2.0", id, LSPMethod::TextDocumentReferences);
    ShowOperation op(config, ShowOperation::Kind::References);

    const core::GlobalState &gs = typechecker.state();
    const auto &uri = params->textDocument->uri;
    auto result =
        LSPQuery::byLoc(config, typechecker, uri, *params->position, LSPMethod::TextDocumentReferences, false);
    if (result.error) {
        // An error happened while setting up the query.
        response->error = move(result.error);
        return response;
    }

    // An explicit null indicates that we don't support this request (or that nothing was at the location).
    // Note: Need to correctly type variant here so it goes into right 'slot' of result variant.
    response->result = variant<JSONNullObject, vector<unique_ptr<Location>>>(JSONNullObject());
    auto &queryResponses = result.responses;
    bool notifyAboutUntypedFile = false;
    core::FileRef fref = config.uri2FileRef(gs, uri);
    bool fileIsTyped = false;
    if (fref.exists()) {
        fileIsTyped = fref.data(gs).strictLevel >= core::StrictLevel::True;
    }
    if (!queryResponses.empty()) {
        auto resp = getQueryResponseForFindAllReferences(gs, queryResponses);

        // If file is untyped, only supports find reference requests from constants and class definitions.
        if (auto constResp = resp->isConstant()) {
            if (fref.data(gs).isPackage(gs)) {
                // Special handling for package files.
                //
                // Case 1. get-refs on a package declaration
                //   class Foo < PackageSpec
                //         ^^^
                //   Returns all `import Foo` statements, globally
                //
                // Case 2. get-refs on an import statement
                //
                //  class Foo < PackageSpec
                //    ...
                //
                //    import Bar
                //          ^^^
                //  Returns all usages of `Bar` or `Test::Bar` *within* the Foo package only.
                //
                // Case 3. get-refs on an export statement
                //
                //  class Foo < PackageSpec
                //    ...
                //
                //    export Foo::A
                //                ^
                //  Returns all global usages of Foo::A

                auto packageName = gs.packageDB().getPackageNameForFile(fref);
                auto symsToCheck = getSymsToCheckWithinPackage(gs, constResp->symbolBeforeDealias, packageName);

                if (!symsToCheck.empty()) {
                    response->result = extractLocations(
                        gs, getReferencesToSymbolsInPackage(typechecker, packageName, move(symsToCheck)));
                } else {
                    auto symbols = core::lsp::Query::Symbol::STORAGE{1, constResp->symbolBeforeDealias};
                    response->result =
                        extractLocations(typechecker.state(), getReferencesToSymbols(typechecker, move(symbols)));
                }
            } else {
                // Normal handling for non-package files
                auto symbols = core::lsp::Query::Symbol::STORAGE{1, constResp->symbolBeforeDealias};
                if (constResp->symbolBeforeDealias.isTypeMember() && hierarchyReferences) {
                    // Only type member constants behave like inherited/overridden methods.
                    // Normal static field constants do not, and we should not treat them as such.
                    addHierarchyRelatedSymbols(gs, symbols);
                }
                response->result =
                    extractLocations(typechecker.state(), getReferencesToSymbols(typechecker, move(symbols)));
            }
        } else if (auto fieldResp = resp->isField()) {
            auto symbols = core::lsp::Query::Symbol::STORAGE{1, fieldResp->symbol};
            // This could be a `prop` or `attr_*`, which have multiple associated symbols.
            addOtherAccessorSymbols(gs, fieldResp->symbol, symbols);
            if (hierarchyReferences) {
                addHierarchyRelatedSymbols(gs, symbols);
            }
            response->result = extractLocations(gs, getReferencesToSymbols(typechecker, move(symbols)));
        } else if (auto defResp = resp->isMethodDef()) {
            if (fileIsTyped) {
                auto symbols = core::lsp::Query::Symbol::STORAGE{1, defResp->symbol};
                // This could be a `prop` or `attr_*`, which have multiple associated symbols.
                addOtherAccessorSymbols(gs, defResp->symbol, symbols);

                if (hierarchyReferences) {
                    addHierarchyRelatedSymbols(gs, symbols);
                }

                response->result = extractLocations(gs, getReferencesToSymbols(typechecker, move(symbols)));
            } else {
                notifyAboutUntypedFile = true;
            }
        } else if (auto identResp = resp->isIdent()) {
            if (fileIsTyped) {
                auto loc = identResp->termLoc;
                if (loc.exists()) {
                    auto run2 = typechecker.query(core::lsp::Query::createVarQuery(identResp->enclosingMethod,
                                                                                   identResp->enclosingMethodLoc,
                                                                                   identResp->variable),
                                                  {loc.file()});
                    response->result = extractLocations(gs, run2.responses);
                }
            } else {
                notifyAboutUntypedFile = true;
            }
        } else if (auto sendResp = resp->isSend()) {
            if (fileIsTyped) {
                auto start = sendResp->dispatchResult.get();
                auto symbols = core::lsp::Query::Symbol::STORAGE{};
                while (start != nullptr) {
                    if (start->main.method.exists() && !start->main.receiver.isUntyped()) {
                        symbols.emplace_back(start->main.method);
                        // This could be a `prop` or `attr_*`, which has multiple associated symbols.
                        addOtherAccessorSymbols(gs, start->main.method, symbols);
                    }
                    start = start->secondary.get();
                }

                if (hierarchyReferences) {
                    addHierarchyRelatedSymbols(gs, symbols);
                }

                response->result = extractLocations(gs, getReferencesToSymbols(typechecker, move(symbols)));
            } else {
                notifyAboutUntypedFile = true;
            }
        }
    } else if (fref.exists() && !fileIsTyped) {
        // The first check ensures that the file actually exists (and therefore
        // we could have gotten responses) and the second check is what we are
        // actually interested in.
        notifyAboutUntypedFile = true;
    }

    if (notifyAboutUntypedFile) {
        ENFORCE(fref.exists());
        auto level = fref.data(gs).strictLevel;
        ENFORCE(level < core::StrictLevel::True);
        string asString = level == core::StrictLevel::Ignore ? "ignore" : "false";
        auto msg = fmt::format("File is `# typed: {}`, could not determine references", asString);
        auto params = make_unique<ShowMessageParams>(MessageType::Info, msg);
        this->config.output->write(make_unique<LSPMessage>(
            make_unique<NotificationMessage>("2.0", LSPMethod::WindowShowMessage, move(params))));
    }
    return response;
}

} // namespace sorbet::realmain::lsp
