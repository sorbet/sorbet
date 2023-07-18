import { env, window } from "vscode";
import {
  SymbolInformation,
  TextDocumentPositionParams,
} from "vscode-languageclient/node";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { ServerStatus } from "../types";

/**
 * Copy symbol at current.
 * @param context Sorbet extension context.
 */
export async function copySymbolToClipboard(
  context: SorbetExtensionContext,
): Promise<void> {
  const { activeLanguageClient: client } = context.statusProvider;
  if (client?.status !== ServerStatus.RUNNING) {
    context.log.warning("Sorbet LSP client is not ready.");
    return;
  }

  if (!client.capabilities?.sorbetShowSymbolProvider) {
    context.log.warning(
      'Sorbet LSP client does not support "show symbol" capability.',
    );
    return;
  }

  const editor = window.activeTextEditor;
  if (!editor) {
    context.log.debug("No active editor to copy symbol from.");
    return;
  }

  if (!editor.selection.isEmpty) {
    context.log.debug(
      "Cannot determine target symbol from a non-empty selection.",
    );
    return; // something is selected, abort
  }

  const position = editor.selection.active;
  const params: TextDocumentPositionParams = {
    textDocument: {
      uri: editor.document.uri.toString(),
    },
    position,
  };
  const response = await client.sendRequest<SymbolInformation>(
    "sorbet/showSymbol",
    params,
  );

  await env.clipboard.writeText(response.name);
  context.log.debug(
    `Copied symbol name to the clipboard. Name:${response.name}`,
  );
}
