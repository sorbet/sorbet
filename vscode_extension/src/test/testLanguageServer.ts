import {
  createConnection,
  ProposedFeatures,
  InitializeParams,
  TextDocumentSyncKind,
} from "vscode-languageserver/node";
import { TestLanguageServerSpecialURIs } from "./testLanguageServerSpecialURIs";

// Create a connection for the server. The connection uses Node's IPC as a transport.
// Also include all preview / proposed LSP features.
const connection = createConnection(ProposedFeatures.all);

connection.onInitialize((_: InitializeParams) => {
  return {
    capabilities: {
      textDocumentSync: TextDocumentSyncKind.Full,
      // Tell the client that the server supports code completion
      completionProvider: {
        resolveProvider: true,
      },
    },
  };
});

connection.onInitialized(() => {});

connection.onHover((e) => {
  switch (e.textDocument.uri) {
    case TestLanguageServerSpecialURIs.SUCCESS:
      return { contents: TestLanguageServerSpecialURIs.SUCCESS };
    case TestLanguageServerSpecialURIs.FAILURE:
      throw new Error(TestLanguageServerSpecialURIs.FAILURE);
    case TestLanguageServerSpecialURIs.EXIT:
      process.exit(1);
      break; // Unreachable, but eslint doesn't know that.
    default:
      throw new Error("Invalid request.");
  }
});

// Listen on the connection
connection.listen();
