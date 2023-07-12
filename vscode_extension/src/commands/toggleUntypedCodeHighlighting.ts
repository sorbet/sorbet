import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { RestartReason } from "../types";

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

  await context.statusProvider.restartSorbet(RestartReason.CONFIG_CHANGE);
  return context.configuration.highlightUntyped;
}
