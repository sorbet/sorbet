import { TextDocumentContentProvider, Uri } from "vscode";
import { CancellationToken, TextDocumentItem } from "vscode-languageclient";
import { SorbetExtensionContext } from "./sorbetExtensionContext";

/**
 * URI scheme supported by {@link SorbetContentProvider}.
 */
export const SORBET_SCHEME = "sorbet";

/**
 * Content provider for URIs with `sorbet` scheme.
 */
export class SorbetContentProvider implements TextDocumentContentProvider {
  private readonly context: SorbetExtensionContext;

  constructor(context: SorbetExtensionContext) {
    this.context = context;
  }

  /**
   * Provide textual content for a given uri.
   */
  public async provideTextDocumentContent(
    uri: Uri,
    token?: CancellationToken,
  ): Promise<string> {
    let content: string;
    const { activeLanguageClient: client } = this.context.statusProvider;
    if (client) {
      this.context.log.info(`Retrieving file contents. URI:${uri}`);
      const response = await client.sendRequest<TextDocumentItem>(
        "sorbet/readFile",
        {
          uri: uri.toString(),
        },
        token,
      );
      content = response.text;
    } else {
      this.context.log.info(
        `Cannot retrieve file contents, no active Sorbet client. URI:${uri}`,
      );
      content = "";
    }
    return content;
  }
}
