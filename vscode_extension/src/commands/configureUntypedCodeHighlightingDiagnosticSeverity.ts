import { QuickPickItem, window, DiagnosticSeverity } from "vscode";
import { ALL_DIAGNOSTIC_SEVERITY } from "../config";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

export interface DiagnosticSeverityQuickPickItem extends QuickPickItem {
  severity: DiagnosticSeverity;
}

async function cycleStates(
  context: SorbetExtensionContext,
  targetState: DiagnosticSeverity,
  forCommand: string,
) {
  const oldHighlightUntyped = context.configuration.highlightUntyped;
  context.configuration.oldHighlightUntyped = oldHighlightUntyped;

  await context.configuration.setHighlightUntypedDiagnosticSeverity(
    targetState,
  );
  context.log.info(
    forCommand,
    "Untyped code highlighting diagnostic severity",
    targetState,
  );
}

/**
 * Set highlighting of untyped code to specific setting.
 * @param context Sorbet extension context.
 */
export async function configureUntypedCodeHighlightingDiagnosticSeverity(
  context: SorbetExtensionContext,
): Promise<DiagnosticSeverity | null> {
  const items: DiagnosticSeverityQuickPickItem[] = ALL_DIAGNOSTIC_SEVERITY.map(
    (severity) => {
      return {
        label: severity,
        // vscode.DiagnosticSeverity is 0-indexed, LSP spec is 1-indexed 🤦‍♂️
        //
        // - <https://microsoft.github.io/language-server-protocol/specifications/lsp/3.17/specification/#diagnosticSeverity>
        // - <https://code.visualstudio.com/api/references/vscode-api#DiagnosticSeverity>
        severity: DiagnosticSeverity[severity] + 1,
      };
    },
  );

  const selectedItem = await window.showQuickPick(items, {
    placeHolder:
      "Select which diagnostic level to use when highlighting untyped",
  });

  if (selectedItem) {
    const targetState = selectedItem.severity;
    await cycleStates(
      context,
      targetState,
      "ConfigureUntypedDiagnosticSeverity",
    );

    const { activeLanguageClient: client } = context.statusProvider;
    if (client) {
      client.sendNotification("workspace/didChangeConfiguration", {
        settings: {
          highlightUntypedDiagnosticSeverity: targetState,
        },
      });
    } else {
      context.log.debug(
        "ConfigureUntypedDiagnosticSeverity: No active Sorbet LSP to notify.",
      );
    }

    return targetState;
  }

  return null;
}
