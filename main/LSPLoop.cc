#include "LSPLoop.h"
#include "../core/Files.h"
#include "absl/strings/str_cat.h"
#include "core/ErrorQueue.h"
#include "core/Files.h"
#include "core/Unfreeze.h"
#include "core/errors/namer.h"
#include "core/errors/resolver.h"
#include "spdlog/fmt/ostr.h"

using namespace std;
using namespace rapidjson;
using namespace sorbet::realmain::LSP;

namespace sorbet {
namespace realmain {

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

void LSPLoop::runLSP() {
    std::string json;
    Document d(&alloc);

    while (true) {
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
            logger->info("eof");
            return;
        }

        json.resize(length);
        cin.read(&json[0], length);

        logger->info("Read: {}", json);
        if (d.Parse(json.c_str()).HasParseError()) {
            logger->info("json parse error");
            return;
        }

        if (handleReplies(d)) {
            continue;
        }

        const LSPMethod method = LSP::getMethod({d["method"].GetString(), d["method"].GetStringLength()});

        ENFORCE(method.kind == LSPMethod::Kind::ClientInitiated || method.kind == LSPMethod::Kind::Both);
        if (method.isNotification) {
            logger->info("Processing notification {} ", (string)method.name);
            if (method == LSP::DidChangeWatchedFiles) {
                sendRequest(LSP::ReadFile, d["params"],
                            [&](Value &edits) -> void {
                                ENFORCE(edits.IsArray());
                                Timer timeit(logger, "handle update");
                                vector<shared_ptr<core::File>> files;

                                for (auto it = edits.Begin(); it != edits.End(); ++it) {
                                    Value &change = *it;
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
                            [](Value &error) -> void {});
            }
            if (method == LSP::TextDocumentDidChange) {
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
                if (startsWith(uri, rootUri)) {
                    files.emplace_back(
                        make_shared<core::File>(remoteName2Local(uri), move(content), core::File::Type::Normal));

                    tryFastPath(files);
                    pushErrors();
                }
            }
            if (method == LSP::TextDocumentDidOpen) {
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
                string content(edits["textDocument"]["text"].GetString(),
                               edits["textDocument"]["text"].GetStringLength());
                if (startsWith(uri, rootUri)) {
                    files.emplace_back(
                        make_shared<core::File>(remoteName2Local(uri), move(content), core::File::Type::Normal));

                    tryFastPath(files);
                    pushErrors();
                }
            }
            if (method == LSP::Inititalized) {
                // initialize ourselves
                {
                    Timer timeit(logger, "index");
                    reIndexFromFileSystem();
                    vector<shared_ptr<core::File>> changedFiles;
                    runSlowPath(move(changedFiles));
                    pushErrors();
                    this->globalStateHashes = computeStateHashes(finalGs->getFiles());
                }
            }
            if (method == LSP::Exit) {
                return;
            }
        } else {
            logger->info("Processing request {}", method.name);
            // is request
            Value result;
            int errorCode = 0;
            string errorString;
            if (method == LSP::Initialize) {
                result.SetObject();
                rootUri = string(d["params"]["rootUri"].GetString(), d["params"]["rootUri"].GetStringLength());
                string serverCap = "{\"capabilities\": {\"textDocumentSync\": 1, \"documentSymbolProvider\": true, "
                                   "\"workspaceSymbolProvider\": true, \"definitionProvider\": true}}";
                Document temp;
                auto &r = temp.Parse(serverCap.c_str());
                ENFORCE(!r.HasParseError());
                result.CopyFrom(temp, alloc);
            } else if (method == LSP::Shutdown) {
                // return default value: null
            } else if (method == LSP::TextDocumentDocumentSymbol) {
                result.SetArray();
                auto uri = string(d["params"]["textDocument"]["uri"].GetString(),
                                  d["params"]["textDocument"]["uri"].GetStringLength());
                auto fref = uri2FileRef(uri);
                for (u4 idx = 1; idx < finalGs->symbolsUsed(); idx++) {
                    core::SymbolRef ref(finalGs.get(), idx);
                    if (ref.data(*finalGs).definitionLoc.file == fref) {
                        auto data = symbolRef2SymbolInformation(ref);
                        if (data) {
                            result.PushBack(move(*data), alloc);
                        }
                    }
                }

            } else if (method == LSP::WorkspaceSymbolsRequest) {
                result.SetArray();
                string searchString = d["params"]["query"].GetString();

                for (u4 idx = 1; idx < finalGs->symbolsUsed(); idx++) {
                    core::SymbolRef ref(finalGs.get(), idx);
                    if (ref.data(*finalGs).name.show(*finalGs).compare(searchString) == 0) {
                        auto data = symbolRef2SymbolInformation(ref);
                        if (data) {
                            result.PushBack(move(*data), alloc);
                        }
                    }
                }
            } else if (method == LSP::TextDocumentDefinition) {
                result.SetArray();

                auto uri = string(d["params"]["textDocument"]["uri"].GetString(),
                                  d["params"]["textDocument"]["uri"].GetStringLength());
                auto fref = uri2FileRef(uri);
                if (fref.exists()) {
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

                    auto queryResponses = finalGs->errorQueue->drainQueryResponses();
                    if (!queryResponses.empty()) {
                        auto resp = std::move(queryResponses[0]);

                        if (resp->kind == core::QueryResponse::Kind::SEND) {
                            for (auto &component : resp->dispatchComponents) {
                                if (component.method.exists()) {
                                    result.PushBack(loc2Location(component.method.data(*finalGs).definitionLoc), alloc);
                                }
                            }
                        } else if (resp->kind == core::QueryResponse::Kind::IDENT) {
                            result.PushBack(loc2Location(resp->retType.origins[0]), alloc);
                        } else {
                            for (auto &component : resp->dispatchComponents) {
                                if (component.method.exists()) {
                                    result.PushBack(loc2Location(component.method.data(*finalGs).definitionLoc), alloc);
                                }
                            }
                        }
                    }
                }

            } else {
                ENFORCE(!method.isSupported, "failing a supported method");
                errorCode = (int)LSP::LSPErrorCodes::MethodNotFound;
                errorString = fmt::format("Unknown method: {}", method.name);
            }

            if (errorCode == 0) {
                sendResult(d, result);
            } else {
                sendError(d, errorCode, errorString);
            }
        }
    }
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
    if (!sym.definitionLoc.file.exists()) {
        return nullptr;
    }
    rapidjson::Value result;
    result.SetObject();
    result.AddMember("name", sym.name.show(*finalGs), alloc);
    result.AddMember("location", loc2Location(sym.definitionLoc), alloc);
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

void LSPLoop::sendRaw(Document &raw) {
    StringBuffer strbuf;
    Writer<StringBuffer> writer(strbuf);
    raw.Accept(writer);
    string outResult = fmt::format("Content-Length: {}\r\n\r\n{}", strbuf.GetLength(), strbuf.GetString());
    logger->info("Write: {}", strbuf.GetString());
    logger->info("\n");
    std::cout << outResult << std::flush;
}

void LSPLoop::sendNotification(LSPMethod meth, Value &data) {
    ENFORCE(meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);
    Document request(&alloc);
    request.SetObject();
    string idStr = fmt::format("ruby-typer-req-{}", ++requestCounter);

    request.AddMember("method", meth.name, alloc);
    request.AddMember("params", data, alloc);

    sendRaw(request);
}
void LSPLoop::sendRequest(LSPMethod meth, Value &data, std::function<void(Value &)> onComplete,
                          std::function<void(Value &)> onFail) {
    ENFORCE(!meth.isNotification);
    ENFORCE(meth.kind == LSPMethod::Kind::ServerInitiated || meth.kind == LSPMethod::Kind::Both);
    Document request(&alloc);
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
    for (auto &e : initialGS->errorQueue->drainErrors()) {
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

Value LSPLoop::loc2Range(core::Loc loc) {
    /**
       {
        start: { line: 5, character: 23 }
        end : { line 6, character : 0 }
        }
     */
    Value ret;
    ret.SetObject();
    Value start;
    start.SetObject();
    Value end;
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

Value LSPLoop::loc2Location(core::Loc loc) {
    //  interface Location {
    //      uri: DocumentUri;
    //      range: Range;
    //  }
    Value ret;
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
            Value publishDiagnosticsParams;

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
                Value diagnostics;
                diagnostics.SetArray();
                for (auto &e : errorsAccumulated[file]) {
                    Value diagnostic;
                    diagnostic.SetObject();

                    diagnostic.AddMember("range", loc2Range(e->loc), alloc);
                    diagnostic.AddMember("code", e->what.code, alloc);
                    diagnostic.AddMember("message", e->formatted, alloc);

                    typecase(e.get(), [&](core::ComplexError *ce) {
                        Value relatedInformation;
                        relatedInformation.SetArray();
                        for (auto &section : ce->sections) {
                            Value relatedInfo;
                            relatedInfo.SetObject();
                            string sectionHeader = section.header;

                            for (auto &errorLine : section.messages) {
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

            sendNotification(LSP::PushDiagnostics, publishDiagnosticsParams);
        }
    }
    updatedErrors.clear();
}

void LSPLoop::sendResult(Document &forRequest, Value &result) {
    forRequest.AddMember("result", result, alloc);
    forRequest.RemoveMember("method");
    forRequest.RemoveMember("params");
    sendRaw(forRequest);
}

void LSPLoop::sendError(Document &forRequest, int errorCode, string errorStr) {
    forRequest.RemoveMember("method");
    forRequest.RemoveMember("params");
    Value error;
    error.SetObject();
    error.AddMember("code", errorCode, alloc);
    Value message(errorStr.c_str(), alloc);
    error.AddMember("message", message, alloc);
    forRequest.AddMember("error", error, alloc);
    sendRaw(forRequest);
}

bool LSPLoop::handleReplies(Document &d) {
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
    auto t = indexOne(opts, *initialGS, fref, kvstore);
    int id = t->loc.file.id();
    if (id >= indexed.size()) {
        indexed.resize(id + 1);
    }
    indexed[id] = move(t);
    return fref;
}

vector<unsigned int> LSPLoop::computeStateHashes(const vector<shared_ptr<core::File>> &files) {
    std::vector<unsigned int> res(files.size());
    shared_ptr<ConcurrentBoundedQueue<int>> fileq = make_shared<ConcurrentBoundedQueue<int>>(files.size());
    for (int i = 0; i < files.size(); i++) {
        auto copy = i;
        fileq->push(move(copy), 1);
    }

    res.resize(files.size());

    shared_ptr<BlockingBoundedQueue<vector<std::pair<int, unsigned int>>>> resultq =
        make_shared<BlockingBoundedQueue<vector<std::pair<int, unsigned int>>>>(files.size());
    const auto &opts = this->opts;
    workers.multiplexJob([fileq, resultq, &opts, files, logger = this->logger]() {
        vector<std::pair<int, unsigned int>> threadResult;
        int processedByThread = 0;
        int job;

        {
            for (auto result = fileq->try_pop(job); !result.done(); result = fileq->try_pop(job)) {
                if (result.gotItem()) {
                    processedByThread++;

                    if (!files[job]) {
                        threadResult.emplace_back(make_pair(job, 0));
                        continue;
                    }
                    shared_ptr<core::GlobalState> lgs =
                        make_shared<core::GlobalState>((std::make_shared<core::ErrorQueue>(*logger, *logger)));
                    lgs->initEmpty();
                    lgs->silenceErrors = true;
                    core::UnfreezeFileTable fileTableAccess(*lgs);
                    core::UnfreezeSymbolTable symbolTable(*lgs);
                    core::UnfreezeNameTable nameTable(*lgs);
                    auto fref = lgs->enterFile(files[job]);
                    vector<unique_ptr<ast::Expression>> single;
                    unique_ptr<KeyValueStore> kvstore;
                    single.emplace_back(indexOne(opts, *lgs, fref, kvstore));
                    resolve(*lgs, move(single), opts);
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
        for (auto result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS); !result.done();
             result = resultq->wait_pop_timed(threadResult, PROGRESS_REFRESH_TIME_MILLIS)) {
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
    for (auto &t : index(initialGS, fileNames, emptyInputFiles, opts, workers, kvstore)) {
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

void LSPLoop::runSlowPath(const std::vector<shared_ptr<core::File>> &changedFiles) {
    logger->info("Taking slow path");
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
    typecheck(finalGs, resolve(*finalGs, move(indexedCopies), opts), opts, workers);
}

const LSP::LSPMethod LSP::getMethod(const absl::string_view name) {
    for (auto &candidate : ALL) {
        if (candidate.name == name) {
            return candidate;
        }
    }
    return LSPMethod{(string)name, true, LSPMethod::Kind::ClientInitiated, false};
}

void LSPLoop::tryFastPath(std::vector<shared_ptr<core::File>> &changedFiles) {
    logger->info("Trying to see if happy path is available after {} file changes", changedFiles.size());
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
            logger->info("Taking sad path because {} is a new file", changedFiles[i]->path());
            good = false;
            if (globalStateHashes.size() <= fref.id()) {
                globalStateHashes.resize(fref.id() + 1);
                globalStateHashes[fref.id()] = hashes[i];
            }
        } else {
            if (hashes[i] != globalStateHashes[fref.id()]) {
                logger->info("Taking sad path because {} has changed definitions", changedFiles[i]->path());
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
        logger->info("Taking happy path");
        // Yaay, reuse existing global state.
        vector<std::string> empty;
        auto updatedIndexed = index(finalGs, empty, subset, opts, workers, kvstore);
        ENFORCE(subset.size() == updatedIndexed.size());
        for (auto &t : updatedIndexed) {
            int id = t->loc.file.id();
            indexed[id] = move(t);
            t = indexed[id]->deepCopy();
        }
        typecheck(finalGs, resolve(*finalGs, move(updatedIndexed), opts), opts, workers);
    } else {
        runSlowPath(changedFiles);
    }
}

std::string LSPLoop::remoteName2Local(const absl::string_view uri) {
    ENFORCE(startsWith(uri, rootUri));
    return string(uri.data() + 1 + rootUri.length(), uri.length() - rootUri.length() - 1);
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

} // namespace realmain
} // namespace sorbet
