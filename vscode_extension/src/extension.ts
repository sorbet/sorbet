import { commands, ExtensionContext, Uri, workspace } from "vscode";
import { TextDocumentItem } from "vscode-languageclient";
import {
  SHOW_ACTIONS_COMMAND_ID,
  SHOW_CONFIG_PICKER_COMMAND_ID,
  SHOW_OUTPUT_COMMAND_ID,
  SORBET_DISABLE_COMMAND_ID,
  SORBET_ENABLE_COMMAND_ID,
  SORBET_RESTART_COMMAND_ID,
} from "./commandIds";
import { ShowSorbetActions } from "./commands/showSorbetActions";
import ShowSorbetConfigurationPicker from "./commands/showSorbetConfigurationPicker";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { SorbetStatusBarEntry } from "./sorbetStatusBarEntry";
import { ServerStatus, RestartReason } from "./types";

/**
 * Extension entrypoint.
 */
export function activate(context: ExtensionContext) {
  const sorbetExtensionContext = new SorbetExtensionContext(context);
  context.subscriptions.push(
    sorbetExtensionContext,
    sorbetExtensionContext.config.onLspConfigChange(
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
        const {
          activeSorbetLanguageClient,
        } = sorbetExtensionContext.statusProvider;
        console.log(`Opening sorbet: file. URI:${uri}`);
        if (activeSorbetLanguageClient) {
          const response: TextDocumentItem = await activeSorbetLanguageClient.languageClient.sendRequest(
            "sorbet/readFile",
            {
              uri: uri.toString(),
            },
          );
          content = response.text;
        } else {
          console.log(" > Cannot retrieve file content, no active client.");
          content = "";
        }
        return content;
      },
    }),
  );

  // Register commands
  context.subscriptions.push(
    commands.registerCommand(SHOW_ACTIONS_COMMAND_ID, () =>
      new ShowSorbetActions(sorbetExtensionContext).execute(),
    ),
    commands.registerCommand(SHOW_CONFIG_PICKER_COMMAND_ID, () =>
      new ShowSorbetConfigurationPicker(sorbetExtensionContext).execute(),
    ),
    commands.registerCommand(SHOW_OUTPUT_COMMAND_ID, () =>
      sorbetExtensionContext.outputChannel.show(),
    ),
    commands.registerCommand(SORBET_ENABLE_COMMAND_ID, () =>
      sorbetExtensionContext.config.setEnabled(true),
    ),
    commands.registerCommand(SORBET_DISABLE_COMMAND_ID, () =>
      sorbetExtensionContext.config.setEnabled(false),
    ),
    commands.registerCommand(
      SORBET_RESTART_COMMAND_ID,
      (reason: RestartReason = RestartReason.COMMAND) =>
        sorbetExtensionContext.statusProvider.restartSorbet(reason),
    ),
    commands.registerCommand("sorbet.toggleHighlightUntyped", () =>
      sorbetExtensionContext.config
        .setHighlightUntyped(!sorbetExtensionContext.config.highlightUntyped)
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
