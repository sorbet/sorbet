import { TrackUntyped, backwardsCompatibleTrackUntyped } from "../config";
import { Log } from "../log";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

function toggle(log: Log, trackWhere: TrackUntyped): TrackUntyped {
  switch (trackWhere) {
    case "nowhere":
      return "everywhere";
    case "everywhere":
      return "nowhere";
    default:
      const exhaustiveCheck: never = trackWhere;
      log.warning(`Got unexpected state: ${exhaustiveCheck}`);
      return "nowhere";
  }
}

/**
 * Toggle highlighting of untyped code.
 * @param context Sorbet extension context.
 * @returns `true` if highlighting is now enabled, `false` otherwise.
 */
export async function toggleUntypedCodeHighlighting(
  context: SorbetExtensionContext,
): Promise<TrackUntyped> {
  const targetState = toggle(
    context.log,
    context.configuration.highlightUntyped,
  );
  await context.configuration.setHighlightUntyped(targetState);
  context.log.info(`ToggleUntyped: Untyped code highlighting: ${targetState}`);

  const { activeLanguageClient: client } = context.statusProvider;
  if (client) {
    client.sendNotification("workspace/didChangeConfiguration", {
      settings: {
        highlightUntyped: backwardsCompatibleTrackUntyped(
          context.log,
          targetState,
        ),
      },
    });
  } else {
    context.log.debug("ToggleUntyped: No active Sorbet LSP to notify.");
  }

  return targetState;
}
