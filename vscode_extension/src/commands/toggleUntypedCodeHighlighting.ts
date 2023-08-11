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
  const { activeLanguageClient: client } = context.statusProvider;
  if (!client || client === undefined) {
    context.log.debug("ToggleUntyped: No active Sorbet LSP.");
  }

  context.log.info(
    `ToggleUntyped: Untyped code highlighting: ${
      targetState ? "enabled" : "disabled"
    }`,
  );

  client?.sendNotification("workspace/didChangeConfiguration", {
    settings: {
      highlightUntyped: targetState,
    },
  });

  return context.configuration.highlightUntyped;
}
