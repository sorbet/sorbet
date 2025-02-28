import { ChildProcess, spawn } from "child_process";
import { Disposable, Event, EventEmitter, workspace } from "vscode";
import {
  CloseAction,
  CloseHandlerResult,
  ErrorAction,
  ErrorHandler,
  ErrorHandlerResult,
  GenericNotificationHandler,
  LanguageClient,
  RevealOutputChannelOn,
  ServerCapabilities,
  ServerOptions,
} from "vscode-languageclient/node";

import { stopProcess } from "./connections";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { ServerStatus, RestartReason } from "./types";
import { backwardsCompatibleTrackUntyped } from "./config";
import { instrumentLanguageClient } from "./languageClient.metrics";

const VALID_STATE_TRANSITIONS: ReadonlyMap<
  ServerStatus,
  ReadonlySet<ServerStatus>
> = new Map<ServerStatus, Set<ServerStatus>>([
  [ServerStatus.DISABLED, new Set([ServerStatus.INITIALIZING])],
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
 * Create Sorbet Language Client.
 */
function createClient(
  context: SorbetExtensionContext,
  serverOptions: ServerOptions,
  errorHandler: ErrorHandler,
) {
  const initializationOptions = {
    // Opt in to sorbet/showOperation notifications.
    supportsOperationNotifications: true,
    // Let Sorbet know that we can handle sorbet:// URIs for generated files.
    supportsSorbetURIs: true,
    highlightUntyped: backwardsCompatibleTrackUntyped(
      context.log,
      context.configuration.highlightUntyped,
    ),
    enableTypedFalseCompletionNudges:
      context.configuration.typedFalseCompletionNudges,
  };

  context.log.debug(
    `Initializing with initializationOptions=${JSON.stringify(
      initializationOptions,
    )}`,
  );

  const client = new LanguageClient("ruby", "Sorbet", serverOptions, {
    documentSelector: [
      { language: "ruby", scheme: "file" },
      // Support queries on generated files with sorbet:// URIs that do not exist editor-side.
      { language: "ruby", scheme: "sorbet" },
    ],
    outputChannel: context.logOutputChannel,
    initializationOptions,
    initializationFailedHandler: (_error) => {
      return true;
    },
    errorHandler,
    revealOutputChannelOn: context.configuration.revealOutputOnError
      ? RevealOutputChannelOn.Error
      : RevealOutputChannelOn.Never,
  });

  return client;
}

export type SorbetServerCapabilities = ServerCapabilities & {
  sorbetShowSymbolProvider: boolean;
};

export class SorbetLanguageClient implements Disposable, ErrorHandler {
  private readonly context: SorbetExtensionContext;
  private readonly languageClient: LanguageClient;
  private readonly onStatusChangeEmitter: EventEmitter<ServerStatus>;
  private readonly restart: (reason: RestartReason) => void;

  private sorbetProcess?: ChildProcess;
  // Sometimes this is an errno, not a process exit code. This happens when set
  // via the `.on("error")` handler, instead of the `.on("exit")` handler.
  private sorbetProcessExitCode?: number;
  private wrappedLastError?: string;
  private wrappedStatus: ServerStatus;

  constructor(
    context: SorbetExtensionContext,
    restart: (reason: RestartReason) => void,
  ) {
    this.context = context;
    this.languageClient = instrumentLanguageClient(
      createClient(context, () => this.startSorbetProcess(), this),
      this.context.metrics,
    );
    this.onStatusChangeEmitter = new EventEmitter<ServerStatus>();
    this.restart = restart;
    this.wrappedStatus = ServerStatus.DISABLED;
  }

  public dispose(): void {
    this.onStatusChangeEmitter.dispose();
    // No awaiting here as we don't want to block the dispose chain
    this.stop();
  }

  /**
   * Sorbet language server {@link SorbetServerCapabilities capabilities}. Only
   * available when the server has been initialized.
   */
  public get capabilities(): SorbetServerCapabilities | undefined {
    return <SorbetServerCapabilities | undefined>(
      this.languageClient.initializeResult?.capabilities
    );
  }

  /**
   * Last error message when {@link status} is {@link ServerStatus.ERROR}.
   */
  public get lastError(): string | undefined {
    return this.wrappedLastError;
  }

  /**
   * Resolves when client is ready to serve requests.
   */
  public async start(): Promise<void> {
    if (!this.languageClient.needsStart()) {
      this.context.log.trace("Ignored unnecessary start request");
      return;
    }

    // const x = await this.startSorbetProcess();
    // createClient(this.context, () => Promise.resolve(x), this);

    // TODO(damolina): DO NOT let double calls
    // TODO(damolina): It's possible for `onReady` to fire after `stop()` is called on the language client. :(
    try {
      await this.languageClient.start();
      this.status = ServerStatus.RUNNING;
    } catch (e) {
      this.context.log.error(`Error starting language client: ${e}`);
      this.status = ServerStatus.ERROR;
    }
  }

  /**
   * Resolves when client is ready to serve requests.
   */
  public async stop(): Promise<void> {
    if (!this.languageClient.needsStop()) {
      this.context.log.trace("Ignored unnecessary stop request");
      return;
    }

    let stopped = false;

    //  stop() only resolves after the language server ACKs the stop request.
    //  Stopping can time out if the language client is repeatedly failing to
    //  start (e.g. if network is down, or path to Sorbet is incorrect), or if
    //  Sorbet never ACKs the stop request.
    //  In the former case (which is the common case), VS code stops retrying
    //  the connection after we call stop(), but never invokes our callback.
    //  Thus, our solution is to wait 5 seconds for a callback, and stop the
    //  process if we haven't heard back.

    const stopTimer = setTimeout(() => {
      stopped = true;
      this.context.metrics.emitCountMetric("stop.timed_out", 1);
      if (this.sorbetProcess?.pid) {
        stopProcess(this.sorbetProcess, this.context.log);
      }
      this.sorbetProcess = undefined;
    }, 5000);

    await this.languageClient.stop();
    this.status = ServerStatus.DISABLED;

    if (!stopped) {
      clearTimeout(stopTimer);
      this.context.metrics.emitCountMetric("stop.success", 1);
      this.context.log.info("Sorbet has stopped.");
    }
  }

  /**
   * Register a handler for a language server notification. See {@link LanguageClient.onNotification}.
   */
  public onNotification(
    method: string,
    handler: GenericNotificationHandler,
  ): Disposable {
    return this.languageClient.onNotification(method, handler);
  }

  /**
   * Event fired on {@link status} changes.
   */
  public get onStatusChange(): Event<ServerStatus> {
    return this.onStatusChangeEmitter.event;
  }

  /**
   * Send a request to the language server. See {@link LanguageClient.sendRequest}.
   */
  public sendRequest<TResponse>(
    method: string,
    param: any,
  ): Promise<TResponse> {
    return this.languageClient.sendRequest<TResponse>(method, param);
  }

  /**
   * Send a notification to the language server. See {@link LanguageClient.sendNotification}.
   */
  public sendNotification(method: string, param: any): Promise<void> {
    return this.languageClient.sendNotification(method, param);
  }

  /**
   * Language client status.
   */
  public get status(): ServerStatus {
    return this.wrappedStatus;
  }

  private set status(newStatus: ServerStatus) {
    if (this.status === newStatus) {
      return;
    }

    const set = VALID_STATE_TRANSITIONS.get(this.status);
    if (!set?.has(newStatus)) {
      this.context.log.error(
        `Invalid Sorbet server transition: ${this.status} => ${newStatus}}`,
      );
    }

    this.wrappedStatus = newStatus;
    this.onStatusChangeEmitter.fire(newStatus);
  }

  /**
   * Runs a Sorbet process using the current active configuration. Debounced so that it runs Sorbet at most every 3 seconds.
   */
  private startSorbetProcess(): Promise<ChildProcess> {
    return new Promise<ChildProcess>((resolve, reject) => {
      this.context.log.info("Starting Sorbet LSP …");
      this.status = ServerStatus.INITIALIZING;

      const activeConfig = this.context.configuration.activeLspConfig;
      if (!activeConfig) {
        const msg = "No active configuration";
        this.context.log.error(msg);
        reject(new Error(msg));
        return;
      }

      const [command, ...args] = activeConfig.command;
      if (!command) {
        const msg = `Missing command configuration. Config:${activeConfig.id}`;
        this.context.log.error(msg);
        reject(new Error(msg));
        return;
      }

      const cwd = workspace.workspaceFolders?.[0]?.uri.fsPath;
      if (!cwd) {
        this.context.log.warning("No workspace folder");
      } else if (workspace.workspaceFolders!.length > 1) {
        this.context.log.warning(
          `Multi-root workspaces are not supported, using first workspace folder: ${cwd}`,
        );
      }

      this.context.log.debug(` > ${activeConfig.command.join(" ")}`);
      this.sorbetProcess = spawn(command, args, {
        cwd,
        env: { ...process.env, ...activeConfig.env },
      })
        .on("exit", (code: number | null, _signal: string | null) => {
          this.sorbetProcess = undefined;
          this.sorbetProcessExitCode = code ?? undefined;
        })
        .on("error", (err: NodeJS.ErrnoException) => {
          if (
            err.code === "ENOENT" &&
            this.status === ServerStatus.INITIALIZING
          ) {
            this.context.log.error(
              `Failed to start: ${command}\nError: ${err.message}`,
            );
            this.context.metrics.emitCountMetric("error.enoent", 1);
            this.status = ServerStatus.ERROR;
          }

          this.sorbetProcess = undefined;
          this.sorbetProcessExitCode = err?.errno;
        });

      return resolve(this.sorbetProcess);
    });
  }

  /** ErrorHandler interface */

  /**
   * LanguageClient has built-in restart capabilities but if it's broken:
   * * It drops all `onNotification` subscriptions after restarting, so we'll miss ShowNotification updates.
   * * It drops all `onReady` subscriptions after restarting, so we won't know when the Sorbet server is running.
   * * It doesn't reset `onReady` state, so we can't even reset our `onReady` callback.
   */
  public error(): ErrorHandlerResult {
    if (this.status !== ServerStatus.ERROR) {
      this.status = ServerStatus.RESTARTING;
      this.restart(RestartReason.CRASH_LC_ERROR);
    }

    return {
      action: ErrorAction.Shutdown,
      // TODO: message ?
    };
  }

  /**
   * Note: If the VPN is disconnected, then Sorbet will repeatedly fail to start.
   */
  public closed(): CloseHandlerResult {
    if (this.status !== ServerStatus.ERROR) {
      let reason: RestartReason;
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
      } else {
        reason = RestartReason.CRASH_LC_CLOSED;
        this.context.log.error("");
        this.context.log.error(
          `The Sorbet LSP process crashed exit_code=${this.sorbetProcessExitCode}`,
        );
        this.context.log.error("The Node.js backtrace above is not useful.");
        this.context.log.error(
          "If there is a C++ backtrace above, that is useful.",
        );
        this.context.log.error(
          "Otherwise, more useful output will be in the --debug-log-file to the Sorbet process",
        );
        this.context.log.error("(if provided as a command-line argument).");
        this.context.log.error("");
      }

      this.status = ServerStatus.RESTARTING;
      this.restart(reason);
    }

    return {
      action: CloseAction.DoNotRestart,
      // TODO: message ?
    };
  }
}
