import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { RestartReason } from "../types";

/**
 * Toggle the auto-complete nudge in `typed: false` files.
 * @param context Sorbet extension context.
 * @returns `true` if the nudge is enabled, `false` otherwise.
 */
export async function toggleTypedFalseCompletionNudges(
  context: SorbetExtensionContext,
): Promise<boolean> {
  const targetState = !context.configuration.typedFalseCompletionNudges;
  await context.configuration.setTypedFalseCompletionNudges(targetState);
  context.log.info(
    `Untyped file auto-complete nudge: ${targetState ? "enabled" : "disabled"}`,
  );

  await context.statusProvider.restartSorbet(RestartReason.CONFIG_CHANGE);
  return context.configuration.typedFalseCompletionNudges;
}
