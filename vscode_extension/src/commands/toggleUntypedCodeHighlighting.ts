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
    `Untyped code highlighting: ${targetState ? "enabled" : "disabled"}`,
  );
  
  context.statusProvider.activeLanguageClient?.sendNotification("workspace/didChangeConfiguration",
    {
      "settings": {
        "highlightUntyped": targetState
      }
    }
  );
  return context.configuration.highlightUntyped;
}
