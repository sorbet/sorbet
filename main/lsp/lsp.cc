#include "lsp.h"
#include "absl/strings/str_cat.h"
#include "ast/treemap/treemap.h"
#include "common/Timer.h"
#include "core/Errors.h"
#include "core/Files.h"
#include "core/Names/resolver.h"
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

bool hideSymbol(core::GlobalState &gs, core::SymbolRef sym) {
    if (!sym.exists() || sym == core::Symbols::root()) {
        return true;
    }
    auto &data = sym.data(gs);
    if (data.isClass() && data.attachedClass(gs).exists()) {
        return true;
    }
    if (data.isClass() && data.superClass == core::Symbols::StubClass()) {
        return true;
    }
    if (data.isMethodArgument() && data.isBlockArgument()) {
        return true;
    }
    if (data.name.data(gs).kind == core::NameKind::UNIQUE &&
        data.name.data(gs).unique.original == core::Names::staticInit()) {
        return true;
    }
    if (data.name.data(gs).kind == core::NameKind::UNIQUE &&
        data.name.data(gs).unique.original == core::Names::blockTemp()) {
        return true;
    }
    return false;
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
                               "       \"signatureHelpProvider\": { "
                               "           \"triggerCharacters\": [\"(\", \",\"]"
                               "       },"
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
                if (!hideSymbol(*finalGs, ref) && (ref.data(*finalGs).owner.data(*finalGs).loc().file != fref ||
                                                   ref.data(*finalGs).owner == core::Symbols::root())) {
                    for (auto definitionLocation : ref.data(*finalGs).locs()) {
                        if (definitionLocation.file == fref) {
                            auto data = symbolRef2DocumentSymbol(ref, fref);
                            if (data) {
                                result.PushBack(move(*data), alloc);
                                break;
                            }
                        }
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
        } else if (method == LSPMethod::TextDocumentSignatureHelp()) {
            handleTextSignatureHelp(result, d);
        } else {
            ENFORCE(!method.isSupported, "failing a supported method");
            errorCode = (int)LSPErrorCodes::MethodNotFound;
            errorString = fmt::format("Unknown method: {}", method.name);
            sendError(d, errorCode, errorString);
        }
    }
}

void addSignatureHelpItem(core::GlobalState &gs, rapidjson::MemoryPoolAllocator<> &alloc, rapidjson::Value &signatures,
                          core::SymbolRef method, const core::QueryResponse &resp, int activeParameter) {
    // signature helps only exist for methods.
    if (!method.exists() || !method.data(gs).isMethod() || hideSymbol(gs, method)) {
        return;
    }
    rapidjson::Value sig;
    sig.SetObject();
    // Label is mandatory, so method name (i.e B#add) is shown for now. Might want to add markup highlighting
    // wtih respect to activeParameter here.
    sig.AddMember("label", (string)method.data(gs).show(gs), alloc);
    rapidjson::Value parameters;
    parameters.SetArray();
    // Documentation is set to be a markdown element that highlights which parameter you are currently typing in.
    string methodDocumentation = "(";
    auto args = method.data(gs).arguments();
    int i = 0;
    for (auto arg : args) {
        rapidjson::Value parameter;
        parameter.SetObject();
        // label field is populated with the name of the variable.
        // Not sure why VSCode does not display this for now.
        parameter.AddMember("label", (string)arg.data(gs).name.show(gs), alloc);
        if (i == activeParameter) {
            // this bolds the active parameter in markdown
            methodDocumentation += "**_" + (string)arg.data(gs).name.show(gs) + "_**";
        } else {
            methodDocumentation += (string)arg.data(gs).name.show(gs);
        }
        if (i != args.size() - 1) {
            methodDocumentation += ", ";
        }
        parameter.AddMember("documentation",
                            (string)getResultType(gs, arg, resp.receiver.type, resp.constraint)->show(gs), alloc);
        parameters.PushBack(move(parameter), alloc);
        i += 1;
    }
    methodDocumentation += ")";
    rapidjson::Value markupContents;
    markupContents.SetObject();
    markupContents.AddMember("kind", "markdown", alloc);
    markupContents.AddMember("value", methodDocumentation, alloc);
    sig.AddMember("documentation", markupContents, alloc);

    sig.AddMember("parameters", move(parameters), alloc);
    signatures.PushBack(move(sig), alloc);
}

