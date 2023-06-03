import { ChildProcess, spawn } from "child_process";
import { commands, env, Position, Uri, window, workspace } from "vscode";
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
import { Tags } from "./metricsClient";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { ServerStatus, RestartReason } from "./types";

function nop() {}

const VALID_STATE_TRANSITIONS: ReadonlyMap<
  ServerStatus,
  Set<ServerStatus>
> = new Map<ServerStatus, Set<ServerStatus>>([
  [
    ServerStatus.INITIALIZING,
    new Set([
      ServerStatus.ERROR,
      ServerStatus.RESTARTING,
      ServerStatus.RUNNING,
    ]),
  ],
  [
    ServerStatus.RUNNING,
    new Set([ServerStatus.ERROR, ServerStatus.RESTARTING]),
  ],
  // Restarting is a terminal state. The restart occurs by terminating this LanguageClient and creating a new one.
  [ServerStatus.RESTARTING, new Set()],
  // Error is a terminal state for this class.
  [ServerStatus.ERROR, new Set()],
]);

/**
 * Shims the language client object so that all requests sent get timed. Exported for tests.
 */
export function shimLanguageClient(
  client: LanguageClient,
  emitTimingMetric: (metric: string, value: number | Date, tags: Tags) => void,
) {
  const originalSendRequest = client.sendRequest;
  client.sendRequest = function(
    this: LanguageClient,
    method: any,
    ...args: any[]
  ) {
    const now = new Date();
    const requestName = typeof method === "string" ? method : method.method;
    // Replace some special characters with underscores.
    const sanitizedRequestName = requestName.replace(/[/$]/g, "_");
    args.unshift(method);
    const rv = originalSendRequest.apply(this, args as any);
    const metricName = `latency.${sanitizedRequestName}_ms`;
    rv.then(
      () =>
        // NOTE: This callback is only called if the request succeeds and was _not_ canceled.
        // If the request is canceled, the promise is rejected.
        emitTimingMetric(metricName, now, { success: "true" }),
      () =>
        // This callback is called if the request failed or was canceled.
        emitTimingMetric(metricName, now, { success: "false" }),
    );
    return rv;
  };
}

export class SorbetLanguageClient implements ErrorHandler {
  private readonly context: SorbetExtensionContext;
  public readonly languageClient: LanguageClient;
  private wrappedStatus: ServerStatus;
  public get status(): ServerStatus {
    return this.wrappedStatus;
  }

  // If status is ERROR, contains the last error message encountered.
  private wrappedLastError: string;
  public get lastError(): string {
    return this.wrappedLastError;
  }

  // Contains the Sorbet process.
  private sorbetProcess: ChildProcess | null = null;
  // Note: sometimes this is actually an errno, not a process exit code.
  // This happens when set via the `.on("error")` handler, instead of the
  // `.on("exit")` handler.
  private sorbetProcessExitCode: number | null = null;

  // Tracks disposable subscriptions so we can clean them up when language client is disposed.
  private subscriptions: { dispose: () => void }[] = [];

  public onStatusChange: (status: ServerStatus) => void = nop;

  constructor(
    context: SorbetExtensionContext,
    private readonly restart: (reason: RestartReason) => void,
  ) {
    this.context = context;
    this.wrappedLastError = "";
    this.wrappedStatus = ServerStatus.INITIALIZING;

    // Create the language client and start the client.
    this.languageClient = new LanguageClient(
      "ruby",
      "Sorbet",
      this.startSorbetProcess.bind(this),
      {
        documentSelector: [
          { language: "ruby", scheme: "file" },
          // Support queries on generated files with sorbet:// URIs that do not exist editor-side.
          { language: "ruby", scheme: "sorbet" },
        ],
        outputChannel: this.context.log.outputChannel,
        initializationOptions: {
          // Opt in to sorbet/showOperation notifications.
          supportsOperationNotifications: true,
          // Let Sorbet know that we can handle sorbet:// URIs for generated files.
          supportsSorbetURIs: true,
          highlightUntyped: this.context.configuration.highlightUntyped,
        },
        errorHandler: this,
        revealOutputChannelOn: this.context.configuration.revealOutputOnError
          ? RevealOutputChannelOn.Error
          : RevealOutputChannelOn.Never,
      },
    );
    shimLanguageClient(this.languageClient, (metric, value, tags) =>
      this.context.metrics.emitTimingMetric(metric, value, tags),
    );

    this.languageClient.onReady().then(() => {
      // Note: It's possible for `onReady` to fire after `stop()` is called on the language client. :(
      if (this.status !== ServerStatus.ERROR) {
        // Language client started successfully.
        this.updateStatus(ServerStatus.RUNNING);
      }

      const caps: any = this.languageClient.initializeResult?.capabilities;
      if (caps.sorbetShowSymbolProvider) {
        this.subscriptions.push(
          commands.registerCommand("sorbet.copySymbolToClipboard", async () => {
            const editor = window.activeTextEditor;
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
            const response: SymbolInformation = await this.languageClient.sendRequest(
              "sorbet/showSymbol",
              params,
            );

            await env.clipboard.writeText(response.name);

            this.context.log.debug(
              `Copied symbol name to the clipboard. Name:${response.name}`,
            );
          }),
        );
      }

      // Unfortunately, we need this command as a wrapper around `editor.action.rename`,
      // because VSCode doesn't allow calling it from the JSON RPC
      // https://github.com/microsoft/vscode/issues/146767
      this.subscriptions.push(
        commands.registerCommand(
          "sorbet.rename",
          (params: TextDocumentPositionParams) => {
            try {
              commands.executeCommand("editor.action.rename", [
                Uri.parse(params.textDocument.uri),
                new Position(params.position.line, params.position.character),
              ]);
            } catch (error) {
              this.context.log.error(
                `Failed to rename symbol at ${params.textDocument.uri}:${params.position.line}:${params.position.character}`,
                error instanceof Error ? error : undefined,
              );
            }
          },
        ),
      );
    });
    this.subscriptions.push(this.languageClient.start());
  }

