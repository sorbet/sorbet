#include "main/lsp/tools/make_lsp_types.h"

using namespace std;

shared_ptr<JSONType> makeStrEnum(const string name, vector<const string> values,
                                 vector<shared_ptr<JSONClassType>> &enumTypes) {
    shared_ptr<JSONStringEnumType> ct = make_shared<JSONStringEnumType>(name, values);
    enumTypes.push_back(ct);
    return ct;
}

shared_ptr<JSONType> makeIntEnum(const string name, vector<pair<const string, int>> values,
                                 vector<shared_ptr<JSONClassType>> &enumTypes) {
    auto et = make_shared<JSONIntEnumType>(name, values);
    enumTypes.push_back(et);
    return et;
}

shared_ptr<JSONType> makeStringConstant(const string name) {
    return make_shared<JSONStringConstantType>(name);
}

shared_ptr<FieldDef> makeField(const string name, shared_ptr<JSONType> type) {
    return make_shared<FieldDef>(name, type);
}

shared_ptr<FieldDef> makeField(const string jsonName, const string cppName, shared_ptr<JSONType> type) {
    return make_shared<FieldDef>(jsonName, cppName, type);
}

shared_ptr<JSONObjectType> makeObject(const string name, vector<shared_ptr<FieldDef>> fields,
                                      vector<shared_ptr<JSONObjectType>> &classTypes,
                                      vector<string> extraMethodDefinitions = {}) {
    shared_ptr<JSONObjectType> ct = make_shared<JSONObjectType>(name, fields, extraMethodDefinitions);
    classTypes.push_back(ct);
    return ct;
}

shared_ptr<JSONType> makeOptional(shared_ptr<JSONType> type) {
    return make_shared<JSONOptionalType>(type);
}

shared_ptr<JSONType> makeVariant(vector<shared_ptr<JSONType>> variants) {
    return make_shared<JSONBasicVariantType>(variants);
}

shared_ptr<JSONType>
makeDiscriminatedUnion(shared_ptr<FieldDef> fieldDef,
                       const vector<pair<const string, shared_ptr<JSONType>>> variantsByDiscriminant) {
    return make_shared<JSONDiscriminatedUnionVariantType>(fieldDef, variantsByDiscriminant);
}

shared_ptr<JSONType> makeArray(shared_ptr<JSONType> type) {
    return make_shared<JSONArrayType>(type);
}

