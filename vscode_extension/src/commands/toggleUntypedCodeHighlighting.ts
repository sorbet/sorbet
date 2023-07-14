import { DidChangeConfigurationParams } from "vscode-languageclient/node";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { ServerStatus } from "../types";

/**
 * Toggle highlighting of untyped code.
 * @param context Sorbet extension context.
 * @returns `true` if highlighting is now enabled, `false` otherwise.
 */
export async function toggleUntypedCodeHighlighting(
  context: SorbetExtensionContext,
): Promise<boolean> {
  const { activeLanguageClient: client } = context.statusProvider;
  if (client?.status !== ServerStatus.RUNNING) {
    context.log.warning("Sorbet LSP client is not ready.");
    return false;
  }

  const params: DidChangeConfigurationParams = {
    settings: {
      toggleUntypedCodeHighlighting: !context.configuration.highlightUntyped,
    },
  };

  // TODO: this is not very OO - this should be `await client.setXXXX(value)`
  const response: boolean = await client.languageClient.sendRequest(
    "workspace/didChangeConfiguration", // This could also be "sorbet/toggleUntypedCodeHighlighting", no params
    params,
  );

  if (response !== context.configuration.highlightUntyped) {
    // TODO: This would internally call `refresh` which is not right anymore. This can
    // really be changed to simple getter/setter.
    // TODO: if changed to a simple prop, can we read this from Sorbet?  This is why it was preserved as a Memento
    // (I think) so rather than read current value from Sorbet, the memento value is actually forced on it.
    // TODO: Unexpectedly, `refresh` does not fire any event because old/new configurations
    // are equal (toggle is not part of them), so the new logic actually would be an improvement as a simple setter.
    context.configuration.setHighlightUntyped(response);
  }

  return context.configuration.highlightUntyped;
}
