import {
  Disposable,
  Event,
  EventEmitter,
  workspace,
  WorkspaceFolder,
} from "vscode";
import {
  CloseAction,
  CloseHandlerResult,
  ErrorAction,
  ErrorHandler,
  ErrorHandlerResult,
  GenericNotificationHandler,
  LanguageClient,
  ServerCapabilities,
} from "vscode-languageclient/node";
import { ChildProcess, spawn } from "child_process";
import { stopProcess } from "./connections";
import { createClient } from "./languageClient";
import { instrumentLanguageClient } from "./languageClient.metrics";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { ServerStatus, RestartReason } from "./types";

const STOP_TIMEOUT_MS = 5000;

const VALID_STATE_TRANSITIONS: ReadonlyMap<
  ServerStatus,
  ReadonlySet<ServerStatus>
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
  [ServerStatus.DISABLED, new Set([ServerStatus.INITIALIZING])],
  // Restarting is a terminal state. The restart occurs by terminating this LanguageClient and creating a new one.
  [ServerStatus.RESTARTING, new Set()],
  // Error is a terminal state for this class.
  [ServerStatus.ERROR, new Set()],
]);

export type SorbetServerCapabilities = ServerCapabilities & {
  sorbetShowSymbolProvider: boolean;
};

export class SorbetLanguageClient implements Disposable, ErrorHandler {
  private readonly context: SorbetExtensionContext;
  private readonly languageClient: LanguageClient;
  private readonly onStatusChangeEmitter: EventEmitter<ServerStatus>;
  private readonly restart: (reason: RestartReason) => void;
  private sorbetProcess?: ChildProcess;
  private stopPromise?: Promise<void>;
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
      createClient(
        context,
        this.workspaceFolder,
        () => this.startSorbetProcess(),
        this,
      ),
      this.context.metrics,
    );

    this.onStatusChangeEmitter = new EventEmitter<ServerStatus>();
    this.restart = restart;
    this.wrappedStatus = ServerStatus.INITIALIZING;
  }

  private get workspaceFolder(): WorkspaceFolder | undefined {
    return workspace.workspaceFolders?.[0];
  }

  /**
   * Implements the disposable interface so this object can be added to the context's subscriptions
   * to keep it alive. Stops the language server and Sorbet processes, and removes UI items.
   */
  public dispose() {
    this.stop().catch((error: Error) => {
      this.context.log.error(
        "Failed to stop Sorbet client during dispose.",
        error,
      );
    });
  }

  public async stop(): Promise<void> {
    if (this.stopPromise) {
      return this.stopPromise;
    }

    this.onStatusChangeEmitter.dispose();

    const { sorbetProcess } = this;

    this.stopPromise = new Promise<void>((resolve) => {
      let finished = false;
      const finish = () => {
        if (!finished) {
          finished = true;
          this.sorbetProcess = undefined;
          resolve();
        }
      };
      /*
       * languageClient.stop() only invokes the then() callback after the language
       * server ACKs the stop request.
       * Stopping can time out if the language client is repeatedly failing to
       * start (e.g. if network is down, or path to Sorbet is incorrect), or if
       * Sorbet never ACKs the stop request.
       * In the former case (which is the common case), VS code stops retrying
       * the connection after we call stop(), but never invokes our callback.
       * Thus, our solution is to wait 5 seconds for a callback, and stop the
       * process if we haven't heard back.
       */
      const stopTimer = setTimeout(() => {
        this.context.metrics.emitCountMetric("stop.timed_out", 1);
        if (sorbetProcess?.pid) {
          this.context.log.warn(
            "Sorbet client stop timed out after 5s; terminating Sorbet process.",
            sorbetProcess.pid,
          );
          stopProcess(sorbetProcess, this.context.log)
            .catch((error: Error) => {
              this.context.log.error(
                "Failed to terminate Sorbet process.",
                error,
              );
            })
            .then(finish);
        } else {
          finish();
        }
      }, STOP_TIMEOUT_MS);

      this.languageClient.stop().then(
        async () => {
          clearTimeout(stopTimer);
          this.context.metrics.emitCountMetric("stop.success", 1);
          this.context.log.info("Sorbet has stopped.");
          const exited = await this.waitForProcessExit(
            sorbetProcess,
            STOP_TIMEOUT_MS,
          );
          if (!exited && sorbetProcess?.pid) {
            this.context.log.warn(
              "Sorbet acknowledged shutdown but did not exit within 5s; terminating Sorbet process.",
              sorbetProcess.pid,
            );
            await stopProcess(sorbetProcess, this.context.log);
          }
          finish();
        },
        async (error: Error) => {
          clearTimeout(stopTimer);
          this.context.log.warn("Sorbet client stop failed.", error);
          if (sorbetProcess?.pid) {
            await stopProcess(sorbetProcess, this.context.log);
          }
          finish();
        },
      );
    });

    return this.stopPromise;
  }

  private waitForProcessExit(
    sorbetProcess: ChildProcess | undefined,
    timeoutMs?: number,
  ): Promise<boolean> {
    if (
      !sorbetProcess ||
      sorbetProcess.exitCode !== null ||
      sorbetProcess.signalCode !== null
    ) {
      return Promise.resolve(true);
    }

    return new Promise<boolean>((resolve) => {
      let resolved = false;
      const timer =
        timeoutMs === undefined
          ? undefined
          : setTimeout(() => {
              if (!resolved) {
                resolved = true;
                sorbetProcess.removeListener("exit", onExit);
                sorbetProcess.removeListener("error", onExit);
                resolve(false);
              }
            }, timeoutMs);
      const onExit = () => {
        if (!resolved) {
          resolved = true;
          if (timer !== undefined) {
            clearTimeout(timer);
          }
          sorbetProcess.removeListener("exit", onExit);
          sorbetProcess.removeListener("error", onExit);
          resolve(true);
        }
      };

      sorbetProcess.on("exit", onExit);
      sorbetProcess.on("error", onExit);
    });
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
      this.context.log.debug("Ignored unnecessary start request");
      return;
    }

    this.status = ServerStatus.INITIALIZING;
    await this.languageClient.start();

    // In case of error (missing Sorbet process), the client might have already
    // transitioned to Error or Restarting so this should not override that.
    if (this.status === ServerStatus.INITIALIZING) {
      this.status = ServerStatus.RUNNING;
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
   * Send a request to language server. See {@link LanguageClient.sendRequest}.
   */
  public sendRequest<TResponse>(
    method: string,
    param: any,
  ): Promise<TResponse> {
    return this.languageClient.sendRequest<TResponse>(method, param);
  }

  /**
   * Send a notification to language server. See {@link LanguageClient.sendNotification}.
   */
  public sendNotification(method: string, param: any): void {
    this.languageClient.sendNotification(method, param);
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

    if (!VALID_STATE_TRANSITIONS.get(this.status)?.has(newStatus)) {
      this.context.log.error(
        `Invalid Sorbet server transition: ${this.status} => ${newStatus}}`,
      );
    }

    this.wrappedStatus = newStatus;
    this.onStatusChangeEmitter.fire(newStatus);
  }

  /**
   * Runs a Sorbet process using the current active configuration. Debounced so that it runs
   * Sorbet at most every MIN_TIME_BETWEEN_RETRIES_MS.
   */
  private startSorbetProcess(): Promise<ChildProcess> {
    this.context.log.info("Running Sorbet LSP.");
    const activeConfig = this.context.configuration.activeLspConfig;
    const [command, ...args] = activeConfig?.command ?? [];
    if (!command) {
      let msg: string;
      if (!activeConfig) {
        msg = "No active Sorbet configuration.";
        this.status = ServerStatus.DISABLED;
      } else {
        msg = `Missing command-line data to start Sorbet. ConfigId:${activeConfig.id}`;
      }

      this.context.log.error(msg);
      return Promise.reject(new Error(msg));
    }

    this.context.log.debug(">", command, ...args);
    this.sorbetProcess = spawn(command, args, {
      cwd: this.workspaceFolder?.uri.fsPath,
      // Spawn in a new process group on POSIX so that force-stop signals can
      // be sent to the group (covering wrapper scripts and the Sorbet binary)
      // without affecting the VS Code extension host process.
      detached: process.platform !== "win32",
      env: { ...process.env, ...activeConfig?.env },
    });
    // N.B.: 'exit' is sometimes not invoked if the process exits with an error/fails to start, as per the Node.js docs.
    // So, we need to handle both events. ¯\_(ツ)_/¯
    this.sorbetProcess.on(
      "exit",
      (code: number | null, _signal: string | null) => {
        this.sorbetProcessExitCode = code ?? undefined;
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
        this.status = ServerStatus.ERROR;
      }
      this.sorbetProcess = undefined;
      this.sorbetProcessExitCode = err?.errno;
    });
    return Promise.resolve(this.sorbetProcess);
  }

  /** ErrorHandler interface */

  public error(): ErrorHandlerResult {
    if (this.status !== ServerStatus.ERROR) {
      this.status = ServerStatus.RESTARTING;
      this.restart(RestartReason.CRASH_LC_ERROR);
    }
    return {
      action: ErrorAction.Shutdown,
      handled: true,
    };
  }

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
        this.context.log.error(
          "The Sorbet LSP process crashed exit_code",
          this.sorbetProcessExitCode,
        );
        this.context.log.error(
          "The Node.js backtrace above is not useful.",
          "If there is a C++ backtrace above, that is useful.",
          "Otherwise, more useful output will be in the --debug-log-file to the Sorbet process",
          "(if provided as a command-line argument).",
        );
      }

      this.status = ServerStatus.RESTARTING;
      this.restart(reason);
    }

    return {
      action: CloseAction.DoNotRestart,
      handled: true,
    };
  }
}
