import {
  commands,
  Location as VScodeLocation,
  Position as VSCodePosition,
  Range as VSCodeRange,
  Uri,
  ProgressLocation,
  window,
} from "vscode";
import { Location } from "vscode-languageclient/node";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { ServerStatus } from "../types";

interface ReferenceParams {
  textDocument: { uri: string };
  position: { line: number; character: number };
  context: { includeDeclaration: boolean };
}

/**
 * Find references to the symbol at the cursor, including references to
 * overridden and overriding symbols throughout the inheritance hierarchy.
 */
export async function findHierarchyReferences(
  context: SorbetExtensionContext,
): Promise<void> {
  const { activeLanguageClient: client } = context.statusProvider;

  if (!client) {
    context.log.warn("HierarchyReferences: No active Sorbet LSP.");
    return;
  }

  if (!client.capabilities?.sorbetHierarchyReferencesProvider) {
    context.log.warn(
      "HierarchyReferences: Sorbet LSP does not support 'hierarchyReferences' capability.",
    );
    return;
  }

  const editor = window.activeTextEditor;
  if (!editor) {
    context.log.debug(
      "HierarchyReferences: No active editor, no target symbol.",
    );
    return;
  }

  if (!editor.selection.isEmpty) {
    context.log.debug(
      "HierarchyReferences: Non-empty selection, cannot determine target symbol.",
    );
    return;
  }

  if (client.status !== ServerStatus.RUNNING) {
    context.log.warn("HierarchyReferences: Sorbet LSP is not ready.");
    return;
  }

  const position = editor.selection.active;
  const params: ReferenceParams = {
    textDocument: {
      uri: editor.document.uri.toString(),
    },
    position,
    context: {
      includeDeclaration: true,
    },
  };

  let response: Location[] | null;
  if (context.statusProvider.operations.length) {
    response = await window.withProgress(
      {
        cancellable: true,
        location: ProgressLocation.Notification,
      },
      async (progress, token) => {
        progress.report({ message: "Finding hierarchy references\u2026" });
        const r = await client.sendRequest<Location[] | null>(
          "sorbet/hierarchyReferences",
          params,
        );

        if (token.isCancellationRequested) {
          context.log.debug(
            "HierarchyReferences: Ignored canceled operation result.",
          );
          return null;
        } else {
          return r;
        }
      },
    );
  } else {
    response = await client.sendRequest<Location[] | null>(
      "sorbet/hierarchyReferences",
      params,
    );
  }

  if (!response || response.length === 0) {
    context.log.debug("HierarchyReferences: No results.");
    return;
  }

  const locations = response.map(
    (loc) =>
      new VScodeLocation(
        Uri.parse(loc.uri),
        new VSCodeRange(
          new VSCodePosition(loc.range.start.line, loc.range.start.character),
          new VSCodePosition(loc.range.end.line, loc.range.end.character),
        ),
      ),
  );

  await commands.executeCommand(
    "editor.action.showReferences",
    editor.document.uri,
    position,
    locations,
  );
}
