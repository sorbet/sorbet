import { commands, ExtensionContext, Uri, workspace } from "vscode";
import { TextDocumentItem } from "vscode-languageclient";
import * as cmdIds from "./commandIds";
import { SetLogLevel } from "./commands/setLogLevel";
import { ShowSorbetActions } from "./commands/showSorbetActions";
import { ShowSorbetConfigurationPicker } from "./commands/showSorbetConfigurationPicker";
import { getLogLevelFromEnvironment } from "./log";
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
    workspace.registerTextDocumentContentProvider("sorbet", {
      // URIs are of the form sorbet:[file_path]
      provideTextDocumentContent: async (uri: Uri): Promise<string> => {
        let content: string;
        const { activeLanguageClient } = sorbetExtensionContext.statusProvider;
        sorbetExtensionContext.log.info(`Opening sorbet: file. URI:${uri}`);
        if (activeLanguageClient) {
          const response: TextDocumentItem = await activeLanguageClient.languageClient.sendRequest(
            "sorbet/readFile",
            {
              uri: uri.toString(),
            },
          );
          content = response.text;
        } else {
          sorbetExtensionContext.log.warning(
            " > Cannot retrieve file content, no active client.",
          );
          content = "";
        }
        return content;
      },
    }),
  );

  // Register commands
  context.subscriptions.push(
    commands.registerCommand(cmdIds.SET_LOGLEVEL_COMMAND_ID, () =>
      new SetLogLevel(sorbetExtensionContext).execute(),
    ),
    commands.registerCommand(cmdIds.SHOW_ACTIONS_COMMAND_ID, () =>
      new ShowSorbetActions(sorbetExtensionContext).execute(),
    ),
    commands.registerCommand(cmdIds.SHOW_CONFIG_PICKER_COMMAND_ID, () =>
      new ShowSorbetConfigurationPicker(sorbetExtensionContext).execute(),
    ),
    commands.registerCommand(cmdIds.SHOW_OUTPUT_COMMAND_ID, () =>
      sorbetExtensionContext.log.outputChannel.show(true),
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
    commands.registerCommand("sorbet.toggleHighlightUntyped", () =>
      sorbetExtensionContext.configuration
        .setHighlightUntyped(
          !sorbetExtensionContext.configuration.highlightUntyped,
        )
        .then(() =>
          sorbetExtensionContext.statusProvider.restartSorbet(
            RestartReason.CONFIG_CHANGE,
          ),
        ),
    ),
  );

  // Start the extension.
  return sorbetExtensionContext.statusProvider.startSorbet();
}