  /**
   * Implements the disposable interface so this object can be added to the context's subscriptions
   * to keep it alive. Stops the language server and Sorbet processes, and removes UI items.
   */
  public dispose() {
    this.subscriptions.forEach((s) => s.dispose());
    this.subscriptions = [];

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
      this.context.metrics.emitCountMetric("stop.timed_out", 1);
      stopProcess(this.sorbetProcess, this.context.log);
      this.sorbetProcess = null;
    }, 5000);

    this.languageClient.stop().then(() => {
      if (!stopped) {
        clearTimeout(stopTimer);
        this.context.metrics.emitCountMetric("stop.success", 1);
        this.context.log.info("Sorbet has stopped.");
      }
    });
  }

  /**
   * Updates the language client's server status. Verifies that the transition is legal.
   */
  private updateStatus(newStatus: ServerStatus) {
    if (this.status === newStatus) {
      return;
    }
    this.assertValid(this.status, newStatus);
    this.wrappedStatus = newStatus;
    this.onStatusChange(newStatus);
  }

  private assertValid(from: ServerStatus, to: ServerStatus) {
    const set = VALID_STATE_TRANSITIONS.get(from);
    if (!set || !set.has(to)) {
      this.context.log.error(
        `Invalid Sorbet server transition: ${from} => ${to}`,
      );
    }
  }

  /**
   * Runs a Sorbet process using the current active configuration. Debounced so that it runs Sorbet at most every 3 seconds.
   */
  private startSorbetProcess(): Promise<ChildProcess> {
    this.updateStatus(ServerStatus.INITIALIZING);
    this.context.log.info("Running Sorbet LSP.");
    const [command, ...args] =
      this.context.configuration.activeLspConfig?.command ?? [];
    this.context.log.debug(` > ${command} ${args.join(" ")}`);
    this.sorbetProcess = spawn(command, args, {
      cwd: workspace.rootPath,
    });
    // N.B.: 'exit' is sometimes not invoked if the process exits with an error/fails to start, as per the Node.js docs.
    // So, we need to handle both events. ¯\_(ツ)_/¯
    this.sorbetProcess.on(
      "exit",
      (code: number | null, _signal: string | null) => {
        this.sorbetProcessExitCode = code;
      },
    );
    this.sorbetProcess.on("error", (err?: NodeJS.ErrnoException) => {
      if (
        err &&
        this.status === ServerStatus.INITIALIZING &&
        err.code === "ENOENT"
      ) {
        this.context.metrics.emitCountMetric("error.enoent", 1);
        // We failed to start the process. The path to Sorbet is likely incorrect.
        this.wrappedLastError = `Could not start Sorbet with command: '${command} ${args.join(
          " ",
        )}'. Encountered error '${
          err.message
        }'. Is the path to Sorbet correct?`;
        this.updateStatus(ServerStatus.ERROR);
      }
      this.sorbetProcess = null;
      this.sorbetProcessExitCode = err?.errno ?? null;
    });
    return Promise.resolve(this.sorbetProcess);
  }

  /** ErrorHandler interface */

  /**
   * LanguageClient has built-in restart capabilities but if it's broken:
   * * It drops all `onNotification` subscriptions after restarting, so we'll miss ShowNotification updates.
   * * It drops all `onReady` subscriptions after restarting, so we won't know when the Sorbet server is running.
   * * It doesn't reset `onReady` state, so we can't even reset our `onReady` callback.
   */
  public error(): ErrorAction {
    if (this.status !== ServerStatus.ERROR) {
      this.updateStatus(ServerStatus.RESTARTING);
      this.restart(RestartReason.CRASH_LC_ERROR);
    }
    return ErrorAction.Shutdown;
  }

  /**
   * Note: If the VPN is disconnected, then Sorbet will repeatedly fail to start.
   */
  public closed(): CloseAction {
    if (this.status !== ServerStatus.ERROR) {
      this.updateStatus(ServerStatus.RESTARTING);
      let reason = RestartReason.CRASH_LC_CLOSED;
      if (this.sorbetProcessExitCode === 11) {
        // 11 number chosen somewhat arbitrarily. Most important is that this doesn't
        // clobber the exit code of Sorbet itself (which means Sorbet cannot return 11).
        //
        // The only thing that matters is that this value is kept in sync with any
        // wrapper scripts that people use with Sorbet. If this number has to
        // change for some reason, we should announce that.
        reason = RestartReason.WRAPPER_REFUSED_SPAWN;
      } else if (this.sorbetProcessExitCode === 143) {
        // 143 = 128 + 15 and 15 is TERM signal
        reason = RestartReason.FORCIBLY_TERMINATED;
      }
      this.restart(reason);
    }
    return CloseAction.DoNotRestart;
  }
}
