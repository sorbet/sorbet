import { ChildProcess, spawn } from "child_process";
import {
  workspace,
  commands,
  OutputChannel,
  window as vscodeWindow,
  env as vscodeEnv,
  Uri,
  Position,
} from "vscode";
import {
  LanguageClient,
  CloseAction,
  ErrorAction,
  ErrorHandler,
  RevealOutputChannelOn,
  SymbolInformation,
  TextDocumentPositionParams,
} from "vscode-languageclient/node";

import { stopProcess } from "./connections";
import { SorbetExtensionConfig } from "./config";
import { ServerStatus, RestartReason } from "./types";
import { emitCountMetric, emitTimingMetric, Tags } from "./veneur";

function nop() {}

const VALID_STATE_TRANSITIONS = new Map<ServerStatus, Set<ServerStatus>>();
VALID_STATE_TRANSITIONS.set(
  ServerStatus.INITIALIZING,
  new Set([ServerStatus.ERROR, ServerStatus.RUNNING, ServerStatus.RESTARTING]),
);
VALID_STATE_TRANSITIONS.set(
  ServerStatus.RUNNING,
  new Set([ServerStatus.ERROR, ServerStatus.RESTARTING]),
);
// Restarting is a terminal state. The restart occurs by terminating this LanguageClient and creating a new one.
VALID_STATE_TRANSITIONS.set(ServerStatus.RESTARTING, new Set([]));
// Error is a terminal state for this class.
VALID_STATE_TRANSITIONS.set(ServerStatus.ERROR, new Set([]));

/**
 * Shims the language client object so that all requests sent get timed. Exported for tests.
 */
export function shimLanguageClient(
  lc: LanguageClient,
  _emitTimingMetric: (metric: string, value: number | Date, tags: Tags) => void,
) {
  const originalSendRequest = lc.sendRequest;
  lc.sendRequest = function(this: LanguageClient, method: any, ...args: any[]) {
    const now = new Date();
    const requestName = typeof method === "string" ? method : method.method;
    // Replace some special characters with underscores.
    const sanitizedRequestName = requestName.replace(/[/$]/g, "_");
    args.unshift(method);
    const rv = originalSendRequest.apply(this, args as any);
    const metricName = `latency.${sanitizedRequestName}_ms`;
    rv.then(
      () => {
        // NOTE: This callback is only called if the request succeeds and was _not_ canceled.
        // If the request is canceled, the promise is rejected.
        _emitTimingMetric(metricName, now, { success: "true" });
      },
      () => {
        // This callback is called if the request failed or was canceled.
        _emitTimingMetric(metricName, now, { success: "false" });
      },
    );
    return rv;
  };
}

export default class SorbetLanguageClient implements ErrorHandler {
  private _languageClient: LanguageClient;
  public get languageClient(): LanguageClient {
    return this._languageClient;
  }

  private _status = ServerStatus.INITIALIZING;
  public get status(): ServerStatus {
    return this._status;
  }

  // If status is ERROR, contains the last error message encountered.
  private _lastError: string = "";
  public get lastError(): string {
    return this._lastError;
  }

  // Contains the Sorbet process.
  private _sorbetProcess: ChildProcess | null = null;
  // Note: sometimes this is actually an errno, not a process exit code.
  // This happens when set via the `.on("error")` handler, instead of the
  // `.on("exit")` handler.
  private _sorbetProcessExitCode: number | null = null;

  // Tracks disposable subscriptions so we can clean them up when language client is disposed.
  private _subscriptions: { dispose: () => void }[] = [];

  public onStatusChange: (status: ServerStatus) => void = nop;

  private _emitCountMetric = emitCountMetric.bind(
    null,
    this._sorbetExtensionConfig,
    this._outputChannel,
  );

  private _emitTimingMetric = emitTimingMetric.bind(
    null,
    this._sorbetExtensionConfig,
    this._outputChannel,
  );

