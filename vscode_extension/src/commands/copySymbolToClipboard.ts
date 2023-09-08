import { env, ProgressLocation, window } from "vscode";
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

  if (!client) {
    context.log.warning("CopySymbol: No active Sorbet LSP.");
    return;
  }

  if (!client.capabilities?.sorbetShowSymbolProvider) {
    context.log.warning(
      "CopySymbol: Sorbet LSP does not support 'showSymbol' capability.",
    );
    return;
  }

  const editor = window.activeTextEditor;
  if (!editor) {
    context.log.debug("CopySymbol: No active editor, no target symbol.");
    return;
  }

  if (!editor.selection.isEmpty) {
    context.log.debug(
      "CopySymbol: Non-empty selection, cannot determine target symbol.",
    );
    return;
  }

  if (client.status !== ServerStatus.RUNNING) {
    context.log.warning("CopySymbol: Sorbet LSP is not ready.");
    return;
  }

  const position = editor.selection.active;
  const params: TextDocumentPositionParams = {
    textDocument: {
      uri: editor.document.uri.toString(),
    },
    position,
  };

  let response: SymbolInformation | undefined;
  if (context.statusProvider.operations.length) {
    response = await window.withProgress(
      {
        cancellable: true,
        location: ProgressLocation.Notification,
      },
      async (progress, token) => {
        progress.report({ message: "Querying Sorbet …" });
        const r = await client.sendRequest<SymbolInformation>(
          "sorbet/showSymbol",
          params,
        );

        if (token.isCancellationRequested) {
          context.log.debug(
            `CopySymbol: Ignored canceled operation result. Symbol:${r.name}`,
          );
          return undefined;
        } else {
          return r;
        }
      },
    );
  } else {
    response = await client.sendRequest<SymbolInformation>(
      "sorbet/showSymbol",
      params,
    );
  }

  if (response) {
    await env.clipboard.writeText(response.name);
    context.log.debug(
      `CopySymbol: Copied symbol name. Symbol:${response.name}`,
    );
  }
}
