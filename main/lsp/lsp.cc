#include "lsp.h"
#include "absl/strings/str_cat.h"
#include "ast/treemap/treemap.h"
#include "common/Timer.h"
#include "core/Errors.h"
#include "core/Files.h"
#include "core/Unfreeze.h"
#include "core/errors/internal.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"
#include "core/lsp/DefLocSaver.h"
#include "core/lsp/QueryResponse.h"
#include "main/pipeline/pipeline.h"
#include "namer/namer.h"
#include "resolver/resolver.h"
#include "spdlog/fmt/ostr.h"
#include <condition_variable> // std::condition_variable
#include <deque>
#include <mutex> // std::mutex, std::unique_lock
#include <unordered_set>

using namespace std;

namespace sorbet {
namespace realmain {
namespace lsp {

bool hasSimilarName(core::GlobalState &gs, core::NameRef name, const absl::string_view &pattern) {
    absl::string_view view = name.data(gs).shortName(gs);
    auto fnd = view.find(pattern);
    return fnd != absl::string_view::npos;
}

static bool startsWith(const absl::string_view str, const absl::string_view prefix) {
    return str.size() >= prefix.size() && 0 == str.compare(0, prefix.size(), prefix.data(), prefix.size());
}

bool safeGetline(std::istream &is, std::string &t) {
    t.clear();

    // The characters in the stream are read one-by-one using a std::streambuf.
    // That is faster than reading them one-by-one using the std::istream.
    // Code that uses streambuf this way must be guarded by a sentry object.
    // The sentry object performs various tasks,
    // such as thread synchronization and updating the stream state.

    std::istream::sentry se(is, true);
    std::streambuf *sb = is.rdbuf();

    for (;;) {
        int c = sb->sbumpc();
        switch (c) {
            case '\n':
                return true;
            case '\r':
                if (sb->sgetc() == '\n')
                    sb->sbumpc();
                return true;
            case std::streambuf::traits_type::eof():
                // Also handle the case when the last line has no line ending
                if (t.empty())
                    is.setstate(std::ios::eofbit);
                return false;
            default:
                t += (char)c;
        }
    }
}

LSPLoop::LSPLoop(std::unique_ptr<core::GlobalState> gs, const options::Options &opts,
                 std::shared_ptr<spd::logger> &logger, WorkerPool &workers)
    : initialGS(move(gs)), opts(opts), logger(logger), workers(workers) {
    errorQueue = dynamic_pointer_cast<realmain::ConcurrentErrorQueue>(initialGS->errorQueue);
    ENFORCE(errorQueue, "LSPLoop got an unexpected error queue");
}

shared_ptr<core::Type> getResultType(core::GlobalState &gs, core::SymbolRef ofWhat, shared_ptr<core::Type> receiver,
                                     shared_ptr<core::TypeConstraint> constr) {
    core::Context ctx(gs, core::Symbols::root());
    auto resultType = ofWhat.data(gs).resultType;
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

bool getNewRequest(rapidjson::Document &d, const std::shared_ptr<spd::logger> &logger) {
    int length = -1;
    {
        std::string line;
        while (safeGetline(cin, line)) {
            logger->trace("raw read: {}", line);
            if (line == "") {
                break;
            }
            sscanf(line.c_str(), "Content-Length: %i", &length);
        }
        logger->trace("final raw read: {}, length: {}", line, length);
    }
    if (length < 0) {
        logger->debug("eof");
        return false;
    }

    string json(length, '\0');
    cin.read(&json[0], length);
    logger->debug("Read: {}", json);
    if (d.Parse(json.c_str()).HasParseError()) {
        logger->error("Last LSP request: `{}` is not a valid json object", json);
        return false;
    }
    return true;
}

void LSPLoop::processRequest(rapidjson::Document &d) {
    const LSPMethod method = LSPMethod::getByName({d["method"].GetString(), d["method"].GetStringLength()});

    ENFORCE(method.kind == LSPMethod::Kind::ClientInitiated || method.kind == LSPMethod::Kind::Both);

    if (!ensureInitialized(method, d)) {
        return;
    }
    if (method.isNotification) {
        logger->debug("Processing notification {} ", (string)method.name);
        if (method == LSPMethod::DidChangeWatchedFiles()) {
            sendRequest(LSPMethod::ReadFile(), d["params"],
                        [&](rapidjson::Value &edits) -> void {
                            ENFORCE(edits.IsArray());
                            Timer timeit(logger, "handle update");
                            vector<shared_ptr<core::File>> files;

                            for (auto it = edits.Begin(); it != edits.End(); ++it) {
                                rapidjson::Value &change = *it;
                                string uri(change["uri"].GetString(), change["uri"].GetStringLength());
                                string content(change["content"].GetString(), change["content"].GetStringLength());
                                if (startsWith(uri, rootUri)) {
                                    files.emplace_back(make_shared<core::File>(remoteName2Local(uri), move(content),
                                                                               core::File::Type::Normal));
                                }
                            }

                            tryFastPath(files);
                            pushErrors();
                        },
                        [](rapidjson::Value &error) -> void {});
        }
        if (method == LSPMethod::TextDocumentDidChange()) {
            Timer timeit(logger, "handle update");
            vector<shared_ptr<core::File>> files;
            auto &edits = d["params"];
            ENFORCE(edits.IsObject());
            /*
              {
              "textDocument":{"uri":"file:///Users/dmitry/stripe/pay-server/cibot/lib/cibot/gerald.rb","version":2},
                "contentChanges":[{"text":"..."}]
                */
            string uri(edits["textDocument"]["uri"].GetString(), edits["textDocument"]["uri"].GetStringLength());
            string content(edits["contentChanges"][0]["text"].GetString(),
                           edits["contentChanges"][0]["text"].GetStringLength());
            // TODO: if this is ever updated to support diffs, be aware that the coordinator thread should be
            // taught about it too: it merges consecutive TextDocumentDidChange
            if (startsWith(uri, rootUri)) {
                files.emplace_back(
                    make_shared<core::File>(remoteName2Local(uri), move(content), core::File::Type::Normal));

                tryFastPath(files);
                pushErrors();
            }
        }
        if (method == LSPMethod::TextDocumentDidOpen()) {
            Timer timeit(logger, "handle open");
            vector<shared_ptr<core::File>> files;
            auto &edits = d["params"];
            ENFORCE(edits.IsObject());
            /*
              {
              "textDocument":{"uri":"file:///Users/dmitry/stripe/pay-server/cibot/lib/cibot/gerald.rb","version":2},
                "contentChanges":[{"text":"..."}]
                */
            string uri(edits["textDocument"]["uri"].GetString(), edits["textDocument"]["uri"].GetStringLength());
            string content(edits["textDocument"]["text"].GetString(), edits["textDocument"]["text"].GetStringLength());
            if (startsWith(uri, rootUri)) {
                files.emplace_back(
                    make_shared<core::File>(remoteName2Local(uri), move(content), core::File::Type::Normal));

                tryFastPath(files);
                pushErrors();
            }
        }
        if (method == LSPMethod::Initialized()) {
            // initialize ourselves
            {
                Timer timeit(logger, "index");
                reIndexFromFileSystem();
                vector<shared_ptr<core::File>> changedFiles;
                runSlowPath(move(changedFiles));
                ENFORCE(finalGs);
                pushErrors();
                this->globalStateHashes = computeStateHashes(finalGs->getFiles());
            }
        }
        if (method == LSPMethod::Exit()) {
            return;
        }
    } else {
        logger->debug("Processing request {}", method.name);
        // is request
        rapidjson::Value result;
        int errorCode = 0;
        string errorString;
        if (d.FindMember("cancelled") != d.MemberEnd()) {
            errorCode = (int)LSPErrorCodes::RequestCancelled;
            errorString = "Request was cancelled";
            sendError(d, errorCode, errorString);
        } else if (method == LSPMethod::Initialize()) {
            result.SetObject();
            rootUri = string(d["params"]["rootUri"].GetString(), d["params"]["rootUri"].GetStringLength());
            string serverCap = "{\"capabilities\": "
                               "   {"
                               "       \"textDocumentSync\": 1, "
                               "       \"documentSymbolProvider\": true, "
                               "       \"workspaceSymbolProvider\": true, "
                               "       \"definitionProvider\": true, "
                               "       \"hoverProvider\":true,"
                               "       \"completionProvider\": { "
                               "           \"triggerCharacters\": [\".\"]"
                               "       }"
                               "   }"
                               "}";
            rapidjson::Document temp;
            auto &r = temp.Parse(serverCap.c_str());
            ENFORCE(!r.HasParseError());
            result.CopyFrom(temp, alloc);
            sendResult(d, result);
        } else if (method == LSPMethod::Shutdown()) {
            // return default value: null
            sendResult(d, result);
        } else if (method == LSPMethod::TextDocumentDocumentSymbol()) {
            result.SetArray();
            auto uri = string(d["params"]["textDocument"]["uri"].GetString(),
                              d["params"]["textDocument"]["uri"].GetStringLength());
            auto fref = uri2FileRef(uri);
            for (u4 idx = 1; idx < finalGs->symbolsUsed(); idx++) {
                core::SymbolRef ref(finalGs.get(), idx);
                if (ref.data(*finalGs).loc.file == fref) {
                    auto data = symbolRef2SymbolInformation(ref);
                    if (data) {
                        result.PushBack(move(*data), alloc);
                    }
                }
            }
            sendResult(d, result);
        } else if (method == LSPMethod::WorkspaceSymbolsRequest()) {
            result.SetArray();
            string searchString = d["params"]["query"].GetString();

            for (u4 idx = 1; idx < finalGs->symbolsUsed(); idx++) {
                core::SymbolRef ref(finalGs.get(), idx);
                if (hasSimilarName(*finalGs, ref.data(*finalGs).name, searchString)) {
                    auto data = symbolRef2SymbolInformation(ref);
                    if (data) {
                        result.PushBack(move(*data), alloc);
                    }
                }
            }
            sendResult(d, result);
        } else if (method == LSPMethod::TextDocumentDefinition()) {
            handleTextDocumentDefinition(result, d);
        } else if (method == LSPMethod::TextDocumentHover()) {
            handleTextDocumentHover(result, d);
        } else if (method == LSPMethod::TextDocumentCompletion()) {
            handleTextDocumentCompletion(result, d);
        } else {
            ENFORCE(!method.isSupported, "failing a supported method");
            errorCode = (int)LSPErrorCodes::MethodNotFound;
            errorString = fmt::format("Unknown method: {}", method.name);
            sendError(d, errorCode, errorString);
        }
    }
}

void LSPLoop::addLocIfExists(rapidjson::Value &result, core::Loc loc) {
    if (loc.file.exists()) {
        result.PushBack(loc2Location(loc), alloc);
    }
}

void LSPLoop::handleTextDocumentDefinition(rapidjson::Value &result, rapidjson::Document &d) {
    result.SetArray();

    if (!setupLSPQueryByLoc(d, LSPMethod::TextDocumentDefinition())) {
        return;
    }
    auto queryResponses = errorQueue->drainQueryResponses();
    if (!queryResponses.empty()) {
        auto resp = std::move(queryResponses[0]);

        if (resp->kind == core::QueryResponse::Kind::IDENT) {
            for (auto &originLoc : resp->retType.origins) {
                addLocIfExists(result, originLoc);
            }
        } else if (resp->kind == core::QueryResponse::Kind::DEFINITION) {
            result.PushBack(loc2Location(resp->termLoc), alloc);
        } else {
            for (auto &component : resp->dispatchComponents) {
                if (component.method.exists()) {
                    addLocIfExists(result, component.method.data(*finalGs).loc);
                }
            }
        }
    }
    sendResult(d, result);
}

unordered_map<core::NameRef, vector<core::SymbolRef>>
mergeMaps(unordered_map<core::NameRef, vector<core::SymbolRef>> &&first,
          unordered_map<core::NameRef, vector<core::SymbolRef>> &&second) {
    for (auto &other : second) {
        first[other.first].insert(first[other.first].end(), make_move_iterator(other.second.begin()),
                                  make_move_iterator(other.second.end()));
    }
    return first;
};

unordered_map<core::NameRef, vector<core::SymbolRef>>
findSimilarMethodsIn(core::GlobalState &gs, shared_ptr<core::Type> receiver, absl::string_view name) {
    unordered_map<core::NameRef, vector<core::SymbolRef>> result;
    typecase(
        receiver.get(),
        [&](core::ClassType *c) {
            auto &owner = c->symbol.data(gs);
            for (auto member : owner.members) {
                auto sym = member.second;
                if (sym.data(gs).isMethod() && hasSimilarName(gs, sym.data(gs).name, name)) {
                    result[sym.data(gs).name].emplace_back(sym);
                }
            }
            for (auto mixin : owner.mixins()) {
                result = mergeMaps(move(result), findSimilarMethodsIn(gs, make_shared<core::ClassType>(mixin), name));
            }
            if (owner.superClass.exists()) {
                result = mergeMaps(move(result),
                                   findSimilarMethodsIn(gs, make_shared<core::ClassType>(owner.superClass), name));
            }
        },
        [&](core::AndType *c) {
            result = mergeMaps(findSimilarMethodsIn(gs, c->left, name), findSimilarMethodsIn(gs, c->right, name));
        },
        [&](core::OrType *c) {
            auto lhs = findSimilarMethodsIn(gs, c->left, name);
            auto rhs = findSimilarMethodsIn(gs, c->right, name);
            for (auto it = rhs.begin(); it != rhs.end(); /*nothing*/) {
                auto &other = *it;
                auto fnd = lhs.find(other.first);
                if (fnd == lhs.end()) {
                    it = rhs.erase(it);
                } else {
                    it->second.insert(it->second.end(), make_move_iterator(fnd->second.begin()),
                                      make_move_iterator(fnd->second.end()));
                    ++it;
                }
            }
        },
        [&](core::AppliedType *c) { result = findSimilarMethodsIn(gs, make_shared<core::ClassType>(c->klass), name); },
        [&](core::ProxyType *c) { result = findSimilarMethodsIn(gs, c->underlying, name); }, [&](core::Type *c) {});
    return result;
}

string methodDetail(core::GlobalState &gs, core::SymbolRef method, shared_ptr<core::Type> receiver,
                    shared_ptr<core::Type> retType, shared_ptr<core::TypeConstraint> constraint) {
    string ret;
    if (!retType) {
        retType = getResultType(gs, method, receiver, constraint);
    }
    string methodReturnType = (retType == core::Types::void_()) ? "void" : "returns(" + retType->show(gs) + ")";
    std::vector<string> typeAndArgNames;

    if (method.data(gs).isMethod()) {
        for (auto &argSym : method.data(gs).arguments()) {
            typeAndArgNames.push_back(argSym.data(gs).name.show(gs) + ": " +
                                      getResultType(gs, argSym, receiver, constraint)->show(gs));
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

string methodSnippet(core::GlobalState &gs, core::SymbolRef method) {
    string ret;

    auto shortName = method.data(gs).name.data(gs).shortName(gs);
    std::vector<string> typeAndArgNames;

    int i = 1;
    if (method.data(gs).isMethod()) {
        for (auto &argSym : method.data(gs).arguments()) {
            string s;
            if (argSym.data(gs).isKeyword()) {
                s += (string)argSym.data(gs).name.data(gs).shortName(gs) + ": ";
            }
            s += "${" + to_string(i++) + "}";
            typeAndArgNames.push_back(s);
        }
    }

    std::stringstream ss;
    for (size_t i = 0; i < typeAndArgNames.size(); ++i) {
        if (i != 0) {
            ss << ", ";
        }
        ss << typeAndArgNames[i];
    }

    return fmt::format("{}({}){}", shortName, ss.str(), "${0}");
}

void addCompletionItem(core::GlobalState &gs, rapidjson::MemoryPoolAllocator<> &alloc, rapidjson::Value &items,
                       core::SymbolRef what, const core::QueryResponse &resp) {
    rapidjson::Value item;
    item.SetObject();
    item.AddMember("label", (string)what.data(gs).name.data(gs).shortName(gs), alloc);
    auto resultType = what.data(gs).resultType;
    if (!resultType) {
        resultType = core::Types::untyped();
    }
    if (what.data(gs).isMethod()) {
        item.AddMember("detail", methodDetail(gs, what, resp.receiver.type, nullptr, resp.constraint), alloc);
        item.AddMember("insertTextFormat", 2, alloc); // Snippet

        item.AddMember("insertText", methodSnippet(gs, what), alloc);
    } else if (what.data(gs).isStaticField()) {
        item.AddMember("detail", resultType->show(gs), alloc);
    }
    items.PushBack(move(item), alloc);
}

void LSPLoop::handleTextDocumentCompletion(rapidjson::Value &result, rapidjson::Document &d) {
    result.SetObject();
    result.AddMember("isIncomplete", "false", alloc);
    rapidjson::Value items;
    items.SetArray();

    if (!setupLSPQueryByLoc(d, LSPMethod::TextDocumentCompletion())) {
        return;
    }
    auto queryResponses = errorQueue->drainQueryResponses();
    if (!queryResponses.empty()) {
        auto resp = std::move(queryResponses[0]);
        auto receiverType = resp->receiver.type;
        if (resp->kind == core::QueryResponse::Kind::SEND) {
            auto pattern = resp->name.data(*finalGs).shortName(*finalGs);
            logger->debug("Looking for method similar to {}", pattern);
            unordered_map<core::NameRef, vector<core::SymbolRef>> methods =
                findSimilarMethodsIn(*finalGs, receiverType, pattern);
            for (auto &entry : methods) {
                addCompletionItem(*finalGs, alloc, items, entry.second[0], *resp);
            }
        } else if (resp->kind == core::QueryResponse::Kind::IDENT ||
                   resp->kind == core::QueryResponse::Kind::CONSTANT) {
            if (auto c = core::cast_type<core::ClassType>(receiverType.get())) {
                auto pattern = c->symbol.data(*finalGs).name.data(*finalGs).shortName(*finalGs);
                logger->debug("Looking for constant similar to {}", pattern);
                core::SymbolRef owner = c->symbol.data(*finalGs).owner;
                for (auto member : owner.data(*finalGs).members) {
                    auto sym = member.second;
                    if (sym.exists() && (sym.data(*finalGs).isClass() || sym.data(*finalGs).isStaticField()) &&
                        sym.data(*finalGs).name.data(*finalGs).kind == core::NameKind::CONSTANT && // hide singletons
                        hasSimilarName(*finalGs, sym.data(*finalGs).name, pattern) &&
                        !sym.data(*finalGs).derivesFrom(*finalGs, core::Symbols::StubClass())) {
                        addCompletionItem(*finalGs, alloc, items, sym, *resp);
                    }
                }
            }
        } else {
        }
    }
    result.AddMember("items", move(items), alloc);
    sendResult(d, result);
}

void LSPLoop::handleTextDocumentHover(rapidjson::Value &result, rapidjson::Document &d) {
    result.SetObject();

    if (!setupLSPQueryByLoc(d, LSPMethod::TextDocumentHover())) {
        return;
    }

    auto queryResponses = errorQueue->drainQueryResponses();
    if (queryResponses.empty()) {
        rapidjson::Value nullreply;
        sendResult(d, nullreply);
        return;
    }

    auto resp = std::move(queryResponses[0]);
    if (resp->kind == core::QueryResponse::Kind::SEND) {
        if (resp->dispatchComponents.empty()) {
            sendError(d, (int)LSPErrorCodes::InvalidParams,
                      "Did not find any dispatchComponents for a SEND QueryResponse in "
                      "textDocument/hover");
            return;
        }
        string contents = "";
        for (auto &dispatchComponent : resp->dispatchComponents) {
            auto retType = resp->retType.type;
            if (resp->constraint) {
                retType = core::Types::instantiate(core::Context(*finalGs, core::Symbols::root()), retType,
                                                   *resp->constraint);
            }
            if (contents.size() > 0) {
                contents += " ";
            }
            contents +=
                methodDetail(*finalGs, dispatchComponent.method, dispatchComponent.receiver, retType, resp->constraint);
        }
        rapidjson::Value markupContents;
        markupContents.SetObject();
        // We use markdown here because if we just use a string, VSCode tries to interpret
        // things like <Class:Foo> as html tags and make them clickable (but the click takes
        // you somewhere nonsensical)
        markupContents.AddMember("kind", "markdown", alloc);
        markupContents.AddMember("value", contents, alloc);
        result.AddMember("contents", markupContents, alloc);
        sendResult(d, result);
    } else if (resp->kind == core::QueryResponse::Kind::IDENT || resp->kind == core::QueryResponse::Kind::CONSTANT ||
               resp->kind == core::QueryResponse::Kind::LITERAL) {
        rapidjson::Value markupContents;
        markupContents.SetObject();
        markupContents.AddMember("kind", "markdown", alloc);
        markupContents.AddMember("value", resp->retType.type->show(*finalGs), alloc);
        result.AddMember("contents", markupContents, alloc);
        sendResult(d, result);
    } else {
        sendError(d, (int)LSPErrorCodes::InvalidParams, "Unhandled QueryResponse kind in textDocument/hover");
    }
}

void LSPLoop::runLSP() {
    // Naming convention: thread that executes this function is called coordinator thread
    deque<rapidjson::Document> pendingRequests;
    bool terminate = false;
    std::mutex mtx;
    std::condition_variable cv;
    rapidjson::MemoryPoolAllocator<>
        inner_alloc; // we need objects created by inner thread to outlive the thread itself.

    auto readerThread = runInAThread([&pendingRequests, &mtx, &cv, &terminate, logger = this->logger, &inner_alloc] {
        // Thread that executes this lambda is called reader thread.
        // This thread _intentionally_ does not capture `this`.
        unique_lock<mutex> globalLock(mtx, defer_lock);
        while (!terminate) {
            rapidjson::Document d(&inner_alloc);
            if (!getNewRequest(d, logger)) {
                terminate = true;
                break;
            }
            unique_ptr<unique_lock<mutex>> lck = globalLock ? nullptr : make_unique<unique_lock<mutex>>(mtx);
            if (!pendingRequests.empty()) {
                // see if we can be smarter about requests that are waiting to be processed.

                const LSPMethod method = LSPMethod::getByName({d["method"].GetString(), d["method"].GetStringLength()});
                if (method == LSPMethod::Pause()) {
                    if (!globalLock) {
                        ENFORCE(lck);
                        globalLock = move(*lck);
                    }
                    continue;
                } else if (method == LSPMethod::Resume()) {
                    if (globalLock) {
                        globalLock.unlock();
                    }
                    continue;
                } else if (method == LSPMethod::TextDocumentDidChange()) {
                    // see if previous notification is modification of the same file, if so, take the most recent
                    // version.
                    // TODO: if we ever support diffs, this would need to be extended.
                    auto &prev = pendingRequests.back();
                    auto prevMethod =
                        LSPMethod::getByName({prev["method"].GetString(), prev["method"].GetStringLength()});
                    if (prevMethod == LSPMethod::TextDocumentDidChange()) {
                        string thisURI(d["params"]["textDocument"]["uri"].GetString(),
                                       d["params"]["textDocument"]["uri"].GetStringLength());
                        string prevURI(prev["params"]["textDocument"]["uri"].GetString(),
                                       prev["params"]["textDocument"]["uri"].GetStringLength());
                        if (prevURI == thisURI) {
                            pendingRequests.pop_back();
                        }
                    }
                } else if (method == LSPMethod::CancelRequest()) {
                    // see if they are cancelling request that we didn't yet even start processing.
                    for (auto &current : pendingRequests) {
                        auto fnd = current.FindMember("id");
                        if (fnd != current.MemberEnd()) {
                            if (d["params"]["id"].IsString()) {
                                if (current["id"].IsString() &&
                                    current["id"].GetString() == d["params"]["id"].GetString()) {
                                    current.AddMember("cancelled", move(d), current.GetAllocator());
                                    break;
                                }
                            } else if (d["params"]["id"].IsInt()) {
                                if (current["id"].IsInt() && current["id"].GetInt() == d["params"]["id"].GetInt()) {
                                    current.AddMember("cancelled", move(d), current.GetAllocator());
                                    break;
                                }
                            } else {
                                Error::raise("should never happen. Request id is neither int nor string.");
                            }
                        }
                    }

                    // if we started processing it already, well... swallow the cancellation request and continue
                    // computing.
                    continue;
                }
            }

            pendingRequests.emplace_back(move(d));
            cv.notify_one();
        }
        {
            // signal that coordinator thread should die
            unique_ptr<unique_lock<mutex>> lck = globalLock ? nullptr : make_unique<unique_lock<mutex>>(mtx);
            terminate = true;
            cv.notify_one();
        }
    });

    while (true) {
        rapidjson::Document doc;
        {
            unique_lock<mutex> lck(mtx);
            while (pendingRequests.empty() && !terminate) {
                cv.wait(lck);
            }
            if (terminate) {
                return;
            }
            doc.CopyFrom(pendingRequests.front(), alloc);
            pendingRequests.pop_front();
        }
        if (!handleReplies(doc)) {
            processRequest(doc);
        }
    }
}

bool LSPLoop::setupLSPQueryByLoc(rapidjson::Document &d, const LSPMethod &forMethod) {
    auto uri =
        string(d["params"]["textDocument"]["uri"].GetString(), d["params"]["textDocument"]["uri"].GetStringLength());

    auto fref = uri2FileRef(uri);
    if (!fref.exists()) {
        sendError(d, (int)LSPErrorCodes::InvalidParams,
                  fmt::format("Did not find file at uri {} in {}", uri, forMethod.name));
        return false;
    }
    core::Loc::Detail reqPos;
    reqPos.line = d["params"]["position"]["line"].GetInt() + 1;
    reqPos.column = d["params"]["position"]["character"].GetInt() + 1;
    auto reqPosOffset = core::Loc::pos2Offset(fref, reqPos, *finalGs);

    initialGS->lspInfoQueryLoc = core::Loc(fref, reqPosOffset, reqPosOffset);
    finalGs->lspInfoQueryLoc = core::Loc(fref, reqPosOffset, reqPosOffset);
    vector<shared_ptr<core::File>> files;
    files.emplace_back(make_shared<core::File>((std::move(fref.data(*finalGs)))));
    tryFastPath(files);

    initialGS->lspInfoQueryLoc = core::Loc::none();
    finalGs->lspInfoQueryLoc = core::Loc::none();
    return true;
}

/**
 * Represents information about programming constructs like variables, classes,
 * interfaces etc.
 *
 *        interface SymbolInformation {
 *                 // The name of this symbol.
 *                name: string;
 *                 // The kind of this symbol.
 *                kind: number;
 *
 *                 // Indicates if this symbol is deprecated.
 *                deprecated?: boolean;
 *
 *                 *
 *                 * The location of this symbol. The location's range is used by a tool
 *                 * to reveal the location in the editor. If the symbol is selected in the
 *                 * tool the range's start information is used to position the cursor. So
 *                 * the range usually spans more then the actual symbol's name and does
 *                 * normally include things like visibility modifiers.
 *                 *
 *                 * The range doesn't have to denote a node range in the sense of a abstract
 *                 * syntax tree. It can therefore not be used to re-construct a hierarchy of
 *                 * the symbols.
 *                 *
 *                location: Location;
 *
 *                 *
 *                 * The name of the symbol containing this symbol. This information is for
 *                 * user interface purposes (e.g. to render a qualifier in the user interface
 *                 * if necessary). It can't be used to re-infer a hierarchy for the document
 *                 * symbols.
 *
 *                containerName?: string;
 *        }
 **/
unique_ptr<rapidjson::Value> LSPLoop::symbolRef2SymbolInformation(core::SymbolRef symRef) {
    auto &sym = symRef.data(*finalGs);
    if (!sym.loc.file.exists()) {
        return nullptr;
    }
    rapidjson::Value result;
    result.SetObject();
    result.AddMember("name", sym.name.show(*finalGs), alloc);
    result.AddMember("location", loc2Location(sym.loc), alloc);
    result.AddMember("containerName", sym.owner.data(*finalGs).fullName(*finalGs), alloc);

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
            result.AddMember("kind", 2, alloc);
        }
        if (sym.isClassClass()) {
            result.AddMember("kind", 5, alloc);
        }
    } else if (sym.isMethod()) {
        if (sym.name == core::Names::initialize()) {
            result.AddMember("kind", 9, alloc);
        } else {
            result.AddMember("kind", 6, alloc);
        }
    } else if (sym.isField()) {
        result.AddMember("kind", 8, alloc);
    } else if (sym.isStaticField()) {
        result.AddMember("kind", 14, alloc);
    } else if (sym.isMethodArgument()) {
        result.AddMember("kind", 13, alloc);
    } else if (sym.isTypeMember()) {
        result.AddMember("kind", 26, alloc);
    } else if (sym.isTypeArgument()) {
        result.AddMember("kind", 26, alloc);
    } else {
        return nullptr;
    }

    return make_unique<rapidjson::Value>(move(result));
}

void LSPLoop::sendRaw(rapidjson::Document &raw) {
    if (raw.FindMember("jsonrpc") == raw.MemberEnd()) {
        raw.AddMember("jsonrpc", "2.0", alloc);
    }
    rapidjson::StringBuffer strbuf;
    rapidjson::Writer<rapidjson::StringBuffer> writer(strbuf);
    raw.Accept(writer);
    string outResult = fmt::format("Content-Length: {}\r\n\r\n{}", strbuf.GetLength(), strbuf.GetString());
    logger->debug("Write: {}\n", strbuf.GetString());
    std::cout << outResult << std::flush;
}

void LSPLoop::sendNotification(LSPMethod meth, rapidjson::Value &data) {
    ENFORCE(meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);
    rapidjson::Document request(&alloc);
    request.SetObject();
    string idStr = fmt::format("ruby-typer-req-{}", ++requestCounter);

    request.AddMember("method", meth.name, alloc);
    request.AddMember("params", data, alloc);

    sendRaw(request);
}

void LSPLoop::sendRequest(LSPMethod meth, rapidjson::Value &data, std::function<void(rapidjson::Value &)> onComplete,
                          std::function<void(rapidjson::Value &)> onFail) {
    ENFORCE(!meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);
    rapidjson::Document request(&alloc);
    request.SetObject();
    string idStr = fmt::format("ruby-typer-req-{}", ++requestCounter);

    request.AddMember("id", idStr, alloc);
    request.AddMember("method", meth.name, alloc);
    request.AddMember("params", data, alloc);

    awaitingResponse[idStr] = ResponseHandler{move(onComplete), move(onFail)};

    sendRaw(request);
}

bool silenceError(core::ErrorClass what) {
    if (what == core::errors::Namer::RedefinitionOfMethod ||
        what == core::errors::Resolver::DuplicateVariableDeclaration ||
        what == core::errors::Resolver::RedefinitionOfParents) {
        return true;
    }
    return false;
}

void LSPLoop::drainErrors() {
    for (auto &e : errorQueue->drainErrors()) {
        if (silenceError(e->what)) {
            continue;
        }
        auto file = e->loc.file;
        errorsAccumulated[file].emplace_back(move(e));

        if (!updatedErrors.empty() && updatedErrors.back() == file) {
            continue;
        }
        updatedErrors.emplace_back(file);
    }
    auto iter = errorsAccumulated.begin();
    for (; iter != errorsAccumulated.end();) {
        if (iter->first.data(*initialGS).source_type == core::File::TombStone) {
            errorsAccumulated.erase(iter++);
        } else {
            ++iter;
        }
    }
}

bool LSPLoop::ensureInitialized(LSPMethod forMethod, rapidjson::Document &d) {
    if (finalGs) {
        return true;
    }
    if (forMethod == LSPMethod::Initialize() || forMethod == LSPMethod::Initialized() ||
        forMethod == LSPMethod::Exit() || forMethod == LSPMethod::Shutdown()) {
        return true;
    }
    logger->error("Serving request before got an Initialize & Initialized handshake from IDE");
    if (!forMethod.isNotification) {
        sendError(d, (int)LSPErrorCodes::ServerNotInitialized,
                  "IDE did not initialize Sorbet correctly. No requests should be made before Initialize&Initialized "
                  "have been completed");
    }
    return false;
}

rapidjson::Value LSPLoop::loc2Range(core::Loc loc) {
    ENFORCE(loc.file.exists());

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
    ENFORCE(loc.file.exists());

    //  interface Location {
    //      uri: DocumentUri;
    //      range: Range;
    //  }
    rapidjson::Value ret;
    ret.SetObject();
    auto &messageFile = loc.file.data(*finalGs);
    string uri;
    if (messageFile.source_type == core::File::Type::Payload) {
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
        uri =
            fmt::format("{}#L{}", (string)messageFile.path(), std::to_string((int)(loc.position(*finalGs).first.line)));
    } else {
        uri = fileRef2Uri(loc.file);
    }

    ret.AddMember("uri", uri, alloc);
    ret.AddMember("range", loc2Range(loc), alloc);
    return ret;
}

void LSPLoop::pushErrors() {
    drainErrors();

    for (auto file : updatedErrors) {
        if (file.exists()) {
            rapidjson::Value publishDiagnosticsParams;

            /**
             * interface PublishDiagnosticsParams {
             *      uri: DocumentUri; // The URI for which diagnostic information is reported.
             *      diagnostics: Diagnostic[]; // An array of diagnostic information items.
             * }
             **/
            /** interface Diagnostic {
             *      range: Range; // The range at which the message applies.
             *      severity?: number; // The diagnostic's severity.
             *      code?: number | string; // The diagnostic's code
             *      source?: string; // A human-readable string describing the source of this diagnostic, e.g.
             *                       // 'typescript' or 'super lint'.
             *      message: string; // The diagnostic's message. relatedInformation?:
             *      DiagnosticRelatedInformation[]; // An array of related diagnostic information
             * }
             **/

            publishDiagnosticsParams.SetObject();
            { // uri
                string uriStr;
                if (file.data(*finalGs).source_type == core::File::Type::Payload) {
                    uriStr = (string)file.data(*finalGs).path();
                } else {
                    uriStr = fmt::format("{}/{}", rootUri, file.data(*finalGs).path());
                }
                publishDiagnosticsParams.AddMember("uri", uriStr, alloc);
            }

            {
                // diagnostics
                rapidjson::Value diagnostics;
                diagnostics.SetArray();
                for (auto &e : errorsAccumulated[file]) {
                    rapidjson::Value diagnostic;
                    diagnostic.SetObject();

                    diagnostic.AddMember("range", loc2Range(e->loc), alloc);
                    diagnostic.AddMember("code", e->what.code, alloc);
                    diagnostic.AddMember("message", e->formatted, alloc);

                    typecase(e.get(), [&](core::ComplexError *ce) {
                        rapidjson::Value relatedInformation;
                        relatedInformation.SetArray();
                        for (auto &section : ce->sections) {
                            string sectionHeader = section.header;

                            for (auto &errorLine : section.messages) {
                                rapidjson::Value relatedInfo;
                                relatedInfo.SetObject();
                                relatedInfo.AddMember("location", loc2Location(errorLine.loc), alloc);

                                string relatedInfoMessage;
                                if (errorLine.formattedMessage.length() > 0) {
                                    relatedInfoMessage = errorLine.formattedMessage;
                                } else {
                                    relatedInfoMessage = sectionHeader;
                                }
                                relatedInfo.AddMember("message", relatedInfoMessage, alloc);
                                relatedInformation.PushBack(relatedInfo, alloc);
                            }
                        }
                        diagnostic.AddMember("relatedInformation", relatedInformation, alloc);
                    });
                    diagnostics.PushBack(diagnostic, alloc);
                }
                publishDiagnosticsParams.AddMember("diagnostics", diagnostics, alloc);
            }

            sendNotification(LSPMethod::PushDiagnostics(), publishDiagnosticsParams);
        }
    }
    updatedErrors.clear();
}

void LSPLoop::sendResult(rapidjson::Document &forRequest, rapidjson::Value &result) {
    forRequest.AddMember("result", result, alloc);
    forRequest.RemoveMember("method");
    forRequest.RemoveMember("params");
    sendRaw(forRequest);
}

void LSPLoop::sendError(rapidjson::Document &forRequest, int errorCode, string errorStr) {
    forRequest.RemoveMember("method");
    forRequest.RemoveMember("params");
    forRequest.RemoveMember("cancelled");
    rapidjson::Value error;
    error.SetObject();
    error.AddMember("code", errorCode, alloc);
    rapidjson::Value message(errorStr.c_str(), alloc);
    error.AddMember("message", message, alloc);
    forRequest.AddMember("error", error, alloc);
    sendRaw(forRequest);
}

bool LSPLoop::handleReplies(rapidjson::Document &d) {
    if (d.FindMember("result") != d.MemberEnd()) {
        if (d.FindMember("id") != d.MemberEnd()) {
            string key(d["id"].GetString(), d["id"].GetStringLength());
            auto fnd = awaitingResponse.find(key);
            if (fnd != awaitingResponse.end()) {
                auto func = move(fnd->second.onResult);
                awaitingResponse.erase(fnd);
                func(d["result"]);
            }
        }
        return true;
    }

    if (d.FindMember("error") != d.MemberEnd()) {
        if (d.FindMember("id") != d.MemberEnd()) {
            string key(d["id"].GetString(), d["id"].GetStringLength());
            auto fnd = awaitingResponse.find(key);
            if (fnd != awaitingResponse.end()) {
                auto func = move(fnd->second.onError);
                awaitingResponse.erase(fnd);
                func(d["error"]);
            }
        }
        return true;
    }
    return false;
}

core::FileRef LSPLoop::addNewFile(const shared_ptr<core::File> &file) {
    core::FileRef fref;
    if (!file)
        return fref;
    fref = initialGS->findFileByPath(file->path());
    if (fref.exists()) {
        initialGS = core::GlobalState::replaceFile(move(initialGS), fref, move(file));
    } else {
        fref = initialGS->enterFile(move(file));
    }

    std::vector<std::string> emptyInputNames;
    auto t = pipeline::indexOne(opts, *initialGS, fref, kvstore, logger);
    int id = t->loc.file.id();
    if (id >= indexed.size()) {
        indexed.resize(id + 1);
    }
    indexed[id] = move(t);
    return fref;
}

vector<unique_ptr<ast::Expression>> incrementalResolve(core::GlobalState &gs, vector<unique_ptr<ast::Expression>> what,
                                                       const options::Options &opts,
                                                       shared_ptr<spdlog::logger> logger) {
    try {
        int i = 0;
        for (auto &tree : what) {
            auto file = tree->loc.file;
            try {
                unique_ptr<ast::Expression> ast;
                core::MutableContext ctx(gs, core::Symbols::root());
                logger->trace("Naming: {}", file.data(gs).path());
                core::ErrorRegion errs(gs, file);
                tree = sorbet::namer::Namer::run(ctx, move(tree));
                i++;
            } catch (SRubyException &) {
                if (auto e = gs.beginError(sorbet::core::Loc::none(file), core::errors::Internal::InternalError)) {
                    e.setHeader("Exception naming file: `{}` (backtrace is above)", file.data(gs).path());
                }
            }
        }

        core::MutableContext ctx(gs, core::Symbols::root());
        {
            Timer timeit(logger, "Incremental resolving");
            logger->trace("Resolving (incremental pass)...");
            core::ErrorRegion errs(gs, sorbet::core::FileRef());
            what = sorbet::resolver::Resolver::runTreePasses(ctx, move(what));
        }
    } catch (SRubyException &) {
        if (auto e = gs.beginError(sorbet::core::Loc::none(), sorbet::core::errors::Internal::InternalError)) {
            e.setHeader("Exception resolving (backtrace is above)");
        }
    }

    return what;
}

vector<unsigned int> LSPLoop::computeStateHashes(const vector<shared_ptr<core::File>> &files) {
    std::vector<unsigned int> res(files.size());
    shared_ptr<ConcurrentBoundedQueue<int>> fileq = make_shared<ConcurrentBoundedQueue<int>>(files.size());
    for (int i = 0; i < files.size(); i++) {
        auto copy = i;
        fileq->push(move(copy), 1);
    }

    logger->debug("Computing state hashes for {} files", files.size());

    res.resize(files.size());

    shared_ptr<BlockingBoundedQueue<vector<std::pair<int, unsigned int>>>> resultq =
        make_shared<BlockingBoundedQueue<vector<std::pair<int, unsigned int>>>>(files.size());
    workers.multiplexJob([fileq, resultq, files, logger = this->logger]() {
        vector<std::pair<int, unsigned int>> threadResult;
        int processedByThread = 0;
        int job;
        options::Options emptyOpts;
        emptyOpts.runLSP = true;

        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!files[job]) {
                        threadResult.emplace_back(make_pair(job, 0));
                        continue;
                    }
                    shared_ptr<core::GlobalState> lgs = make_shared<core::GlobalState>(
                        (std::make_shared<realmain::ConcurrentErrorQueue>(*logger, *logger)));
                    lgs->initEmpty();
                    lgs->silenceErrors = true;
                    core::UnfreezeFileTable fileTableAccess(*lgs);
                    core::UnfreezeSymbolTable symbolTable(*lgs);
                    core::UnfreezeNameTable nameTable(*lgs);
                    auto fref = lgs->enterFile(files[job]);
                    vector<unique_ptr<ast::Expression>> single;
                    unique_ptr<KeyValueStore> kvstore;
                    single.emplace_back(pipeline::indexOne(emptyOpts, *lgs, fref, kvstore, logger));
                    pipeline::resolve(*lgs, move(single), emptyOpts, logger, true);
                    threadResult.emplace_back(make_pair(job, lgs->hash()));
                }
            }
        }

        if (processedByThread > 0) {
            resultq->push(move(threadResult), processedByThread);
        }
    });