void LSPLoop::handleTextSignatureHelp(rapidjson::Value &result, rapidjson::Document &d) {
    result.SetObject();
    rapidjson::Value signatures;
    signatures.SetArray();

    if (setupLSPQueryByLoc(d, LSPMethod::TextDocumentSignatureHelp(), false)) {
        auto queryResponses = errorQueue->drainQueryResponses();
        if (!queryResponses.empty()) {
            auto resp = std::move(queryResponses[0]);
            auto receiverType = resp->receiver.type;
            // only triggers on sends. Some SignatureHelps are triggered when the variable is being typed.
            if (resp->kind == core::QueryResponse::Kind::SEND) {
                auto sendLocIndex = resp->termLoc.beginPos;

                auto uri = string(d["params"]["textDocument"]["uri"].GetString(),
                                  d["params"]["textDocument"]["uri"].GetStringLength());
                auto fref = uri2FileRef(uri);
                if (!fref.exists()) {
                    return;
                }
                auto src = fref.data(*finalGs).source();
                auto loc = lspPos2Loc(fref, d, *finalGs);
                if (!loc) {
                    return;
                }
                string call_str = (string)src.substr(sendLocIndex, loc->endPos - sendLocIndex);
                int numberCommas = count(call_str.begin(), call_str.end(), ',');
                // Active parameter depends on number of ,'s in the current string being typed. (0 , = first arg, 1 , =
                // 2nd arg)
                result.AddMember("activeParameter", numberCommas, alloc);

                auto firstDispatchComponentMethod = resp->dispatchComponents.front().method;
                addSignatureHelpItem(*finalGs, alloc, signatures, firstDispatchComponentMethod, *resp, numberCommas);
            }
        }
    }

    result.AddMember("signatures", move(signatures), alloc);
    sendResult(d, result);
}

unique_ptr<core::Loc> LSPLoop::lspPos2Loc(core::FileRef fref, rapidjson::Document &d, const core::GlobalState &gs) {
    if (!d.HasMember("params") || !d["params"].HasMember("position") ||
        !d["params"]["position"].HasMember("character") || !d["params"]["position"].HasMember("line")) {
        sendError(d, (int)LSPErrorCodes::InvalidRequest,
                  "Request must have a \"params\" field that has a nested \"position\", with nested \"line\" and "
                  "\"character\"");
        return nullptr;
    }
    core::Loc::Detail reqPos;
    reqPos.line = d["params"]["position"]["line"].GetInt() + 1;
    reqPos.column = d["params"]["position"]["character"].GetInt() + 1;
    auto offset = core::Loc::pos2Offset(fref, reqPos, *finalGs);
    return make_unique<core::Loc>(core::Loc(fref, offset, offset));
}

void LSPLoop::addLocIfExists(rapidjson::Value &result, core::Loc loc) {
    if (loc.file.exists()) {
        result.PushBack(loc2Location(loc), alloc);
    }
}

void LSPLoop::handleTextDocumentDefinition(rapidjson::Value &result, rapidjson::Document &d) {
    result.SetArray();
    if (setupLSPQueryByLoc(d, LSPMethod::TextDocumentDefinition(), true)) {
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
                        addLocIfExists(result, component.method.data(*finalGs).loc());
                    }
                }
            }
        }
    }
    sendResult(d, result);
}

UnorderedMap<core::NameRef, vector<core::SymbolRef>>
mergeMaps(UnorderedMap<core::NameRef, vector<core::SymbolRef>> &&first,
          UnorderedMap<core::NameRef, vector<core::SymbolRef>> &&second) {
    for (auto &other : second) {
        first[other.first].insert(first[other.first].end(), make_move_iterator(other.second.begin()),
                                  make_move_iterator(other.second.end()));
    }
    return first;
};