void makeLSPTypes(vector<shared_ptr<JSONClassType>> &enumTypes, vector<shared_ptr<JSONObjectType>> &classTypes) {
    // Singletons
    shared_ptr<JSONType> JSONNull = make_shared<JSONNullType>();
    shared_ptr<JSONType> JSONBool = make_shared<JSONBooleanType>();
    shared_ptr<JSONType> JSONInt = make_shared<JSONIntType>();
    shared_ptr<JSONType> JSONDouble = make_shared<JSONDoubleType>();
    shared_ptr<JSONType> JSONString = make_shared<JSONStringType>();

    // Converted from https://microsoft.github.io/language-server-protocol/specification
    // Last updated on 11/14/18.

    auto ResourceOperationKind = makeStrEnum("ResourceOperationKind", {"create", "rename", "delete"}, enumTypes);
    auto MarkupKind = makeStrEnum("MarkupKind", {"plaintext", "markdown"}, enumTypes);
    auto TraceKind = makeStrEnum("TraceKind", {"off", "messages", "verbose"}, enumTypes);
    auto JSONRPCConstant = makeStringConstant("2.0");
    auto CreateConstant = makeStringConstant("create");
    auto RenameConstant = makeStringConstant("rename");
    auto DeleteConstant = makeStringConstant("delete");

    auto ResponseError = makeObject("ResponseError",
                                    {
                                        makeField("code", JSONInt), makeField("message", JSONString),
                                        // Unused in Sorbet.
                                        // makeField("data", makeOptional(JSONAny)),
                                    },
                                    classTypes);

    auto CancelParams = makeObject("CancelParams",
                                   {
                                       makeField("id", makeVariant({JSONInt, JSONString})),
                                   },
                                   classTypes);

    auto Position =
        makeObject("Position",
                   {
                       makeField("line", JSONInt),
                       makeField("character", JSONInt),
                   },
                   classTypes,
                   {
                       "int cmp(const Position &b) const;",
                       "std::unique_ptr<Position> copy() const;",
                       "std::string showRaw() const;",
                       "std::optional<core::Loc> toLoc(const core::GlobalState &gs, core::FileRef fref) const;",
                   });

    auto Range =
        makeObject("Range",
                   {
                       makeField("start", Position),
                       makeField("end", Position),
                   },
                   classTypes,
                   {
                       "// Returns nullptr if loc does not exist",
                       "static std::unique_ptr<Range> fromLoc(const core::GlobalState &gs, core::Loc loc);",
                       "std::optional<core::Loc> toLoc(const core::GlobalState &gs, core::FileRef file) const;",
                       "int cmp(const Range &b) const;",
                       "std::unique_ptr<Range> copy() const;",
                       "bool contains(const Range &b) const;",
                       "bool contains(const Position &b) const;",
                   });

    auto Location = makeObject("Location",
                               {
                                   makeField("uri", JSONString),
                                   makeField("range", Range),
                               },
                               classTypes,
                               {
                                   "int cmp(const Location &b) const;",
                                   "std::unique_ptr<Location> copy() const;",
                               });

    auto DiagnosticRelatedInformation =
        makeObject("DiagnosticRelatedInformation",
                   {
                       makeField("location", Location),
                       makeField("message", JSONString),
                   },
                   classTypes, {"std::unique_ptr<DiagnosticRelatedInformation> copy() const;"});

    auto DiagnosticSeverity =
        makeIntEnum("DiagnosticSeverity", {{"Error", 1}, {"Warning", 2}, {"Information", 3}, {"Hint", 4}}, enumTypes);

    auto DiagnosticTag = makeIntEnum("DiagnosticTag", {{"Unnecessary", 1}, {"Deprecated", 2}}, enumTypes);

    auto CodeDescription = makeObject("CodeDescription",
                                      {
                                          // URI
                                          makeField("href", JSONString),
                                      },
                                      classTypes);

    auto Diagnostic =
        makeObject("Diagnostic",
                   {
                       makeField("range", Range),
                       makeField("severity", makeOptional(DiagnosticSeverity)),
                       makeField("code", makeOptional(makeVariant({JSONInt, JSONString}))),
                       makeField("codeDescription", makeOptional(CodeDescription)),
                       makeField("source", makeOptional(JSONString)),
                       makeField("message", JSONString),
                       makeField("relatedInformation", makeOptional(makeArray(DiagnosticRelatedInformation))),
                       makeField("tags", makeOptional(makeArray(DiagnosticTag))),
                   },
                   classTypes, {"std::unique_ptr<Diagnostic> copy() const;"});

    auto TextDocumentIdentifier = makeObject("TextDocumentIdentifier",
                                             {
                                                 makeField("uri", JSONString),
                                             },
                                             classTypes);

    auto TextDocumentPositionParams = makeObject("TextDocumentPositionParams",
                                                 {
                                                     makeField("textDocument", TextDocumentIdentifier),
                                                     makeField("position", Position),
                                                 },
                                                 classTypes);
    auto Command = makeObject("Command",
                              {
                                  makeField("title", JSONString),
                                  makeField("command", JSONString),
                                  // The only thing we use this for does not require arguments, so it's null.
                                  // The LSP spec has `arguments?: LSPAny[]` here, so feel free to expand
                                  makeField("arguments", makeOptional(makeArray(JSONBool))),
                              },
                              classTypes);

    auto TextEdit = makeObject("TextEdit",
                               {
                                   makeField("range", Range),
                                   makeField("newText", JSONString),
                               },
                               classTypes);

    auto VersionedTextDocumentIdentifier = makeObject("VersionedTextDocumentIdentifier",
                                                      {
                                                          makeField("uri", JSONString),
                                                          makeField("version", makeVariant({JSONDouble, JSONNull})),
                                                      },
                                                      classTypes);

    auto TextDocumentEdit = makeObject("TextDocumentEdit",
                                       {
                                           makeField("textDocument", VersionedTextDocumentIdentifier),
                                           makeField("edits", makeArray(TextEdit)),
                                       },
                                       classTypes);

    // These have the same shape.
    auto CreateOrRenameFileOptions = makeObject("CreateOrRenameFileOptions",
                                                {
                                                    makeField("overwrite", makeOptional(JSONBool)),
                                                    makeField("ignoreIfExists", makeOptional(JSONBool)),
                                                },
                                                classTypes);

    auto CreateFile = makeObject("CreateFile",
                                 {
                                     makeField("kind", CreateConstant),
                                     makeField("uri", JSONString),
                                     makeField("options", makeOptional(CreateOrRenameFileOptions)),
                                 },
                                 classTypes);
    auto RenameFile = makeObject("RenameFile",
                                 {
                                     makeField("kind", RenameConstant),
                                     makeField("oldUri", JSONString),
                                     makeField("newUri", JSONString),
                                     makeField("options", makeOptional(CreateOrRenameFileOptions)),
                                 },
                                 classTypes);

    auto DeleteFileOptions = makeObject("DeleteFileOptions",
                                        {
                                            makeField("recursive", makeOptional(JSONBool)),
                                            makeField("ignoreIfNotExists", makeOptional(JSONBool)),
                                        },
                                        classTypes);

    auto DeleteFile = makeObject("DeleteFile",
                                 {
                                     makeField("kind", DeleteConstant),
                                     makeField("uri", JSONString),
                                     makeField("options", makeOptional(DeleteFileOptions)),
                                 },
                                 classTypes);

    auto WorkspaceEdit = makeObject("WorkspaceEdit",
                                    {// Map type (string => TextEdit[]). Unused by sorbet currently.
                                     // makeField("changes", makeArray(JSONAny)),
                                     // Need a variant that can discriminate based on a `kind` field and
                                     // the presence of that field. (TextDocumentEdit[] | (TextDocumentEdit | CreateFile
                                     // | RenameFile | DeleteFile)[]) makeField("documentChanges",
                                     // makeOptional(makeArray(JSONAny))). What follows is a subset of the true type.
                                     makeField("documentChanges", makeOptional(makeArray(TextDocumentEdit)))},
                                    classTypes);

    auto TextDocumentItem = makeObject("TextDocumentItem",
                                       {
                                           makeField("uri", JSONString),
                                           makeField("languageId", JSONString),
                                           makeField("version", JSONInt),
                                           makeField("text", JSONString),
                                       },
                                       classTypes);

    auto DocumentFilter = makeObject("DocumentFilter",
                                     {
                                         makeField("language", makeOptional(JSONString)),
                                         makeField("scheme", makeOptional(JSONString)),
                                         makeField("pattern", makeOptional(JSONString)),
                                     },
                                     classTypes);

    auto MarkupContent = makeObject("MarkupContent",
                                    {
                                        makeField("kind", MarkupKind),
                                        makeField("value", JSONString),
                                    },
                                    classTypes);

    auto FailureHandlingKind =
        makeStrEnum("FailureHandlingKind", {"abort", "transactional", "undo", "textOnlyTransactional"}, enumTypes);

    auto WorkspaceEditCapabilities =
        makeObject("WorkspaceEditCapabilities",
                   {
                       makeField("documentChanges", makeOptional(JSONBool)),
                       makeField("resourceOperations", makeOptional(makeArray(ResourceOperationKind))),
                       makeField("failureHandling", makeOptional(FailureHandlingKind)),
                   },
                   classTypes);

    auto DynamicRegistrationOption = makeObject("DynamicRegistrationOption",
                                                {
                                                    makeField("dynamicRegistration", makeOptional(JSONBool)),
                                                },
                                                classTypes);

    auto SymbolKind = makeIntEnum("SymbolKind",
                                  {
                                      {"Unknown", 0},   {"File", 1},        {"Module", 2},         {"Namespace", 3},
                                      {"Package", 4},   {"Class", 5},       {"Method", 6},         {"Property", 7},
                                      {"Field", 8},     {"Constructor", 9}, {"Enum", 10},          {"Interface", 11},
                                      {"Function", 12}, {"Variable", 13},   {"Constant", 14},      {"String", 15},
                                      {"Number", 16},   {"Boolean", 17},    {"Array", 18},         {"Object", 19},
                                      {"Key", 20},      {"Null", 21},       {"EnumMember", 22},    {"Struct", 23},
                                      {"Event", 24},    {"Operator", 25},   {"TypeParameter", 26},
                                  },
                                  enumTypes);

    auto SymbolKindOption = makeObject("SymbolKindOptions",
                                       {
                                           makeField("valueSet", makeOptional(makeArray(SymbolKind))),
                                       },
                                       classTypes);

    auto SymbolConfiguration = makeObject("SymbolConfiguration",
                                          {
                                              makeField("dynamicRegistration", makeOptional(JSONBool)),
                                              makeField("symbolKind", makeOptional(SymbolKindOption)),
                                          },
                                          classTypes);

    auto WorkspaceClientCapabilities =
        makeObject("WorkspaceClientCapabilities",
                   {
                       makeField("applyEdit", makeOptional(JSONBool)),
                       makeField("workspaceEdit", makeOptional(WorkspaceEditCapabilities)),
                       makeField("didChangeConfiguration", makeOptional(DynamicRegistrationOption)),
                       makeField("didChangeWatchedFiles", makeOptional(DynamicRegistrationOption)),
                       makeField("symbol", makeOptional(SymbolConfiguration)),
                       makeField("executeCommand", makeOptional(DynamicRegistrationOption)),
                       makeField("workspaceFolders", makeOptional(JSONBool)),
                       makeField("configuration", makeOptional(JSONBool)),
                   },
                   classTypes);

    auto SynchronizationCapabilities = makeObject("SynchronizationCapabilities",
                                                  {
                                                      makeField("dynamicRegistration", makeOptional(JSONBool)),
                                                      makeField("willSave", makeOptional(JSONBool)),
                                                      makeField("willSaveWaitUntil", makeOptional(JSONBool)),
                                                      makeField("didSave", makeOptional(JSONBool)),
                                                  },
                                                  classTypes);

    auto CompletionItemCapabilities =
        makeObject("CompletionItemCapabilities",
                   {
                       makeField("snippetSupport", makeOptional(JSONBool)),
                       makeField("commitCharactersSupport", makeOptional(JSONBool)),
                       makeField("documentationFormat", makeOptional(makeArray(MarkupKind))),
                       makeField("deprecatedSupport", makeOptional(JSONBool)),
                       makeField("preselectSupport", makeOptional(JSONBool)),
                   },
                   classTypes);

    auto CompletionItemKind =
        makeIntEnum("CompletionItemKind",
                    {
                        {"Text", 1},      {"Method", 2},  {"Function", 3},   {"Constructor", 4}, {"Field", 5},
                        {"Variable", 6},  {"Class", 7},   {"Interface", 8},  {"Module", 9},      {"Property", 10},
                        {"Unit", 11},     {"Value", 12},  {"Enum", 13},      {"Keyword", 14},    {"Snippet", 15},
                        {"Color", 16},    {"File", 17},   {"Reference", 18}, {"Folder", 19},     {"EnumMember", 20},
                        {"Constant", 21}, {"Struct", 22}, {"Event", 23},     {"Operator", 24},   {"TypeParameter", 25},
                    },
                    enumTypes);

    auto CompletionItemKindCapabilities =
        makeObject("CompletionItemKindCapabilities",
                   {
                       makeField("valueSet", makeOptional(makeArray(CompletionItemKind))),
                   },
                   classTypes);

    auto CompletionCapabilities =
        makeObject("CompletionCapabilities",
                   {
                       makeField("dynamicRegistration", makeOptional(JSONBool)),
                       makeField("completionItem", makeOptional(CompletionItemCapabilities)),
                       makeField("completionItemKind", makeOptional(CompletionItemKindCapabilities)),
                       makeField("contextSupport", makeOptional(JSONBool)),
                   },
                   classTypes);

    auto HoverCapabilities = makeObject("HoverCapabilities",
                                        {
                                            makeField("dynamicRegistration", makeOptional(JSONBool)),
                                            makeField("contentFormat", makeOptional(makeArray(MarkupKind))),
                                        },
                                        classTypes);

    auto SignatureInformationCapabilities =
        makeObject("SignatureInformationCapabilities",
                   {
                       makeField("documentationFormat", makeOptional(makeArray(MarkupKind))),
                   },
                   classTypes);

    auto SignatureHelpCapabilities =
        makeObject("SignatureHelpCapabilities",
                   {
                       makeField("dynamicRegistration", makeOptional(JSONBool)),
                       makeField("signatureInformation", makeOptional(SignatureInformationCapabilities)),
                   },
                   classTypes);

    auto DocumentSymbolCapabilities =
        makeObject("DocumentSymbolCapabilities",
                   {
                       makeField("dynamicRegistration", makeOptional(JSONBool)),
                       makeField("symbolKind", makeOptional(SymbolKindOption)),
                       makeField("hierarchicalDocumentSymbolSupport", makeOptional(JSONBool)),
                   },
                   classTypes);

    auto CodeActionKind = makeStrEnum("CodeActionKind",
                                      {"quickfix", "refactor", "refactor.extract", "refactor.inline",
                                       "refactor.rewrite", "source", "source.organizeImports", "source.fixAll.sorbet"},
                                      enumTypes);

    auto CodeActionKindSupport =
        makeObject("CodeActionKindSupport",
                   {
                       // Note: The set of code actions is open, so these could be arbitrary strings.
                       makeField("valueSet", makeArray(JSONString)),
                   },
                   classTypes);

    auto CodeActionLiteralSupport = makeObject("CodeActionLiteralSupport",
                                               {
                                                   makeField("codeActionKind", CodeActionKindSupport),
                                               },
                                               classTypes);

    auto CodeActionResolveSupport =
        makeObject("CodeActionResolveSupport", {makeField("properties", makeArray(JSONString))}, classTypes);
    auto CodeActionCapabilities =
        makeObject("CodeActionCapabilities",
                   {
                       makeField("dynamicRegistration", makeOptional(JSONBool)),
                       makeField("codeActionLiteralSupport", makeOptional(CodeActionLiteralSupport)),
                       makeField("dataSupport", makeOptional(JSONBool)),
                       makeField("resolveSupport", makeOptional(CodeActionResolveSupport)),
                   },
                   classTypes);

    auto RenameCapabilities = makeObject("RenameCapabilities",
                                         {
                                             makeField("dynamicRegistration", makeOptional(JSONBool)),
                                             makeField("prepareSupport", makeOptional(JSONBool)),
                                         },
                                         classTypes);

    auto PublishDiagnosticsCapabilities = makeObject("PublishDiagnosticsCapabilities",
                                                     {
                                                         makeField("relatedInformation", makeOptional(JSONBool)),
                                                     },
                                                     classTypes);

    auto FoldingRangeCapabilities = makeObject("FoldingRangeCapabilities",
                                               {
                                                   makeField("dynamicRegistration", makeOptional(JSONBool)),
                                                   makeField("rangeLimit", makeOptional(JSONInt)),
                                                   makeField("lineFoldingOnly", makeOptional(JSONBool)),
                                               },
                                               classTypes);

    auto TextDocumentClientCapabilities =
        makeObject("TextDocumentClientCapabilities",
                   {
                       makeField("synchronization", makeOptional(SynchronizationCapabilities)),
                       makeField("completion", makeOptional(CompletionCapabilities)),
                       makeField("hover", makeOptional(HoverCapabilities)),
                       makeField("signatureHelp", makeOptional(SignatureHelpCapabilities)),
                       makeField("references", makeOptional(DynamicRegistrationOption)),
                       makeField("documentHighlight", makeOptional(DynamicRegistrationOption)),
                       makeField("documentSymbol", makeOptional(DocumentSymbolCapabilities)),
                       makeField("formatting", makeOptional(DynamicRegistrationOption)),
                       makeField("rangeFormatting", makeOptional(DynamicRegistrationOption)),
                       makeField("onTypeFormatting", makeOptional(DynamicRegistrationOption)),
                       makeField("definition", makeOptional(DynamicRegistrationOption)),
                       makeField("typeDefinition", makeOptional(DynamicRegistrationOption)),
                       makeField("implementation", makeOptional(DynamicRegistrationOption)),
                       makeField("codeAction", makeOptional(CodeActionCapabilities)),
                       makeField("codeLens", makeOptional(DynamicRegistrationOption)),
                       makeField("documentLink", makeOptional(DynamicRegistrationOption)),
                       makeField("colorProvider", makeOptional(DynamicRegistrationOption)),
                       makeField("rename", makeOptional(RenameCapabilities)),
                       makeField("publishDiagnostics", makeOptional(PublishDiagnosticsCapabilities)),
                       makeField("foldingRange", makeOptional(FoldingRangeCapabilities)),
                   },
                   classTypes);

    auto ClientCapabilities = makeObject("ClientCapabilities",
                                         {
                                             makeField("workspace", makeOptional(WorkspaceClientCapabilities)),
                                             makeField("textDocument", makeOptional(TextDocumentClientCapabilities)),
                                             // Unused in Sorbet.
                                             // makeField("experimental", makeOptional(JSONAny)),
                                         },
                                         classTypes);

    auto CompletionOptions = makeObject("CompletionOptions",
                                        {
                                            makeField("resolveProvider", makeOptional(JSONBool)),
                                            makeField("triggerCharacters", makeOptional(makeArray(JSONString))),
                                        },
                                        classTypes);

    auto SignatureHelpOptions = makeObject(
        "SignatureHelpOptions", {makeField("triggerCharacters", makeOptional(makeArray(JSONString)))}, classTypes);

    auto CodeActionOptions = makeObject("CodeActionOptions",
                                        {
                                            makeField("codeActionKinds", makeOptional(makeArray(CodeActionKind))),
                                            makeField("resolveProvider", makeOptional(JSONBool)),
                                        },
                                        classTypes);

    auto CodeLensOptions = makeObject("CodeLensOptions",
                                      {
                                          makeField("resolveProvider", makeOptional(JSONBool)),
                                      },
                                      classTypes);

    auto DocumentOnTypeFormattingOptions =
        makeObject("DocumentOnTypeFormattingOptions",
                   {
                       makeField("firstTriggerCharacter", JSONString),
                       makeField("moreTriggerCharacter", makeOptional(makeArray(JSONString))),
                   },
                   classTypes);

    auto RenameOptions = makeObject("RenameOptions",
                                    {
                                        makeField("prepareProvider", JSONBool),
                                    },
                                    classTypes);

    auto DocumentLinkOptions = makeObject("DocumentLinkOptions",
                                          {
                                              makeField("resolveProvider", JSONBool),
                                          },
                                          classTypes);

    auto ExecuteCommandOptions = makeObject("ExecuteCommandOptions",
                                            {
                                                makeField("commands", makeArray(JSONString)),
                                            },
                                            classTypes);

    auto SaveOptions = makeObject("SaveOptions",
                                  {
                                      makeField("includeText", makeOptional(JSONBool)),
                                  },
                                  classTypes);

    auto EmptyOptions = makeObject("EmptyOptions", {}, classTypes);

    auto TextDocumentSyncKind = makeIntEnum("TextDocumentSyncKind",
                                            {
                                                {"None", 0},
                                                {"Full", 1},
                                                {"Incremental", 2},
                                            },
                                            enumTypes);

    auto TextDocumentSyncOptions = makeObject("TextDocumentSyncOptions",
                                              {
                                                  makeField("openClose", makeOptional(JSONBool)),
                                                  makeField("change", makeOptional(TextDocumentSyncKind)),
                                                  makeField("willSave", makeOptional(JSONBool)),
                                                  makeField("willSaveWaitUntil", makeOptional(JSONBool)),
                                                  makeField("save", makeOptional(SaveOptions)),
                                              },
                                              classTypes);

    auto DocumentSelector = makeArray(DocumentFilter);

    auto TextDocumentAndStaticRegistrationOptions =
        makeObject("TextDocumentAndStaticRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({JSONNull, DocumentSelector})),
                       makeField("id", JSONString),
                   },
                   classTypes);

    auto WorkspaceFoldersOptions =
        makeObject("WorkspaceFoldersOptions",
                   {
                       makeField("supported", makeOptional(JSONBool)),
                       makeField("changeNotifications", makeOptional(makeVariant({JSONString, JSONBool}))),
                   },
                   classTypes);

    auto WorkspaceOptions = makeObject("WorkspaceOptions",
                                       {
                                           makeField("workspaceFolders", makeOptional(WorkspaceFoldersOptions)),
                                       },
                                       classTypes);

    auto ServerCapabilities = makeObject(
        "ServerCapabilities",
        {
            makeField("textDocumentSync", makeOptional(makeVariant({TextDocumentSyncOptions, TextDocumentSyncKind}))),
            makeField("hoverProvider", makeOptional(JSONBool)),
            makeField("completionProvider", makeOptional(CompletionOptions)),
            makeField("signatureHelpProvider", makeOptional(SignatureHelpOptions)),
            makeField("definitionProvider", makeOptional(JSONBool)),
            makeField("typeDefinitionProvider", makeOptional(JSONBool)),
            makeField("implementationProvider", makeOptional({JSONBool})),
            makeField("referencesProvider", makeOptional(JSONBool)),
            makeField("documentHighlightProvider", makeOptional(JSONBool)),
            makeField("documentSymbolProvider", makeOptional(JSONBool)),
            makeField("workspaceSymbolProvider", makeOptional(JSONBool)),
            makeField("codeActionProvider", makeOptional(makeVariant({JSONBool, CodeActionOptions}))),
            makeField("codeLensProvider", makeOptional(CodeLensOptions)),
            makeField("documentFormattingProvider", makeOptional(JSONBool)),
            makeField("documentRangeFormattingProvider", makeOptional(JSONBool)),
            makeField("documentOnTypeFormattingProvider", makeOptional(DocumentOnTypeFormattingOptions)),
            makeField("renameProvider", makeOptional(makeVariant({JSONBool, RenameOptions}))),
            makeField("documentLinkProvider", makeOptional(DocumentLinkOptions)),
            // Unused in Sorbet.
            // makeField("colorProvider", makeOptional(makeVariant({JSONBool, JSONAnyObject}))),
            // makeField("foldingRangeProvider", makeOptional(makeVariant({JSONBool, JSONAnyObject}))),
            makeField("executeCommandProvider", makeOptional(ExecuteCommandOptions)),
            makeField("workspace", makeOptional(WorkspaceOptions)),
            // Unused in Sorbet.
            // makeField("experimental", makeOptional(JSONAny)),
            // -- Sorbet extensions --
            makeField("sorbetShowSymbolProvider", makeOptional(JSONBool)),
        },
        classTypes);

    auto MessageType = makeIntEnum("MessageType",
                                   {
                                       {"Error", 1},
                                       {"Warning", 2},
                                       {"Info", 3},
                                       {"Log", 4},
                                   },
                                   enumTypes);

    auto ShowMessageParams = makeObject("ShowMessageParams",
                                        {
                                            makeField("type", MessageType),
                                            makeField("message", JSONString),
                                        },
                                        classTypes);

    auto MessageActionItem = makeObject("MessageActionItem",
                                        {
                                            makeField("title", JSONString),
                                        },
                                        classTypes);

    auto ShowMessageRequestParams = makeObject("ShowMessageRequestParams",
                                               {
                                                   makeField("type", MessageType),
                                                   makeField("message", JSONString),
                                                   makeField("actions", makeOptional(makeArray(MessageActionItem))),
                                               },
                                               classTypes);

    auto LogMessageParams = makeObject("LogMessageParams",
                                       {
                                           makeField("type", MessageType),
                                           makeField("message", JSONString),
                                       },
                                       classTypes);

    auto Registration = makeObject("Registration",
                                   {
                                       makeField("id", JSONString), makeField("method", JSONString),
                                       // Unused in Sorbet.
                                       // makeField("registerOptions", makeOptional(JSONAny)),
                                   },
                                   classTypes);

    auto RegistrationParams = makeObject("RegistrationParams",
                                         {
                                             makeField("registrations", makeArray(Registration)),
                                         },
                                         classTypes);

    auto TextDocumentRegistrationOptions =
        makeObject("TextDocumentRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({DocumentSelector, JSONNull})),
                   },
                   classTypes);

    auto Unregistration = makeObject("Unregistration",
                                     {
                                         makeField("id", JSONInt),
                                         makeField("method", JSONString),
                                     },
                                     classTypes);

    auto UnregistrationParams = makeObject("UnregistrationParams",
                                           {
                                               makeField("unregistrations", makeArray(Unregistration)),
                                           },
                                           classTypes);

    auto WorkspaceFolder = makeObject("WorkspaceFolder",
                                      {
                                          makeField("uri", JSONString),
                                          makeField("name", JSONString),
                                      },
                                      classTypes);

    auto WorkspaceFoldersChangeEvent = makeObject("WorkspaceFoldersChangeEvent",
                                                  {
                                                      makeField("added", makeArray(WorkspaceFolder)),
                                                      makeField("removed", makeArray(WorkspaceFolder)),
                                                  },
                                                  classTypes);

    auto DidChangeWorkspaceFoldersParams = makeObject("DidChangeWorkspaceFoldersParams",
                                                      {
                                                          makeField("event", WorkspaceFoldersChangeEvent),
                                                      },
                                                      classTypes);

    auto ConfigurationItem = makeObject("ConfigurationItem",
                                        {
                                            makeField("scopeUri", makeOptional(JSONString)),
                                            makeField("section", makeOptional(JSONString)),
                                        },
                                        classTypes);

    auto ConfigurationParams = makeObject("ConfigurationParams",
                                          {
                                              makeField("items", makeArray(ConfigurationItem)),
                                          },
                                          classTypes);

    auto FileChangeType = makeIntEnum("FileChangeType",
                                      {
                                          {"Created", 1},
                                          {"Changed", 2},
                                          {"Deleted", 3},
                                      },
                                      enumTypes);

    auto FileEvent = makeObject("FileEvent",
                                {
                                    makeField("uri", JSONString),
                                    makeField("type", FileChangeType),
                                },
                                classTypes);

    auto DidChangeWatchedFilesParams = makeObject("DidChangeWatchedFilesParams",
                                                  {
                                                      makeField("changes", makeArray(FileEvent)),
                                                  },
                                                  classTypes);

    // Note: Three base enum values are used as bitmasks, so I filled in the gaps.
    auto WatchKind = makeIntEnum("WatchKind",
                                 {{"Create", 1},
                                  {"Change", 2},
                                  {"CreateAndChange", 3},
                                  {"Delete", 4},
                                  {"CreateAndDelete", 5},
                                  {"ChangeAndDelete", 6},
                                  {"CreateAndChangeAndDelete", 7}},
                                 enumTypes);

    auto FileSystemWatcher = makeObject("FileSystemWatcher",
                                        {
                                            makeField("globPattern", JSONString),
                                            makeField("kind", makeOptional(WatchKind)),
                                        },
                                        classTypes);

    auto DidChangeWatchedFilesRegistrationOptions = makeObject("DidChangeWatchedFilesRegistrationOptions",
                                                               {
                                                                   makeField("watchers", makeArray(FileSystemWatcher)),
                                                               },
                                                               classTypes);

    auto WorkspaceSymbolParams = makeObject("WorkspaceSymbolParams",
                                            {
                                                makeField("query", JSONString),
                                            },
                                            classTypes);

    auto ExecuteCommandParams = makeObject("ExecuteCommandParams",
                                           {
                                               makeField("command", JSONString),
                                               // Unused in Sorbet.
                                               // makeField("arguments", makeOptional(makeArray(JSONAny))),
                                           },
                                           classTypes);

    auto ExecuteCommandRegistrationOptions = makeObject("ExecuteCommandRegistrationOptions",
                                                        {
                                                            makeField("commands", makeArray(JSONString)),
                                                        },
                                                        classTypes);

    auto ApplyWorkspaceEditParams = makeObject("ApplyWorkspaceEditParams",
                                               {
                                                   makeField("label", makeOptional(JSONString)),
                                                   makeField("edit", WorkspaceEdit),
                                               },
                                               classTypes);

    auto ApplyWorkspaceEditResponse = makeObject("ApplyWorkspaceEditResponse",
                                                 {
                                                     makeField("applied", JSONBool),
                                                 },
                                                 classTypes);

    auto DidOpenTextDocumentParams = makeObject("DidOpenTextDocumentParams",
                                                {
                                                    makeField("textDocument", TextDocumentItem),
                                                },
                                                classTypes);

    auto TextDocumentContentChangeEvent =
        makeObject("TextDocumentContentChangeEvent",
                   {
                       makeField("range", makeOptional(Range)),
                       makeField("rangeLength", makeOptional(JSONInt)),
                       makeField("text", JSONString),
                   },
                   classTypes, {"std::string apply(std::string_view oldContents) const;"});

    auto DidChangeTextDocumentParams =
        makeObject("DidChangeTextDocumentParams",
                   {
                       makeField("textDocument", VersionedTextDocumentIdentifier),
                       makeField("contentChanges", makeArray(TextDocumentContentChangeEvent)),
                       // Used in tests only.
                       makeField("sorbetCancellationExpected", makeOptional(JSONBool)),
                       makeField("sorbetPreemptionsExpected", makeOptional(JSONInt)),
                   },
                   classTypes, {"std::string getSource(std::string_view oldFileContents) const;"});

    auto TextDocumentChangeRegistrationOptions =
        makeObject("TextDocumentChangeRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({DocumentSelector, JSONNull})),
                       makeField("syncKind", TextDocumentSyncKind),
                   },
                   classTypes);

    auto TextDocumentSaveReason = makeIntEnum("TextDocumentSaveReason",
                                              {
                                                  {"Manual", 1},
                                                  {"AfterDelay", 2},
                                                  {"FocusOut", 3},
                                              },
                                              enumTypes);

    auto WillSaveTextDocumentParams = makeObject("WillSaveTextDocumentParams",
                                                 {
                                                     makeField("textDocument", TextDocumentIdentifier),
                                                     makeField("reason", TextDocumentSaveReason),
                                                 },
                                                 classTypes);

    auto DidSaveTextDocumentParams = makeObject("DidSaveTextDocumentParams",
                                                {
                                                    makeField("textDocument", TextDocumentIdentifier),
                                                    makeField("text", makeOptional(JSONString)),
                                                },
                                                classTypes);

    auto TextDocumentSaveRegistrationOptions =
        makeObject("TextDocumentSaveRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({DocumentSelector, JSONNull})),
                       makeField("includeText", makeOptional(JSONBool)),
                   },
                   classTypes);

    auto DidCloseTextDocumentParams = makeObject("DidCloseTextDocumentParams",
                                                 {
                                                     makeField("textDocument", TextDocumentIdentifier),
                                                 },
                                                 classTypes);

    auto PublishDiagnosticsParams = makeObject("PublishDiagnosticsParams",
                                               {
                                                   makeField("uri", JSONString),
                                                   makeField("diagnostics", makeArray(Diagnostic)),
                                               },
                                               classTypes);

    auto CompletionTriggerKind = makeIntEnum("CompletionTriggerKind",
                                             {
                                                 {"Invoked", 1},
                                                 {"TriggerCharacter", 2},
                                                 {"TriggerForIncompleteCompletions", 3},
                                             },
                                             enumTypes);

    auto CompletionContext = makeObject("CompletionContext",
                                        {
                                            makeField("triggerKind", CompletionTriggerKind),
                                            makeField("triggerCharacter", makeOptional(JSONString)),
                                        },
                                        classTypes);

    auto CompletionParams = makeObject("CompletionParams",
                                       {
                                           makeField("textDocument", TextDocumentIdentifier),
                                           makeField("position", Position),
                                           makeField("context", makeOptional(CompletionContext)),
                                       },
                                       classTypes);

    auto InsertTextFormat = makeIntEnum("InsertTextFormat",
                                        {
                                            {"PlainText", 1},
                                            {"Snippet", 2},
                                        },
                                        enumTypes);

    auto CompletionItem = makeObject(
        "CompletionItem",
        {
            makeField("label", JSONString), makeField("kind", makeOptional(CompletionItemKind)),
            makeField("detail", makeOptional(JSONString)),
            makeField("documentation", makeOptional(makeVariant({JSONString, MarkupContent}))),
            makeField("deprecated", makeOptional(JSONBool)), makeField("preselect", makeOptional(JSONBool)),
            makeField("sortText", makeOptional(JSONString)), makeField("filterText", makeOptional(JSONString)),
            makeField("insertText", makeOptional(JSONString)),
            makeField("insertTextFormat", makeOptional(InsertTextFormat)),
            makeField("textEdit", makeOptional(TextEdit)),
            makeField("additionalTextEdits", makeOptional(makeArray(TextEdit))),
            makeField("commitCharacters", makeOptional(makeArray(JSONString))),
            makeField("command", makeOptional(Command)),
            // Unused in Sorbet.
            // makeField("data", makeOptional(JSONAny)),
        },
        classTypes);

    auto CompletionRegistrationOptions =
        makeObject("CompletionRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({DocumentSelector, JSONNull})),
                       makeField("triggerCharacters", makeOptional(makeArray(JSONString))),
                       makeField("resolveProvider", makeOptional(JSONBool)),
                   },
                   classTypes);

    auto CompletionList = makeObject("CompletionList",
                                     {
                                         makeField("isIncomplete", JSONBool),
                                         makeField("items", makeArray(CompletionItem)),
                                     },
                                     classTypes);

    auto Hover = makeObject("Hover",
                            {
                                // Is `string | { language: string, value: string} | { language: string, value:
                                // string}[] | MarkupContent` in spec, but we only use the last form in Sorbet.
                                makeField("contents", MarkupContent),
                                makeField("range", makeOptional(Range)),
                            },
                            classTypes);

    auto ParameterInformation =
        makeObject("ParameterInformation",
                   {
                       makeField("label", JSONString),
                       makeField("documentation", makeOptional(makeVariant({JSONString, MarkupContent}))),
                   },
                   classTypes);

    auto SignatureInformation =
        makeObject("SignatureInformation",
                   {
                       makeField("label", JSONString),
                       makeField("documentation", makeOptional(makeVariant({JSONString, MarkupContent}))),
                       makeField("parameters", makeOptional(makeArray(ParameterInformation))),
                   },
                   classTypes);

    auto SignatureHelp = makeObject("SignatureHelp",
                                    {
                                        makeField("signatures", makeArray(SignatureInformation)),
                                        makeField("activeSignature", makeOptional(JSONInt)),
                                        makeField("activeParameter", makeOptional(JSONInt)),
                                    },
                                    classTypes);

    auto ReferenceContext = makeObject("ReferenceContext",
                                       {
                                           makeField("includeDeclaration", JSONBool),
                                       },
                                       classTypes);

    auto ReferenceParams = makeObject("ReferenceParams",
                                      {
                                          makeField("textDocument", TextDocumentIdentifier),
                                          makeField("position", Position),
                                          makeField("context", ReferenceContext),
                                      },
                                      classTypes);

    auto DocumentHighlightKind = makeIntEnum("DocumentHighlightKind",
                                             {
                                                 {"Text", 1},
                                                 {"Read", 2},
                                                 {"Write", 3},
                                             },
                                             enumTypes);

    auto DocumentHighlight = makeObject("DocumentHighlight",
                                        {
                                            makeField("range", Range),
                                            makeField("kind", makeOptional(DocumentHighlightKind)),
                                        },
                                        classTypes);

    auto DocumentSymbolParams = makeObject("DocumentSymbolParams",
                                           {
                                               makeField("textDocument", TextDocumentIdentifier),
                                           },
                                           classTypes);

    auto DocumentSymbol = makeObject("DocumentSymbol",
                                     {
                                         makeField("name", JSONString),
                                         makeField("detail", makeOptional(JSONString)),
                                         makeField("kind", SymbolKind),
                                         makeField("deprecated", makeOptional(JSONBool)),
                                         makeField("range", Range),
                                         makeField("selectionRange", Range),
                                     },
                                     classTypes);
    DocumentSymbol->addField(makeField("children", makeOptional(makeArray(DocumentSymbol))));

    auto SymbolInformation = makeObject("SymbolInformation",
                                        {
                                            makeField("name", JSONString),
                                            makeField("kind", SymbolKind),
                                            makeField("deprecated", makeOptional(JSONBool)),
                                            makeField("location", Location),
                                            makeField("containerName", makeOptional(JSONString)),
                                        },
                                        classTypes);

    auto CodeActionContext = makeObject("CodeActionContext",
                                        {
                                            makeField("diagnostics", makeArray(Diagnostic)),
                                            makeField("only", makeOptional(makeArray(CodeActionKind))),
                                        },
                                        classTypes);

    auto CodeActionParams = makeObject("CodeActionParams",
                                       {
                                           makeField("textDocument", TextDocumentIdentifier),
                                           makeField("range", Range),
                                           makeField("context", CodeActionContext),
                                       },
                                       classTypes);

    auto CodeAction = makeObject("CodeAction",
                                 {
                                     makeField("title", JSONString),
                                     makeField("kind", makeOptional(CodeActionKind)),
                                     makeField("diagnostics", makeOptional(makeArray(Diagnostic))),
                                     makeField("edit", makeOptional(WorkspaceEdit)),
                                     makeField("command", makeOptional(Command)),
                                     // The LSP spec defines the `data` field as `LSPAny`,
                                     // but we use it only to transfer `CodeActionParams`
                                     makeField("data", makeOptional(CodeActionParams)),
                                 },
                                 classTypes);

    auto CodeActionRegistrationOptions =
        makeObject("CodeActionRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({DocumentSelector, JSONNull})),
                       makeField("codeActionKinds", makeOptional(makeArray(CodeActionKind))),
                   },
                   classTypes);

    auto ImplementationParams = makeObject("ImplementationParams",
                                           {
                                               makeField("textDocument", TextDocumentIdentifier),
                                               makeField("position", Position),
                                           },
                                           classTypes);

    auto CodeLensParams = makeObject("CodeLensParams",
                                     {
                                         makeField("textDocument", TextDocumentIdentifier),
                                     },
                                     classTypes);

    auto CodeLens = makeObject("CodeLens",
                               {
                                   makeField("range", Range), makeField("command", makeOptional(Command)),
                                   // Unused in Sorbet.
                                   // makeField("data", makeOptional(JSONAny)),
                               },
                               classTypes);

    auto DocumentLinkAndCodeLensRegistrationOptions =
        makeObject("CodeLensRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({DocumentSelector, JSONNull})),
                       makeField("resolveProvider", makeOptional(JSONBool)),
                   },
                   classTypes);

    auto DocumentLinkParams = makeObject("DocumentLinkParams",
                                         {
                                             makeField("textDocument", TextDocumentIdentifier),
                                         },
                                         classTypes);

    auto DocumentLink = makeObject("DocumentLink",
                                   {
                                       makeField("range", Range),
                                       // URI
                                       makeField("target", makeOptional(JSONString)),
                                       // Unused in Sorbet.
                                       // makeField("data", makeOptional(JSONAny)),
                                   },
                                   classTypes);

    auto DocumentColorParams = makeObject("DocumentColorParams",
                                          {
                                              makeField("textDocument", TextDocumentIdentifier),
                                          },
                                          classTypes);

    auto Color = makeObject("Color",
                            {
                                makeField("red", JSONDouble),
                                makeField("green", JSONDouble),
                                makeField("blue", JSONDouble),
                                makeField("alpha", JSONDouble),
                            },
                            classTypes);

    auto ColorInformation = makeObject("ColorInformation",
                                       {
                                           makeField("range", Range),
                                           makeField("color", Color),
                                       },
                                       classTypes);

    auto ColorPresentationParams = makeObject("ColorPresentationParams",
                                              {
                                                  makeField("textDocument", TextDocumentIdentifier),
                                                  makeField("color", Color),
                                                  makeField("range", Range),
                                              },
                                              classTypes);

    auto ColorPresentation = makeObject("ColorPresentation",
                                        {
                                            makeField("label", JSONString),
                                            makeField("textEdit", makeOptional(TextEdit)),
                                            makeField("additionalTextEdits", makeOptional(makeArray(TextEdit))),
                                        },
                                        classTypes);

    auto FormattingOptions = makeObject("FormattingOptions",
                                        {
                                            makeField("tabSize", JSONInt),
                                            makeField("insertSpaces", JSONBool),
                                        },
                                        classTypes);

    auto DocumentFormattingParams = makeObject("DocumentFormattingParams",
                                               {
                                                   makeField("textDocument", TextDocumentIdentifier),
                                                   makeField("options", FormattingOptions),
                                               },
                                               classTypes);

    auto DocumentRangeFormattingParams = makeObject("DocumentRangeFormattingParams",
                                                    {
                                                        makeField("textDocument", TextDocumentIdentifier),
                                                        makeField("range", Range),
                                                        makeField("options", FormattingOptions),
                                                    },
                                                    classTypes);

    auto DocumentOnTypeFormattingParams = makeObject("DocumentOnTypeFormattingParams",
                                                     {
                                                         makeField("textDocument", TextDocumentIdentifier),
                                                         makeField("position", Position),
                                                         makeField("ch", JSONString),
                                                         makeField("options", FormattingOptions),
                                                     },
                                                     classTypes);

    auto DocumentOnTypeFormattingRegistrationOptions =
        makeObject("DocumentOnTypeFormattingRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({DocumentSelector, JSONNull})),
                       makeField("firstTriggerCharacter", JSONString),
                       makeField("moreTriggerCharacter", makeOptional(JSONString)),
                   },
                   classTypes);

    auto RenameParams = makeObject("RenameParams",
                                   {
                                       makeField("textDocument", TextDocumentIdentifier),
                                       makeField("position", Position),
                                       makeField("newName", JSONString),
                                   },
                                   classTypes);

    auto RenameRegistrationOptions =
        makeObject("RenameRegistrationOptions",
                   {
                       makeField("documentSelector", makeVariant({DocumentSelector, JSONNull})),
                       makeField("prepareProvider", makeOptional(JSONBool)),
                   },
                   classTypes);

    auto FoldingRangeParams = makeObject("FoldingRangeParams",
                                         {
                                             makeField("textDocument", TextDocumentIdentifier),
                                         },
                                         classTypes);

    auto FoldingRangeKind = makeStrEnum("FoldingRangeKind", {"comment", "imports", "region"}, enumTypes);

    auto FoldingRange = makeObject("FoldingRange",
                                   {
                                       makeField("startLine", JSONInt),
                                       makeField("startCharacter", makeOptional(JSONInt)),
                                       makeField("endLine", JSONInt),
                                       makeField("endCharacter", makeOptional(JSONInt)),
                                       makeField("kind", makeOptional(FoldingRangeKind)),
                                   },
                                   classTypes);

    auto InitializeResult = makeObject("InitializeResult",
                                       {
                                           makeField("capabilities", ServerCapabilities),
                                       },
                                       classTypes);

    auto InitializeError = makeObject("InitializeError", {makeField("retry", JSONBool)}, classTypes);

    auto SorbetInitializationOptions =
        makeObject("SorbetInitializationOptions",
                   {
                       makeField("supportsOperationNotifications", makeOptional(JSONBool)),
                       makeField("supportsSorbetURIs", makeOptional(JSONBool)),
                       makeField("enableTypecheckInfo", makeOptional(JSONBool)),
                       makeField("highlightUntyped", makeOptional(makeVariant({JSONBool, JSONString}))),
                       makeField("enableTypedFalseCompletionNudges", makeOptional(JSONBool)),
                   },
                   classTypes);
    auto InitializeParams =
        makeObject("InitializeParams",
                   {
                       makeField("processId", makeOptional(makeVariant({JSONDouble, JSONNull}))),
                       makeField("rootPath", makeOptional(makeVariant({JSONString, JSONNull}))),
                       makeField("rootUri", makeVariant({JSONString, JSONNull})),
                       makeField("initializationOptions", makeOptional(SorbetInitializationOptions)),
                       makeField("capabilities", ClientCapabilities),
                       makeField("trace", makeOptional(TraceKind)),
                       makeField("workspaceFolders", makeOptional(makeVariant({JSONNull, makeArray(WorkspaceFolder)}))),
                   },
                   classTypes);

    // Empty object.
    auto InitializedParams = makeObject("InitializedParams", {}, classTypes);

    auto DidChangeConfigurationParams = makeObject("DidChangeConfigurationParams",
                                                   {
                                                       makeField("settings", SorbetInitializationOptions),
                                                   },
                                                   classTypes);

    auto PrepareRenameResult = makeObject("PrepareRenameResult",
                                          {
                                              makeField("range", Range),
                                              makeField("placeholder", makeOptional(JSONString)),
                                          },
                                          classTypes);

    /* Sorbet LSP extensions */
    auto SorbetOperationStatus = makeStrEnum("SorbetOperationStatus", {"start", "end"}, enumTypes);
    auto SorbetShowOperationParams = makeObject("SorbetShowOperationParams",
                                                {
                                                    makeField("operationName", JSONString),
                                                    makeField("description", JSONString),
                                                    makeField("status", SorbetOperationStatus),
                                                },
                                                classTypes);
    auto SorbetErrorParams = makeObject("SorbetErrorParams",
                                        {
                                            makeField("code", JSONInt),
                                            makeField("message", JSONString),
                                        },
                                        classTypes);

    /* Watchman JSON response objects */
    auto WatchmanWatchProjectResponse =
        makeObject("WatchmanWatchProjectResponse",
                   {
                       makeField("version", JSONString),
                       makeField("watcher", JSONString),
                       makeField("watch", JSONString),
                       makeField("relative_path", "relativePath", makeOptional(JSONString)),
                   },
                   classTypes);

    auto WatchmanQueryResponse = makeObject("WatchmanQueryResponse",
                                            {
                                                makeField("version", JSONString),
                                                makeField("clock", JSONString),
                                                makeField("is_fresh_instance", "isFreshInstance", JSONBool),
                                                makeField("files", makeArray(JSONString)),
                                            },
                                            classTypes);

    auto WatchmanStateEnter = makeObject("WatchmanStateEnter",
                                         {
                                             makeField("subscription", JSONString),
                                             makeField("root", JSONString),
                                             makeField("state-enter", "stateEnter", JSONString),
                                             makeField("clock", JSONString),
                                         },
                                         classTypes);

    auto WatchmanStateLeave = makeObject("WatchmanStateLeave",
                                         {
                                             makeField("subscription", JSONString),
                                             makeField("root", JSONString),
                                             makeField("state-leave", "stateLeave", JSONString),
                                             makeField("clock", JSONString),
                                             makeField("abandoned", makeOptional(JSONBool)),
                                         },
                                         classTypes);

    auto SorbetWorkspaceEditParams =
        makeObject("SorbetWorkspaceEditParams", {}, classTypes,
                   {
                       "uint32_t epoch = 0;",
                       "// Contains the number of individual edit messages merged into this edit.",
                       "uint32_t mergeCount = 0;",
                       "// Used in multithreaded tests to wait for a cancellation to occur when processing this edit.",
                       "bool sorbetCancellationExpected = false;",
                       "// Used in multithreaded tests to wait for a preemption to occur when processing this edit.",
                       "int sorbetPreemptionsExpected = 0;",
                       "// For each edit rolled up into update, contains a timer used to report diagnostic latency.",
                       "std::vector<std::unique_ptr<Timer>> diagnosticLatencyTimers;",
                       "// File updates contained in this edit.",
                       "std::vector<std::shared_ptr<core::File>> updates;",
                       "// Merge newerParams into this object, which mutates `epoch` and `updates`",
                       "void merge(SorbetWorkspaceEditParams &newerParams);",
                   });

    auto SorbetTypecheckRunStatus =
        makeIntEnum("SorbetTypecheckRunStatus", {{"Started", 0}, {"Cancelled", 1}, {"Ended", 2}}, enumTypes);

    auto TypecheckingPath = makeStrEnum("TypecheckingPath", {"fast", "slow"}, enumTypes);

    auto SorbetTypecheckRunInfo = makeObject("SorbetTypecheckRunInfo",
                                             {
                                                 makeField("status", SorbetTypecheckRunStatus),
                                                 makeField("typecheckingPath", TypecheckingPath),
                                                 makeField("filesTypechecked", makeArray(JSONString)),
                                             },
                                             classTypes);

    auto SorbetCounters = makeObject("SorbetCounters", {}, classTypes, {"CounterState counters;"});

    /* Core LSPMessage objects */
    // N.B.: Only contains LSP methods that Sorbet actually cares about.
    // All others are ignored.
    auto LSPMethod = makeStrEnum("LSPMethod",
                                 {
                                     "__GETCOUNTERS__",
                                     "__PAUSE__",
                                     "__RESUME__",
                                     "$/cancelRequest",
                                     "exit",
                                     "initialize",
                                     "initialized",
                                     "shutdown",
                                     "sorbet/error",
                                     "sorbet/fence",
                                     "sorbet/indexerInitialization",
                                     "sorbet/readFile",
                                     "sorbet/showOperation",
                                     "sorbet/showSymbol",
                                     "sorbet/typecheckRunInfo",
                                     "sorbet/watchmanFileChange",
                                     "sorbet/watchmanStateEnter",
                                     "sorbet/watchmanStateLeave",
                                     "sorbet/workspaceEdit",
                                     "textDocument/codeAction",
                                     "codeAction/resolve",
                                     "textDocument/completion",
                                     "textDocument/definition",
                                     "textDocument/typeDefinition",
                                     "textDocument/didChange",
                                     "textDocument/didClose",
                                     "textDocument/didOpen",
                                     "textDocument/documentHighlight",
                                     "textDocument/documentSymbol",
                                     "textDocument/formatting",
                                     "textDocument/hover",
                                     "textDocument/prepareRename",
                                     "textDocument/publishDiagnostics",
                                     "textDocument/references",
                                     "textDocument/rename",
                                     "textDocument/signatureHelp",
                                     "window/showMessage",
                                     "workspace/symbol",
                                     "workspace/didChangeConfiguration",
                                     "textDocument/implementation",
                                 },
                                 enumTypes);

    auto methodField = makeField("method", LSPMethod);
    auto RequestMessageParamsType =
        makeDiscriminatedUnion(methodField, {
                                                {"__GETCOUNTERS__", makeOptional(JSONNull)},
                                                {"initialize", InitializeParams},
                                                {"shutdown", makeOptional(JSONNull)},
                                                {"textDocument/documentHighlight", TextDocumentPositionParams},
                                                {"textDocument/documentSymbol", DocumentSymbolParams},
                                                {"textDocument/definition", TextDocumentPositionParams},
                                                {"textDocument/typeDefinition", TextDocumentPositionParams},
                                                {"textDocument/hover", TextDocumentPositionParams},
                                                {"textDocument/completion", CompletionParams},
                                                {"textDocument/prepareRename", TextDocumentPositionParams},
                                                {"textDocument/references", ReferenceParams},
                                                {"textDocument/rename", RenameParams},
                                                {"textDocument/signatureHelp", TextDocumentPositionParams},
                                                {"textDocument/codeAction", CodeActionParams},
                                                {"codeAction/resolve", CodeAction},
                                                {"textDocument/implementation", ImplementationParams},
                                                {"textDocument/formatting", DocumentFormattingParams},
                                                {"workspace/symbol", WorkspaceSymbolParams},
                                                {"sorbet/error", SorbetErrorParams},
                                                {"sorbet/readFile", TextDocumentIdentifier},
                                                {"sorbet/showSymbol", TextDocumentPositionParams},
                                            });
    auto RequestMessage =
        makeObject("RequestMessage",
                   {makeField("jsonrpc", JSONRPCConstant), makeField("id", makeVariant({JSONInt, JSONString})),
                    methodField, makeField("params", RequestMessageParamsType)},
                   classTypes);

    auto requestMethodField = makeField("requestMethod", LSPMethod);
    auto ResponseMessageResultType = makeDiscriminatedUnion(
        requestMethodField,
        {
            {"__GETCOUNTERS__", SorbetCounters},
            {"initialize", InitializeResult},
            {"shutdown", JSONNull},
            // DocumentHighlight[] | null
            {"textDocument/documentHighlight", makeVariant({JSONNull, makeArray(DocumentHighlight)})},
            // DocumentSymbol[] | SymbolInformation[] | null
            // Sorbet only uses DocumentSymbol[].
            {"textDocument/documentSymbol", makeVariant({JSONNull, makeArray(DocumentSymbol)})},
            // Location | Location[] | LocationLink[] | null
            // Sorbet only uses Location[].
            {"textDocument/definition", makeVariant({JSONNull, makeArray(Location)})},
            {"textDocument/typeDefinition", makeVariant({JSONNull, makeArray(Location)})},
            {"textDocument/hover", makeVariant({JSONNull, Hover})},
            // CompletionItem[] | CompletionList | null
            // Sorbet only sends CompletionList.
            {"textDocument/completion", CompletionList},
            {"textDocument/prepareRename", makeVariant({JSONNull, PrepareRenameResult})},
            {"textDocument/references", makeVariant({JSONNull, makeArray(Location)})},
            {"textDocument/rename", makeVariant({JSONNull, WorkspaceEdit})},
            {"textDocument/signatureHelp", makeVariant({JSONNull, SignatureHelp})},
            {"textDocument/formatting", makeVariant({JSONNull, makeArray(TextEdit)})},
            // (CodeAction | Command)[] | null
            // Sorbet only sends CodeAction[].
            {"textDocument/codeAction", makeVariant({JSONNull, makeArray(CodeAction)})},
            // TODO: the following are more correct but I can only get the above to work.
            // {"textDocument/codeAction", makeVariant({JSONNull, makeArray(makeVariant({CodeAction, Command}))})},
            // {"textDocument/codeAction", makeVariant({JSONNull, makeArray(CodeAction), makeArray(Command)})},
            {"codeAction/resolve", makeOptional(CodeAction)},
            {"textDocument/implementation", makeVariant({JSONNull, makeArray(Location)})},
            {"workspace/symbol", makeVariant({JSONNull, makeArray(SymbolInformation)})},
            {"sorbet/error", SorbetErrorParams},
            {"sorbet/readFile", TextDocumentItem},
            {"sorbet/showSymbol", makeVariant({JSONNull, SymbolInformation})},
        });
    // N.B.: ResponseMessage.params must be optional, as it is not present when an error occurs.
    // N.B.: We add a 'requestMethod' field to response messages to make the discriminated union work.
    // Also note that we cannot name this field 'method', as it tricks clients into thinking it's a request
    // rather than a response.
    auto ResponseMessage = makeObject("ResponseMessage",
                                      {makeField("jsonrpc", JSONRPCConstant),
                                       makeField("id", makeVariant({JSONInt, JSONString, JSONNull})),
                                       requestMethodField, makeField("result", makeOptional(ResponseMessageResultType)),
                                       makeField("error", makeOptional(ResponseError))},
                                      classTypes);

    auto NotificationMessageParamsType =
        makeDiscriminatedUnion(methodField, {
                                                {"$/cancelRequest", CancelParams},
                                                {"initialized", InitializedParams},
                                                {"exit", makeOptional(JSONNull)},
                                                {"textDocument/publishDiagnostics", PublishDiagnosticsParams},
                                                {"textDocument/didOpen", DidOpenTextDocumentParams},
                                                {"textDocument/didChange", DidChangeTextDocumentParams},
                                                {"textDocument/didClose", DidCloseTextDocumentParams},
                                                {"window/showMessage", ShowMessageParams},
                                                {"__PAUSE__", makeOptional(JSONNull)},
                                                {"__RESUME__", makeOptional(JSONNull)},
                                                {"sorbet/indexerInitialization", makeOptional(JSONNull)},
                                                {"sorbet/watchmanFileChange", WatchmanQueryResponse},
                                                {"sorbet/watchmanStateEnter", WatchmanStateEnter},
                                                {"sorbet/watchmanStateLeave", WatchmanStateLeave},
                                                {"sorbet/showOperation", SorbetShowOperationParams},
                                                {"sorbet/error", SorbetErrorParams},
                                                {"sorbet/fence", JSONInt},
                                                {"sorbet/workspaceEdit", SorbetWorkspaceEditParams},
                                                {"sorbet/typecheckRunInfo", SorbetTypecheckRunInfo},
                                                {"workspace/didChangeConfiguration", DidChangeConfigurationParams},
                                            });
    auto NotificationMessage = makeObject("NotificationMessage",
                                          {makeField("jsonrpc", JSONRPCConstant), makeField("method", LSPMethod),
                                           makeField("params", NotificationMessageParamsType)},
                                          classTypes);
}
