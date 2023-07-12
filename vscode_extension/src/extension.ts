import { commands, ExtensionContext, workspace } from "vscode";
import { TextDocumentPositionParams } from "vscode-languageclient";
import * as cmdIds from "./commandIds";
import { copySymbolToClipboard } from "./commands/copySymbolToClipboard";
import { renameSymbol } from "./commands/renameSymbol";
import { setLogLevel } from "./commands/setLogLevel";
import { showSorbetActions } from "./commands/showSorbetActions";
import { showSorbetConfigurationPicker } from "./commands/showSorbetConfigurationPicker";
import { toggleUntypedCodeHighlighting } from "./commands/toggleUntypedCodeHighlighting";
import { getLogLevelFromEnvironment, LogLevel } from "./log";
import { SorbetContentProvider, SORBET_SCHEME } from "./sorbetContentProvider";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { SorbetStatusBarEntry } from "./sorbetStatusBarEntry";
import { ServerStatus, RestartReason } from "./types";

/**
 * Extension entrypoint.
 */
export function activate(context: ExtensionContext) {
  const sorbetExtensionContext = new SorbetExtensionContext(context);
  sorbetExtensionContext.log.level = getLogLevelFromEnvironment();

  context.subscriptions.push(
    sorbetExtensionContext,
    sorbetExtensionContext.configuration.onLspConfigChange(
      async ({ oldLspConfig, newLspConfig }) => {
        const { statusProvider } = sorbetExtensionContext;
        if (oldLspConfig && newLspConfig) {
          // Something about the config changed, so restart
          await statusProvider.restartSorbet(RestartReason.CONFIG_CHANGE);
        } else if (oldLspConfig) {
          await statusProvider.stopSorbet(ServerStatus.DISABLED);
        } else {
          await statusProvider.startSorbet();
        }
      },
    ),
  );

  const statusBarEntry = new SorbetStatusBarEntry(sorbetExtensionContext);
  context.subscriptions.push(statusBarEntry);

  // Register providers
  context.subscriptions.push(
    workspace.registerTextDocumentContentProvider(
      SORBET_SCHEME,
      new SorbetContentProvider(sorbetExtensionContext),
    ),
  );

  // Register commands
  context.subscriptions.push(
    commands.registerCommand(cmdIds.COPY_SYMBOL_COMMAND_ID, () =>
      copySymbolToClipboard(sorbetExtensionContext),
    ),
    commands.registerCommand(
      cmdIds.RENAME_SYMBOL_COMMAND_ID,
      (params: TextDocumentPositionParams) =>
        renameSymbol(sorbetExtensionContext, params),
    ),
    commands.registerCommand(
      cmdIds.SET_LOGLEVEL_COMMAND_ID,
      (level?: LogLevel) => setLogLevel(sorbetExtensionContext, level),
    ),
    commands.registerCommand(cmdIds.SHOW_ACTIONS_COMMAND_ID, () =>
      showSorbetActions(sorbetExtensionContext),
    ),
    commands.registerCommand(cmdIds.SHOW_CONFIG_PICKER_COMMAND_ID, () =>
      showSorbetConfigurationPicker(sorbetExtensionContext),
    ),
    commands.registerCommand(cmdIds.SHOW_OUTPUT_COMMAND_ID, () =>
      sorbetExtensionContext.logOutputChannel.show(true),
    ),
    commands.registerCommand(cmdIds.SORBET_ENABLE_COMMAND_ID, () =>
      sorbetExtensionContext.configuration.setEnabled(true),
    ),
    commands.registerCommand(cmdIds.SORBET_DISABLE_COMMAND_ID, () =>
      sorbetExtensionContext.configuration.setEnabled(false),
    ),
    commands.registerCommand(
      cmdIds.SORBET_RESTART_COMMAND_ID,
      (reason: RestartReason = RestartReason.COMMAND) =>
        sorbetExtensionContext.statusProvider.restartSorbet(reason),
    ),
    commands.registerCommand(cmdIds.TOGGLE_HIGHLIGHT_UNTYPED_COMMAND_ID, () =>
      toggleUntypedCodeHighlighting(sorbetExtensionContext),
    ),
  );

  // Start the extension.
  return sorbetExtensionContext.statusProvider.startSorbet();
}
