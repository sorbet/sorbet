#include "AbstractRenamer.h"
#include "main/lsp/lsp.h"
using namespace std;

namespace sorbet::realmain::lsp {

namespace {

core::ClassOrModuleRef findRootClassWithMethod(const core::GlobalState &gs, core::ClassOrModuleRef klass,
                                               core::NameRef methodName) {
    auto root = klass;
    while (true) {
        auto tmp = root.data(gs)->superClass();
        ENFORCE(tmp.exists()); // everything derives from Kernel::Object so we can't ever reach the actual top type
        if (!tmp.exists() || !(tmp.data(gs)->findMember(gs, methodName).exists())) {
            break;
        }
        root = tmp;
    }
    return root;
}

} // namespace

bool AbstractRenamer::UniqueSymbolQueue::tryEnqueue(core::SymbolRef s) {
    auto insertResult = set.insert(s);
    bool isNew = insertResult.second;
    if (isNew) {
        symbols.emplace_back(s);
    }
    return isNew;
}

core::SymbolRef AbstractRenamer::UniqueSymbolQueue::pop() {
    if (!symbols.empty()) {
        auto s = symbols.front();
        symbols.pop_front();
        return s;
    }
    return core::Symbols::noSymbol();
}

variant<JSONNullObject, unique_ptr<WorkspaceEdit>> AbstractRenamer::buildEdit() {
    if (invalid) {
        return JSONNullObject();
    }

    UnorderedMap<string, vector<unique_ptr<TextEdit>>> tmpEdits;
    vector<unique_ptr<TextDocumentEdit>> textDocEdits;
    // collect changes per file
    for (auto &item : edits) {
        core::Loc loc = item.first;
        string newsrc = item.second;
        auto location = config.loc2Location(gs, loc);
        ENFORCE(location != nullptr); // loc should always exist
        if (location == nullptr) {
            continue;
        }
        tmpEdits[location->uri].push_back(make_unique<TextEdit>(move(location->range), move(newsrc)));
    }
    for (auto &item : tmpEdits) {
        // TODO: Version.
        textDocEdits.push_back(make_unique<TextDocumentEdit>(
            make_unique<VersionedTextDocumentIdentifier>(item.first, JSONNullObject()), move(item.second)));
    }
    auto we = make_unique<WorkspaceEdit>();
    we->documentChanges = move(textDocEdits);
    return we;
}

bool AbstractRenamer::getInvalid() {
    return invalid;
}

std::string AbstractRenamer::getError() {
    return error;
}

std::shared_ptr<AbstractRenamer::UniqueSymbolQueue> AbstractRenamer::getQueue() {
    // return symbolQueue;
    return symbolQueue;
}

// Add subclass-related methods (methods overriding and overridden by `symbol`) to the `methods` vector.
void AbstractRenamer::addSubclassRelatedMethods(const core::GlobalState &gs, core::MethodRef symbol,
                                                shared_ptr<UniqueSymbolQueue> methods) {
    auto symbolData = symbol.data(gs);

    // We have to check for methods as part of a class hierarchy: Follow superClass() links till we find the root;
    // then find the full tree; then look for methods with the same name as ours; then find all references to all
    // those methods and rename them.
    auto symbolClass = symbol.enclosingClass(gs);

    // We have to be careful to follow superclass links only as long as we find a method that `symbol` overrides.
    // Otherwise we will find unrelated methods and rename them even though they don't need to be (see the
    // method_class_hierarchy test case for an example).
    auto root = findRootClassWithMethod(gs, symbolClass, symbolData->name);

    // Scans whole symbol table. This is slow, and we might need to make this faster eventually.
    auto includeRoot = true;
    auto subclasses = getSubclassesSlow(gs, root, includeRoot);

    // find the target method definition in each subclass
    for (auto c : subclasses) {
        auto classSymbol = c.data(gs);
        auto member = classSymbol->findMethod(gs, symbolData->name);
        if (!member.exists()) {
            continue;
        }
        methods->tryEnqueue(member);
    }
}

// Add methods that are related because of dispatching via secondary components in sends (union types).
void AbstractRenamer::addDispatchRelatedMethods(const core::GlobalState &gs, const core::DispatchResult *dispatchResult,
                                                shared_ptr<UniqueSymbolQueue> methods) {
    for (const core::DispatchResult *dr = dispatchResult; dr != nullptr; dr = dr->secondary.get()) {
        auto method = dr->main.method;
        ENFORCE(method.exists());
        auto isNew = methods->tryEnqueue(method);
        if (isNew) {
            addSubclassRelatedMethods(gs, method, methods);
        }
    }
}

} // namespace sorbet::realmain::lsp