UnorderedMap<core::NameRef, vector<core::SymbolRef>>
findSimilarMethodsIn(core::GlobalState &gs, shared_ptr<core::Type> receiver, absl::string_view name) {
    UnorderedMap<core::NameRef, vector<core::SymbolRef>> result;
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
    ENFORCE(method.exists());
    // handle this case anyways so that we don't crash in prod when this method is mis-used
    if (!method.exists()) {
        return "";
    }

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

unique_ptr<string> findDocumentation(absl::string_view sourceCode, int beginIndex) {
    // everything in the file before the method definition.
    auto preDefinition = sourceCode.substr(0, sourceCode.rfind('\n', beginIndex));

    int last_newline_loc = preDefinition.rfind('\n');
    // if there is no '\n' in preDefinition, we're at the top of the file.
    if (last_newline_loc == preDefinition.npos) {
        return nullptr;
    }
    auto prevLine = preDefinition.substr(last_newline_loc, preDefinition.size() - last_newline_loc);
    if (prevLine.find('#') == prevLine.npos) {
        return nullptr;
    }

    std::string documentation = "";
    // keep looking for previous newline locations, searching for lines with # in them.
    while (prevLine.find('#') != prevLine.npos) {
        documentation = (string)prevLine.substr(prevLine.find('#') + 1, prevLine.size()) + "\n" + documentation;
        int prev_newline_loc = preDefinition.rfind('\n', last_newline_loc - 1);
        // if there is no '\n', we're at the top of the file, so just return documentation.
        if (prev_newline_loc == preDefinition.npos) {
            break;
        }
        prevLine = preDefinition.substr(prev_newline_loc, last_newline_loc - prev_newline_loc);
        last_newline_loc = prev_newline_loc;
    }
    if (documentation.empty()) {
        return nullptr;
    }
    return make_unique<string>(documentation);
}

void addCompletionItem(core::GlobalState &gs, rapidjson::MemoryPoolAllocator<> &alloc, rapidjson::Value &items,
                       core::SymbolRef what, const core::QueryResponse &resp) {
    ENFORCE(what.exists());
    rapidjson::Value item;
    item.SetObject();
    item.AddMember("label", (string)what.data(gs).name.data(gs).shortName(gs), alloc);
    auto resultType = what.data(gs).resultType;
    if (!resultType) {
        resultType = core::Types::untyped();
    }
    if (what.data(gs).isMethod()) {
        item.AddMember("kind", 3, alloc); // Function
        if (what.exists()) {
            item.AddMember("detail", methodDetail(gs, what, resp.receiver.type, nullptr, resp.constraint), alloc);
        }
        item.AddMember("insertTextFormat", 2, alloc); // Snippet
        item.AddMember("insertText", methodSnippet(gs, what), alloc);

        unique_ptr<string> documentation = nullptr;
        if (what.data(gs).loc().file.exists()) {
            documentation = findDocumentation(what.data(gs).loc().file.data(gs).source(), what.data(gs).loc().beginPos);
        }
        if (documentation) {
            if (documentation->find("@deprecated") != documentation->npos) {
                item.AddMember("deprecated", true, alloc);
            }
            item.AddMember("documentation", move(*documentation), alloc);
        }

    } else if (what.data(gs).isStaticField()) {
        item.AddMember("kind", 21, alloc); // Constant
        item.AddMember("detail", resultType->show(gs), alloc);
    } else if (what.data(gs).isClass()) {
        item.AddMember("kind", 7, alloc); // Class
    }
    items.PushBack(move(item), alloc);
}

void LSPLoop::handleTextDocumentCompletion(rapidjson::Value &result, rapidjson::Document &d) {
    result.SetObject();
    result.AddMember("isIncomplete", "false", alloc);
    rapidjson::Value items;
    items.SetArray();

    if (setupLSPQueryByLoc(d, LSPMethod::TextDocumentCompletion(), false)) {
        auto queryResponses = errorQueue->drainQueryResponses();
        if (!queryResponses.empty()) {
            auto resp = std::move(queryResponses[0]);

            auto receiverType = resp->receiver.type;
            if (resp->kind == core::QueryResponse::Kind::SEND) {
                auto pattern = resp->name.data(*finalGs).shortName(*finalGs);
                logger->debug("Looking for method similar to {}", pattern);
                UnorderedMap<core::NameRef, vector<core::SymbolRef>> methods =
                    findSimilarMethodsIn(*finalGs, receiverType, pattern);
                for (auto &entry : methods) {
                    if (entry.second[0].exists()) {
                        addCompletionItem(*finalGs, alloc, items, entry.second[0], *resp);
                    }
                }
            } else if (resp->kind == core::QueryResponse::Kind::IDENT ||
                       resp->kind == core::QueryResponse::Kind::CONSTANT) {
                if (auto c = core::cast_type<core::ClassType>(receiverType.get())) {
                    auto pattern = c->symbol.data(*finalGs).name.data(*finalGs).shortName(*finalGs);
                    logger->debug("Looking for constant similar to {}", pattern);
                    core::SymbolRef owner = c->symbol;
                    do {
                        owner = owner.data(*finalGs).owner;
                        for (auto member : owner.data(*finalGs).members) {
                            auto sym = member.second;
                            if (sym.exists() && (sym.data(*finalGs).isClass() || sym.data(*finalGs).isStaticField()) &&
                                sym.data(*finalGs).name.data(*finalGs).kind == core::NameKind::CONSTANT &&
                                // hide singletons
                                hasSimilarName(*finalGs, sym.data(*finalGs).name, pattern) &&
                                !sym.data(*finalGs).derivesFrom(*finalGs, core::Symbols::StubClass())) {
                                addCompletionItem(*finalGs, alloc, items, sym, *resp);
                            }
                        }
                    } while (owner != core::Symbols::root());
                }
            } else {
            }
        }
        result.AddMember("items", move(items), alloc);
    }
    sendResult(d, result);
}

void LSPLoop::handleTextDocumentHover(rapidjson::Value &result, rapidjson::Document &d) {
    result.SetObject();

    if (setupLSPQueryByLoc(d, LSPMethod::TextDocumentHover(), false)) {
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
                if (dispatchComponent.method.exists()) {
                    if (contents.size() > 0) {
                        contents += " ";
                    }
                    contents += methodDetail(*finalGs, dispatchComponent.method, dispatchComponent.receiver, retType,
                                             resp->constraint);
                }
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
        } else if (resp->kind == core::QueryResponse::Kind::IDENT ||
                   resp->kind == core::QueryResponse::Kind::CONSTANT ||
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

bool LSPLoop::setupLSPQueryByLoc(rapidjson::Document &d, const LSPMethod &forMethod, bool errorIfFileIsUntyped) {
    auto uri =
        string(d["params"]["textDocument"]["uri"].GetString(), d["params"]["textDocument"]["uri"].GetStringLength());

    auto fref = uri2FileRef(uri);
    if (!fref.exists()) {
        sendError(d, (int)LSPErrorCodes::InvalidParams,
                  fmt::format("Did not find file at uri {} in {}", uri, forMethod.name));
        return false;
    }

    if (errorIfFileIsUntyped && fref.data(*finalGs).sigil == core::StrictLevel::Stripe) {
        sendError(d, (int)LSPErrorCodes::InvalidParams, "This feature only works correctly on typed ruby files.");
        sendShowMessageNotification(
            1, "This feature only works correctly on typed ruby files. Results you see may be heuristic results.");
        return false;
    }

    initialGS->lspInfoQueryLoc = *lspPos2Loc(fref, d, *finalGs);
    finalGs->lspInfoQueryLoc = *lspPos2Loc(fref, d, *finalGs);
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
    if (!sym.loc().file.exists() || hideSymbol(*finalGs, symRef)) {
        return nullptr;
    }
    rapidjson::Value result;
    result.SetObject();
    result.AddMember("name", sym.name.show(*finalGs), alloc);
    result.AddMember("location", loc2Location(sym.loc()), alloc);
    result.AddMember("containerName", sym.owner.data(*finalGs).fullName(*finalGs), alloc);
    result.AddMember("kind", symbolRef2SymbolKind(symRef), alloc);

    return make_unique<rapidjson::Value>(move(result));
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

void LSPLoop::symbolRef2DocumentSymbolWalkMembers(core::SymbolRef sym, core::FileRef filter, rapidjson::Value &out) {
    for (auto mem : sym.data(*finalGs).members) {
        if (mem.first != core::Names::attached() && mem.first != core::Names::singleton()) {
            bool foundThisFile = false;
            for (auto loc : mem.second.data(*finalGs).locs()) {
                foundThisFile = foundThisFile || loc.file == filter;
            }
            if (!foundThisFile) {
                continue;
            }
            auto inner = LSPLoop::symbolRef2DocumentSymbol(mem.second, filter);
            if (inner) {
                out.PushBack(*inner, alloc);
            }
        }
    }
}

/**
 *
 * Represents programming constructs like variables, classes, interfaces etc. that appear in a document. Document
 * symbols can be hierarchical and they have two ranges: one that encloses its definition and one that points to its
 * most interesting range, e.g. the range of an identifier.
 *
 *  export class DocumentSymbol {
 *
 *       // The name of this symbol.
 *      name: string;
 *
 *       // More detail for this symbol, e.g the signature of a function. If not provided the
 *       // name is used.
 *      detail?: string;
 *
 *       // The kind of this symbol.
 *      kind: SymbolKind;
 *
 *       // Indicates if this symbol is deprecated.
 *      deprecated?: boolean;
 *
 *      //  The range enclosing this symbol not including leading/trailing whitespace but everything else
 *      //  like comments. This information is typically used to determine if the the clients cursor is
 *      //  inside the symbol to reveal in the symbol in the UI.
 *      range: Range;
 *
 *      //  The range that should be selected and revealed when this symbol is being picked, e.g the name of a function.
 *      //  Must be contained by the the `range`.
 *      selectionRange: Range;
 *
 *      //  Children of this symbol, e.g. properties of a class.
 *      children?: DocumentSymbol[];
 *  }
 */

unique_ptr<rapidjson::Value> LSPLoop::symbolRef2DocumentSymbol(core::SymbolRef symRef, core::FileRef filter) {
    if (!symRef.exists()) {
        return nullptr;
    }
    auto &sym = symRef.data(*finalGs);
    if (!sym.loc().file.exists() || hideSymbol(*finalGs, symRef)) {
        return nullptr;
    }
    rapidjson::Value result;
    result.SetObject();
    string prefix = "";
    if (sym.owner.exists() && sym.owner.data(*finalGs).isClass() &&
        sym.owner.data(*finalGs).attachedClass(*finalGs).exists()) {
        prefix = "self.";
    }
    result.AddMember("name", prefix + sym.name.show(*finalGs), alloc);
    if (sym.isMethod()) {
        result.AddMember("detail", methodDetail(*finalGs, symRef, nullptr, nullptr, nullptr), alloc);
    } else {
        // Currently released version of VSCode has a bug that requires this non-optional field to be present
        result.AddMember("detail", "", alloc);
    }
    result.AddMember("kind", symbolRef2SymbolKind(symRef), alloc);
    result.AddMember("range", loc2Range(sym.loc()), alloc); // TODO: this range should cover body. Currently it doesn't.
    result.AddMember("selectionRange", loc2Range(sym.loc()), alloc);

    rapidjson::Value children;
    children.SetArray();
    symbolRef2DocumentSymbolWalkMembers(symRef, filter, children);
    if (sym.isClass()) {
        auto singleton = sym.lookupSingletonClass(*finalGs);
        if (singleton.exists()) {
            symbolRef2DocumentSymbolWalkMembers(singleton, filter, children);
        }
    }
    result.AddMember("children", children, alloc);

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
        if (iter->first.data(*initialGS).sourceType == core::File::TombStone) {
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
    if (!loc.file.exists()) {
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

    if (!loc.file.exists()) {
        uri = localName2Remote("???");
    } else {
        auto &messageFile = loc.file.data(*finalGs);
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
            uri = fileRef2Uri(loc.file);
        }
    }

    ret.AddMember("uri", uri, alloc);
    ret.AddMember("range", loc2Range(loc), alloc);

    return ret;
}

void LSPLoop::pushErrors() {
    drainErrors();

    // Dedup updates
    sort(updatedErrors.begin(), updatedErrors.end());
    updatedErrors.erase(unique(updatedErrors.begin(), updatedErrors.end()), updatedErrors.end());

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
                if (file.data(*finalGs).sourceType == core::File::Type::Payload) {
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
                if (errorsAccumulated.find(file) != errorsAccumulated.end()) {
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
                }
                publishDiagnosticsParams.AddMember("diagnostics", diagnostics, alloc);
            }

            sendNotification(LSPMethod::PushDiagnostics(), publishDiagnosticsParams);
        }
    }
    updatedErrors.clear();
}

void LSPLoop::sendShowMessageNotification(int messageType, string message) {
    rapidjson::Value result;
    result.SetObject();
    result.AddMember("type", messageType, alloc);
    result.AddMember("message", message, alloc);
    sendNotification(LSPMethod::WindowShowMessage(), result);
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

bool LSPLoop::isTestFile(const shared_ptr<core::File> &file) {
    if (file->path().find("/test/") != file->path().npos) {
        return true;
    } else if (file->path().find_first_of("test/") == 0) {
        return true;
    } else {
        return false;
    }
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
        if (f.data(*initialGS, true).sourceType == core::File::Type::Normal) {
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
    updatedErrors.clear();
    for (auto &e : errorsAccumulated) {
        updatedErrors.push_back(e.first);
    }
    errorsAccumulated.clear();
}

void LSPLoop::invalidateErrorsFor(const vector<core::FileRef> &vec) {
    for (auto f : vec) {
        auto fnd = errorsAccumulated.find(f);
        if (fnd != errorsAccumulated.end()) {
            errorsAccumulated.erase(fnd);
            updatedErrors.push_back(f);
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
        if (!isTestFile(t)) {
            addNewFile(t);
        }
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
                                                    TextDocumentSignatureHelp(),
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
        if (!f || isTestFile(f)) {
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
    if (file.data(*finalGs).sourceType == core::File::Type::Payload) {
        return (string)file.data(*finalGs).path();
    } else {
        return localName2Remote((string)file.data(*finalGs).path());
    }
}

} // namespace lsp
} // namespace realmain
} // namespace sorbet
