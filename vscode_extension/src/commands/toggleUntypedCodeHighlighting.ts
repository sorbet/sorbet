import { SorbetExtensionContext } from "../sorbetExtensionContext";

/**
 * Toggle highlighting of untyped code.
 * @param context Sorbet extension context.
 * @returns `true` if highlighting is now enabled, `false` otherwise.
 */
export async function toggleUntypedCodeHighlighting(
  context: SorbetExtensionContext,
): Promise<boolean> {
  const targetState = !context.configuration.highlightUntyped;
  await context.configuration.setHighlightUntyped(targetState);
  context.log.info(
    `ToggleUntyped: Untyped code highlighting: ${
      targetState ? "enabled" : "disabled"
    }`,
  );

  const { activeLanguageClient: client } = context.statusProvider;
  if (client) {
    client.sendNotification("workspace/didChangeConfiguration", {
      settings: {
        highlightUntyped: targetState,
      },
    });
  } else {
    context.log.debug("ToggleUntyped: No active Sorbet LSP to notify.");
  }

  return targetState;
}
