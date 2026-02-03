import {
  CancellationToken,
  InlayHint,
  InlayHintKind,
  InlayHintsProvider,
  Position,
  ProviderResult,
  Range,
  TextDocument,
} from "vscode";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { InlayTypeHintsStyle } from "./config";

/**
 * Provides inlay type hints for Ruby code using Sorbet type information.
 * Displays type annotations for variable assignments based on method return types.
 */
export class SorbetInlayHintsProvider implements InlayHintsProvider {
  private readonly context: SorbetExtensionContext;

  constructor(context: SorbetExtensionContext) {
    this.context = context;
  }

  public provideInlayHints(
    document: TextDocument,
    range: Range,
    token: CancellationToken,
  ): ProviderResult<InlayHint[]> {
    const style = this.context.configuration.inlayTypeHints;

    if (style === "off") {
      return [];
    }

    const client = this.context.statusProvider.activeLanguageClient;
    if (!client) {
      return [];
    }

    return this.requestInlayHintsFromServer(document, range, style, token);
  }

  private async requestInlayHintsFromServer(
    document: TextDocument,
    range: Range,
    style: InlayTypeHintsStyle,
    token: CancellationToken,
  ): Promise<InlayHint[]> {
    const client = this.context.statusProvider.activeLanguageClient;
    if (!client) {
      return [];
    }

    try {
      if (token.isCancellationRequested) {
        return [];
      }

      const response = await client.sendRequest<any>("textDocument/inlayHint", {
        textDocument: { uri: document.uri.toString() },
        range: {
          start: { line: range.start.line, character: range.start.character },
          end: { line: range.end.line, character: range.end.character },
        },
      });

      if (!response || !Array.isArray(response)) {
        return [];
      }

      return response
        .map((hint: any) => this.convertServerHintToVSCodeHint(hint, style, document))
        .filter((hint): hint is InlayHint => hint !== null);
    } catch (error) {
      this.context.log.debug(
        "Failed to request inlay hints from server:",
        error,
      );
      return [];
    }
  }

  private convertServerHintToVSCodeHint(
    serverHint: any,
    style: InlayTypeHintsStyle,
    document: TextDocument,
  ): InlayHint | null {
    if (!serverHint || !serverHint.position || !serverHint.label) {
      return null;
    }

    const typeLabel =
      typeof serverHint.label === "string"
        ? serverHint.label
        : serverHint.label.value || "";

    let position: Position;
    let label: string;

    switch (style) {
      case "before_var":
        position = new Position(
          serverHint.position.line,
          serverHint.position.character,
        );
        label = `${typeLabel} `;
        break;
      case "after_var":
        position = new Position(
          serverHint.position.line,
          serverHint.position.character,
        );
        label = `: ${typeLabel}`;
        break;
      case "RBS":
        // RBS style places the type at the end of the line
        const line = document.lineAt(serverHint.position.line);
        position = new Position(
          serverHint.position.line,
          line.range.end.character,
        );
        label = ` #: ${typeLabel}`;
        break;
      default:
        return null;
    }

    const hint = new InlayHint(
      position,
      label,
      serverHint.kind === 1 ? InlayHintKind.Type : InlayHintKind.Parameter,
    );

    if (serverHint.paddingLeft) {
      hint.paddingLeft = true;
    }
    if (serverHint.paddingRight) {
      hint.paddingRight = true;
    }

    return hint;
  }
}
