import { commands, Position, Uri } from "vscode";
import { TextDocumentPositionParams } from "vscode-languageclient";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { ServerStatus } from "../types";

/**
 * Rename symbol at {@link TextDocumentPositionParams position}.
 *
 * Unfortunately, we need this command as a wrapper around `editor.action.rename`,
 * because VSCode doesn't allow calling it from the JSON RPC
 * https://github.com/microsoft/vscode/issues/146767
 *
 * @param context Sorbet extension context.
 * @param params Document position.
 */
export async function renameSymbol(
  context: SorbetExtensionContext,
  params: TextDocumentPositionParams,
): Promise<void> {
  if (context.statusProvider.serverStatus !== ServerStatus.RUNNING) {
    context.log.warning("Sorbet LSP client is not ready.");
    return;
  }

  const {
    textDocument: { uri },
    position: { line, character },
  } = params;

  try {
    commands.executeCommand("editor.action.rename", [
      Uri.parse(uri),
      new Position(line, character),
    ]);
  } catch (error) {
    context.log.error(
      `Failed to rename symbol at ${uri}:${line}:${character}`,
      error instanceof Error ? error : undefined,
    );
  }
}