  constructor(
    private readonly _sorbetExtensionConfig: SorbetExtensionConfig,
    private readonly _outputChannel: OutputChannel,
    private readonly _restart: (reason: RestartReason) => void,
  ) {
    // Create the language client and start the client.
    this._languageClient = new LanguageClient(
      "ruby",
      "Sorbet",
      this._startSorbetProcess.bind(this),
      {
        documentSelector: [
          { language: "ruby", scheme: "file" },
          // Support queries on generated files with sorbet:// URIs that do not exist editor-side.
          { language: "ruby", scheme: "sorbet" },
        ],
        outputChannel: this._outputChannel,
        initializationOptions: {
          // Opt in to sorbet/showOperation notifications.
          supportsOperationNotifications: true,
          // Let Sorbet know that we can handle sorbet:// URIs for generated files.
          supportsSorbetURIs: true,
        },
        errorHandler: this,
        revealOutputChannelOn: this._sorbetExtensionConfig.revealOutputOnError
          ? RevealOutputChannelOn.Error
          : RevealOutputChannelOn.Never,
      },
    );
    shimLanguageClient(this._languageClient, this._emitTimingMetric);
    this._languageClient.onReady().then(() => {
      // Note: It's possible for `onReady` to fire after `stop()` is called on the language client. :(
      if (this._status !== ServerStatus.ERROR) {
        // Language client started successfully.
        this._updateStatus(ServerStatus.RUNNING);
      }

      const caps: any = this._languageClient.initializeResult?.capabilities;
      if (caps.sorbetShowSymbolProvider) {
        this._subscriptions.push(
          commands.registerCommand("sorbet.copySymbolToClipboard", async () => {
            const editor = vscodeWindow.activeTextEditor;
            if (!editor) {
              return;
            }

            if (!editor.selection.isEmpty) {
              return; // something is selected, abort
            }

            const position = editor.selection.active;
            const params: TextDocumentPositionParams = {
              textDocument: {
                uri: editor.document.uri.toString(),
              },
              position,
            };
            const response: SymbolInformation = await this._languageClient.sendRequest(
              "sorbet/showSymbol",
              params,
            );

            await vscodeEnv.clipboard.writeText(response.name);

            console.log(`Copied ${response.name} to the clipboard.`);
          }),
        );
      }

      // Unfortunately, we need this command as a wrapper around `editor.action.rename`,
      // because VSCode doesn't allow calling it from the JSON RPC
      // https://github.com/microsoft/vscode/issues/146767
      this._subscriptions.push(
        commands.registerCommand(
          "sorbet.rename",
          (params: TextDocumentPositionParams) => {
            try {
              commands.executeCommand("editor.action.rename", [
                Uri.parse(params.textDocument.uri),
                new Position(params.position.line, params.position.character),
              ]);
            } catch (error) {
              console.log(
                `Failed to rename symbol at ${params.textDocument.uri}:${params.position.line}:${params.position.character}, ${error}`,
              );
            }
          },
        ),
      );
    });
    this._subscriptions.push(this._languageClient.start());
  }

  /**
   * Implements the disposable interface so this object can be added to the context's subscriptions
   * to keep it alive. Stops the language server and Sorbet processes, and removes UI items.
   */
  public dispose() {
    this._subscriptions.forEach((s) => s.dispose());
    this._subscriptions = [];

    let stopped = false;
    /*
     * stop() only invokes the then() callback after the language server
     * ACKs the stop request.
     * Stopping can time out if the language client is repeatedly failing to
     * start (e.g. if network is down, or path to Sorbet is incorrect), or if
     * Sorbet never ACKs the stop request.
     * In the former case (which is the common case), VS code stops retrying
     * the connection after we call stop(), but never invokes our callback.
     * Thus, our solution is to wait 5 seconds for a callback, and stop the
     * process if we haven't heard back.
     */
    const stopTimer = setTimeout(() => {
      stopped = true;
      this._emitCountMetric("stop.timed_out", 1);
      stopProcess(this._sorbetProcess);
      this._sorbetProcess = null;
    }, 5000);

    this._languageClient.stop().then(() => {
      if (!stopped) {
        clearTimeout(stopTimer);
        this._emitCountMetric("stop.success", 1);
        this._outputChannel.appendLine("Sorbet has stopped.");
      }
    });
  }