    {
        std::vector<std::pair<int, unsigned int>> threadResult;
        for (auto result = resultq->wait_pop_timed(threadResult, pipeline::PROGRESS_REFRESH_TIME_MILLIS);
             !result.done(); result = resultq->wait_pop_timed(threadResult, pipeline::PROGRESS_REFRESH_TIME_MILLIS)) {
            if (result.gotItem()) {
                for (auto &a : threadResult) {
                    res[a.first] = a.second;
                }
            }
        }
    }
    return res;
}

void LSPLoop::reIndexFromFileSystem() {
    indexed.clear();
    unordered_set<string> fileNamesDedup(opts.inputFileNames.begin(), opts.inputFileNames.end());
    for (int i = 1; i < initialGS->filesUsed(); i++) {
        core::FileRef f(i);
        if (f.data(*initialGS, true).source_type == core::File::Type::Normal) {
            fileNamesDedup.insert((string)f.data(*initialGS, true).path());
        }
    }
    std::vector<string> fileNames(make_move_iterator(fileNamesDedup.begin()), make_move_iterator(fileNamesDedup.end()));
    std::vector<core::FileRef> emptyInputFiles;
    for (auto &t : pipeline::index(initialGS, fileNames, emptyInputFiles, opts, workers, kvstore, logger)) {
        int id = t->loc.file.id();
        if (id >= indexed.size()) {
            indexed.resize(id + 1);
        }
        indexed[id] = move(t);
    }
}

void LSPLoop::invalidateAllErrors() {
    errorsAccumulated.clear();
    updatedErrors.clear();
}

void LSPLoop::invalidateErrorsFor(const vector<core::FileRef> &vec) {
    for (auto f : vec) {
        auto fnd = errorsAccumulated.find(f);
        if (fnd != errorsAccumulated.end()) {
            errorsAccumulated.erase(fnd);
        }
    }
}

void LSPLoop::runSlowPath(const std::vector<shared_ptr<core::File>>

                              &changedFiles) {
    logger->debug("Taking slow path");

    invalidateAllErrors();

    std::vector<core::FileRef> changedFileRefs;
    indexed.reserve(indexed.size() + changedFiles.size());
    for (auto &t : changedFiles) {
        addNewFile(t);
    }

    vector<unique_ptr<ast::Expression>> indexedCopies;
    for (const auto &tree : indexed) {
        if (tree) {
            indexedCopies.emplace_back(tree->deepCopy());
        }
    }

    finalGs = initialGS->deepCopy(true);
    tryApplyDefLocSaver(finalGs, indexedCopies);
    pipeline::typecheck(finalGs, pipeline::resolve(*finalGs, move(indexedCopies), opts, logger), opts, workers, logger);
}

void LSPLoop::tryApplyDefLocSaver(unique_ptr<core::GlobalState> &finalGs,
                                  vector<unique_ptr<ast::Expression>> &indexedCopies) {
    if (finalGs->lspInfoQueryLoc == core::Loc::none()) {
        return;
    }
    for (auto &t : indexedCopies) {
        sorbet::lsp::DefLocSaver defLocSaver;
        core::Context ctx(*finalGs, core::Symbols::root());
        t = ast::TreeMap::apply(ctx, defLocSaver, move(t));
    }
}

const std::vector<LSPMethod> LSPMethod::ALL_METHODS{CancelRequest(),
                                                    Initialize(),
                                                    Shutdown(),
                                                    Exit(),
                                                    RegisterCapability(),
                                                    UnRegisterCapability(),
                                                    DidChangeWatchedFiles(),
                                                    PushDiagnostics(),
                                                    TextDocumentDidOpen(),
                                                    TextDocumentDidChange(),
                                                    TextDocumentDocumentSymbol(),
                                                    TextDocumentDefinition(),
                                                    TextDocumentHover(),
                                                    TextDocumentCompletion(),
                                                    ReadFile(),
                                                    WorkspaceSymbolsRequest(),
                                                    CancelRequest(),
                                                    Pause(),
                                                    Resume()};

const LSPMethod LSPMethod::getByName(const absl::string_view name) {
    for (auto &candidate : ALL_METHODS) {
        if (candidate.name == name) {
            return candidate;
        }
    }
    return LSPMethod{(string)name, true, LSPMethod::Kind::ClientInitiated, false};
}

void LSPLoop::tryFastPath(std::vector<shared_ptr<core::File>>

                              &changedFiles) {
    logger->debug("Trying to see if happy path is available after {} file changes", changedFiles.size());
    bool good = true;
    auto hashes = computeStateHashes(changedFiles);
    ENFORCE(changedFiles.size() == hashes.size());
    vector<core::FileRef> subset;
    int i = -1;
    for (auto &f : changedFiles) {
        ++i;
        if (!f) {
            continue;
        }
        auto wasFiles = initialGS->filesUsed();
        auto fref = addNewFile(f);
        if (wasFiles != initialGS->filesUsed()) {
            logger->debug("Taking sad path because {} is a new file", changedFiles[i]->path());
            good = false;
            if (globalStateHashes.size() <= fref.id()) {
                globalStateHashes.resize(fref.id() + 1);
                globalStateHashes[fref.id()] = hashes[i];
            }
        } else {
            if (hashes[i] != globalStateHashes[fref.id()]) {
                logger->debug("Taking sad path because {} has changed definitions", changedFiles[i]->path());
                good = false;
                globalStateHashes[fref.id()] = hashes[i];
            }
            if (good) {
                finalGs = core::GlobalState::replaceFile(move(finalGs), fref, changedFiles[i]);
            }

            subset.emplace_back(fref);
        }
    }

    if (good) {
        invalidateErrorsFor(subset);
        logger->debug("Taking happy path");

        std::vector<std::unique_ptr<ast::Expression>> updatedIndexed;
        for (auto &f : subset) {
            auto t = pipeline::indexOne(opts, *finalGs, f, kvstore, logger);
            int id = t->loc.file.id();
            indexed[id] = move(t);
            updatedIndexed.emplace_back(indexed[id]->deepCopy());
        }

        tryApplyDefLocSaver(finalGs, updatedIndexed);
        pipeline::typecheck(finalGs, incrementalResolve(*finalGs, move(updatedIndexed), opts, logger), opts, workers,
                            logger);
    } else {
        runSlowPath(changedFiles);
    }
}

std::string LSPLoop::remoteName2Local(const absl::string_view uri) {
    ENFORCE(startsWith(uri, rootUri));
    const char *start = uri.data() + rootUri.length();
    if (*start == '/') {
        ++start;
    }
    return string(start, uri.end());
}

std::string LSPLoop::localName2Remote(const absl::string_view uri) {
    ENFORCE(!startsWith(uri, rootUri));
    return absl::StrCat(rootUri, "/", uri);
}

core::FileRef LSPLoop::uri2FileRef(const absl::string_view uri) {
    if (!startsWith(uri, rootUri)) {
        return core::FileRef();
    }
    auto needle = remoteName2Local(uri);
    return initialGS->findFileByPath(needle);
}

std::string LSPLoop::fileRef2Uri(core::FileRef file) {
    if (file.data(*finalGs).source_type == core::File::Type::Payload) {
        return (string)file.data(*finalGs).path();
    } else {
        return localName2Remote((string)file.data(*finalGs).path());
    }
}

} // namespace lsp
} // namespace realmain
} // namespace sorbet
