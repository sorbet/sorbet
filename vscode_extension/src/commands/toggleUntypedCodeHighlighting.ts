import { QuickPickItem, window } from "vscode";
import { TrackUntyped, backwardsCompatibleTrackUntyped, SorbetExtensionConfig } from "../config";
import { Log } from "../log";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

export interface TrackUntypedQuickPickItem extends QuickPickItem {
  trackWhere: TrackUntyped;
}

function toggle(log: Log, configuration: SorbetExtensionConfig): TrackUntyped {
  if (configuration.oldHighlightUntyped != null) {
    return configuration.oldHighlightUntyped;
  }

  switch (configuration.highlightUntyped) {
    case "nowhere":
      return "everywhere";
    case "everywhere-but-tests":
      return "nowhere";
    case "everywhere":
      return "nowhere";
    default:
      const exhaustiveCheck: never = configuration.highlightUntyped;
      log.warning(`Got unexpected state: ${exhaustiveCheck}`);
      return "nowhere";
  }
}

/**
 * Toggle highlighting of untyped code.
 * @param context Sorbet extension context.
 * @returns The new TrackUntyped setting
 */
export async function toggleUntypedCodeHighlighting(
  context: SorbetExtensionContext,
): Promise<TrackUntyped> {
  const targetState = toggle(context.log, context.configuration);
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

/**
 * Set highlighting of untyped code to sepcific setting.
 * @param context Sorbet extension context.
 */
export async function configureUntypedCodeHighlighting(
  context: SorbetExtensionContext,
): Promise<void> {
  const items: TrackUntypedQuickPickItem[] = [
    {
      label: "Nowhere",
      trackWhere: "nowhere",
    },
    {
      label: "Everywhere but tests",
      trackWhere: "everywhere-but-tests",
    },
    {
      label: "Everywhere",
      trackWhere: "everywhere",
    },
  ];

  const selectedItem = await window.showQuickPick(items, {
    placeHolder: "Select where to highlight untyped code",
  });

  if (selectedItem) {
    const oldHighlightUntyped = context.configuration.highlightUntyped;
    context.configuration.oldHighlightUntyped = oldHighlightUntyped;

    const targetState = selectedItem.trackWhere;
    await context.configuration.setHighlightUntyped(targetState);
    context.log.info(
      `ConfigureUntyped: Untyped code highlighting: ${targetState}`,
    );

    const { activeLanguageClient: client } = context.statusProvider;
    if (client) {
      client.sendNotification("workspace/didChangeConfiguration", {
        settings: {
          highlightUntyped: targetState,
        },
      });
    } else {
      context.log.debug("ConfigureUntyped: No active Sorbet LSP to notify.");
    }
  }
}