  /**
   * Updates the language client's server status. Verifies that the transition is legal.
   */
  private _updateStatus(newStatus: ServerStatus) {
    if (this._status === newStatus) {
      return;
    }
    this._assertValid(this._status, newStatus);
    this._status = newStatus;
    this.onStatusChange(newStatus);
  }

  private _assertValid(from: ServerStatus, to: ServerStatus) {
    const set = VALID_STATE_TRANSITIONS.get(from);
    if (!set || !set.has(to)) {
      this._outputChannel.appendLine(
        `Invalid Sorbet server transition: ${from} => ${to}`,
      );
    }
  }

  /**
   * Runs a Sorbet process using the current active configuration. Debounced so that it runs Sorbet at most every 3 seconds.
   */
  private _startSorbetProcess(): Promise<ChildProcess> {
    this._updateStatus(ServerStatus.INITIALIZING);
    this._outputChannel.appendLine("Running Sorbet LSP with:");
    const [
      command,
      ...args
    ] = this._sorbetExtensionConfig.activeLspConfig!.command;
    this._outputChannel.appendLine(`    ${command} ${args.join(" ")}`);
    this._sorbetProcess = spawn(command, args, {
      cwd: workspace.rootPath,
    });
    // N.B.: 'exit' is sometimes not invoked if the process exits with an error/fails to start, as per the Node.js docs.
    // So, we need to handle both events. ¯\_(ツ)_/¯
    this._sorbetProcess.on(
      "exit",
      (code: number | null, _signal: string | null) => {
        this._sorbetProcessExitCode = code;
      },
    );
    this._sorbetProcess.on("error", (err?: NodeJS.ErrnoException) => {
      if (
        err &&
        this._status === ServerStatus.INITIALIZING &&
        err.code === "ENOENT"
      ) {
        this._emitCountMetric("error.enoent", 1);
        // We failed to start the process. The path to Sorbet is likely incorrect.
        this._lastError = `Could not start Sorbet with command: '${command} ${args.join(
          " ",
        )}'. Encountered error '${
          err.message
        }'. Is the path to Sorbet correct?`;
        this._updateStatus(ServerStatus.ERROR);
      }
      this._sorbetProcess = null;
      this._sorbetProcessExitCode = err?.errno ?? null;
    });
    return Promise.resolve(this._sorbetProcess);
  }

  /** ErrorHandler interface */

  /**
   * LanguageClient has built-in restart capabilities, but it's broken:
   * * It drops all `onNotification` subscriptions after restarting, so we'll miss ShowNotification updates.
   * * It drops all `onReady` subscriptions after restarting, so we won't know when the Sorbet server is running.
   * * It doesn't reset `onReady` state, so we can't even reset our `onReady` callback.
   */
  public error(): ErrorAction {
    if (this._status !== ServerStatus.ERROR) {
      this._updateStatus(ServerStatus.RESTARTING);
      this._restart(RestartReason.CRASH_LC_ERROR);
    }
    return ErrorAction.Shutdown;
  }

  /**
   * Note: If the VPN is disconnected, then Sorbet will repeatedly fail to start.
   */
  public closed(): CloseAction {
    if (this._status !== ServerStatus.ERROR) {
      this._updateStatus(ServerStatus.RESTARTING);
      let reason = RestartReason.CRASH_LC_CLOSED;
      if (this._sorbetProcessExitCode === 11) {
        // 11 number chosen somewhat arbitrarily. Most important is that this doesn't
        // clobber the exit code of Sorbet itself (which means Sorbet cannot return 11).
        //
        // The only thing that matters is that this value is kept in sync with any
        // wrapper scripts that people use with Sorbet. If this number has to
        // change for some reason, we should announce that.
        reason = RestartReason.WRAPPER_REFUSED_SPAWN;
      } else if (this._sorbetProcessExitCode === 143) {
        // 143 = 128 + 15 and 15 is TERM signal
        reason = RestartReason.FORCIBLY_TERMINATED;
      }
      this._restart(reason);
    }
    return CloseAction.DoNotRestart;
  }
}
