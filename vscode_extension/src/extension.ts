import {
  ExtensionContext,
  commands,
  window,
  TextDocumentContentProvider,
  Uri,
  workspace,
} from "vscode";
import { TextDocumentItem } from "vscode-languageclient";

import SorbetConfigPicker from "./ConfigPicker";
import {
  SorbetExtensionConfig,
  SorbetLspConfigChangeEvent,
  DefaultSorbetWorkspaceContext,
} from "./config";
import SorbetLanguageClient from "./LanguageClient";
import { ServerStatus, RestartReason } from "./types";
import SorbetStatusBarEntry from "./SorbetStatusBarEntry";
import { emitCountMetric } from "./veneur";

/**
 * Extension entrypoint.
 */
export function activate(context: ExtensionContext) {
  const sorbetExtensionConfig = new SorbetExtensionConfig(
    new DefaultSorbetWorkspaceContext(context),
  );
  sorbetExtensionConfig.onLspConfigChange(handleConfigChange);
  const outputChannel = window.createOutputChannel("Sorbet");
  const statusBarEntry = new SorbetStatusBarEntry(
    outputChannel,
    sorbetExtensionConfig,
    restartSorbet,
  );

  const emitMetric = emitCountMetric.bind(
    null,
    sorbetExtensionConfig,
    outputChannel,
  );

  let activeSorbetLanguageClient: SorbetLanguageClient | null = null;
  context.subscriptions.push(outputChannel, statusBarEntry);

  function stopSorbet(newStatus: ServerStatus) {
    if (activeSorbetLanguageClient) {
      activeSorbetLanguageClient.dispose();
      // Garbage collect the language client.
      const i = context.subscriptions.indexOf(activeSorbetLanguageClient);
      if (i !== -1) {
        context.subscriptions.splice(i, 1);
      }
      activeSorbetLanguageClient = null;
    }
    // Reset status bar state impacted by previous language client.
    statusBarEntry.clearOperations();
    statusBarEntry.changeServerStatus(newStatus);
  }

  function restartSorbet(reason: RestartReason) {
    stopSorbet(ServerStatus.RESTARTING);

    // NOTE: `reason` is an enum type with a small and finite number of values.
    emitMetric(`restart.${reason}`, 1);
    startSorbet();
  }

  let lastSorbetRetryTime = 0;
  const minTimeBetweenRetries = 7000;
  // Mutex for startSorbet. Prevents us from starting multiple processes at once.
  let isStarting = false;
  async function startSorbet() {
    if (isStarting) return;

    const currentTime = Date.now();
    // Debounce by 7 seconds. Returns 0 if the calculated time to sleep is negative.
    const timeToSleep = Math.max(
      0,
      minTimeBetweenRetries - (currentTime - lastSorbetRetryTime),
    );
    if (timeToSleep > 0) {
      console.log(
        `Waiting ${timeToSleep.toFixed(0)} ms before restarting Sorbet...`,
      );
    }

    // Wait timeToSleep ms. Use mutex, as this yields the event loop for future events.
    isStarting = true;
    await new Promise((res) => setTimeout(res, timeToSleep));
    isStarting = false;

    lastSorbetRetryTime = Date.now();

    const sorbet = new SorbetLanguageClient(
      sorbetExtensionConfig,
      outputChannel,
      filterUpdatesFromOldClients(restartSorbet),
    );
    activeSorbetLanguageClient = sorbet;
    context.subscriptions.push(activeSorbetLanguageClient);

    // Helper function. Drops any status updates and operations from old clients that are in the process of shutting down.
    function filterUpdatesFromOldClients<TS extends any[]>(
      fn: (...args: TS) => void,
    ): (...args: TS) => void {
      return (...args: TS): void => {
        if (activeSorbetLanguageClient !== sorbet) {
          return;
        }
        return fn(...args);
      };
    }

    // Pipe updates to status bar, and reset status bar state impacted by previous language client.
    statusBarEntry.changeServerStatus(sorbet.status, sorbet.lastError);

    sorbet.onStatusChange = filterUpdatesFromOldClients(
      (status: ServerStatus) => {
        statusBarEntry.changeServerStatus(status, sorbet.lastError);
      },
    );

    sorbet.languageClient.onReady().then(
      filterUpdatesFromOldClients(() => {
        sorbet.languageClient.onNotification(
          "sorbet/showOperation",
          filterUpdatesFromOldClients(
            statusBarEntry.handleShowOperation.bind(statusBarEntry),
          ),
        );
      }),
    );
  }

  context.subscriptions.push(
    commands.registerCommand("sorbet.enable", () => {
      sorbetExtensionConfig.setEnabled(true);
    }),
  );

  context.subscriptions.push(
    commands.registerCommand("sorbet.disable", () => {
      sorbetExtensionConfig.setEnabled(false);
    }),
  );

  context.subscriptions.push(
    commands.registerCommand("sorbet.restart", () => {
      restartSorbet(RestartReason.COMMAND);
    }),
  );

  context.subscriptions.push(
    commands.registerCommand("sorbet.configure", () => {
      new SorbetConfigPicker(sorbetExtensionConfig).show();
    }),
  );

  context.subscriptions.push(
    commands.registerCommand("sorbet.showOutput", () => {
      outputChannel.show();
    }),
  );

  const provider: TextDocumentContentProvider = {
    provideTextDocumentContent: async (uri: Uri): Promise<string> => {
      // URIs are of the form sorbet:[file_path]
      console.log(`Opening sorbet: file at uri ${uri.toString()}`);
      if (activeSorbetLanguageClient) {
        const response: TextDocumentItem = await activeSorbetLanguageClient.languageClient.sendRequest(
          "sorbet/readFile",
          {
            uri: uri.toString(),
          },
        );
        return response.text;
      }
      return "";
    },
  };
  context.subscriptions.push(
    workspace.registerTextDocumentContentProvider("sorbet", provider),
  );

  function handleConfigChange(event: SorbetLspConfigChangeEvent) {
    const { oldLspConfig, newLspConfig } = event;
    if (oldLspConfig && newLspConfig) {
      // Something about the config changed, so restart
      restartSorbet(RestartReason.CONFIG_CHANGE);
    } else if (oldLspConfig) {
      // Stop using Sorbet
      stopSorbet(ServerStatus.DISABLED);
    } else if (newLspConfig) {
      // Start using Sorbet
      startSorbet();
    }
  }

  // Start the extension.
  handleConfigChange({
    oldLspConfig: undefined,
    newLspConfig: sorbetExtensionConfig.activeLspConfig,
  });
}
