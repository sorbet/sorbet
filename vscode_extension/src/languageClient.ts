import { ChildProcess, spawn } from "child_process";
import {
  CancellationToken,
  Disposable,
  Event,
  EventEmitter,
  workspace,
} from "vscode";
import {
  CloseAction,
  ErrorAction,
  ErrorHandler,
  GenericNotificationHandler,
  LanguageClient,
  RevealOutputChannelOn,
  ServerCapabilities,
  ServerOptions,
} from "vscode-languageclient/node";

import { stopProcess } from "./connections";
import { Tags } from "./metricsClient";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { ServerStatus, RestartReason } from "./types";

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
  const client = new LanguageClient("ruby", "Sorbet", serverOptions, {
    documentSelector: [
      { language: "ruby", scheme: "file" },
      // Support queries on generated files with sorbet:// URIs that do not exist editor-side.
      { language: "ruby", scheme: "sorbet" },
    ],
    outputChannel: context.logOutputChannel,
    initializationOptions: {
      // Opt in to sorbet/showOperation notifications.
      supportsOperationNotifications: true,
      // Let Sorbet know that we can handle sorbet:// URIs for generated files.
      supportsSorbetURIs: true,
      highlightUntyped: context.configuration.highlightUntyped,
    },
    errorHandler,
    revealOutputChannelOn: context.configuration.revealOutputOnError
      ? RevealOutputChannelOn.Error
      : RevealOutputChannelOn.Never,
  });

  return client;
}

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
  return client;
}

export type SorbetServerCapabilities = ServerCapabilities & {
  sorbetShowSymbolProvider: boolean;
};

export class SorbetLanguageClient implements Disposable, ErrorHandler {
  private readonly context: SorbetExtensionContext;
  private readonly disposables: Disposable[];
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
    this.onStatusChangeEmitter = new EventEmitter<ServerStatus>();
    this.restart = restart;
    this.wrappedStatus = ServerStatus.INITIALIZING;

    this.languageClient = shimLanguageClient(
      createClient(context, () => this.startSorbetProcess(), this),
      (metric, value, tags) =>
        this.context.metrics.emitTimingMetric(metric, value, tags),
    );
    // It's possible for `onReady` to fire after `stop()` is called on the language client. :(
    this.languageClient.onReady().then(() => {
      if (this.status !== ServerStatus.ERROR) {
        // Language client started successfully.
        this.status = ServerStatus.RUNNING;
      }
    });

    this.disposables = [
      this.languageClient.start(),
      this.onStatusChangeEmitter,
    ];
  }

  /**
   * Implements the disposable interface so this object can be added to the context's subscriptions
   * to keep it alive. Stops the language server and Sorbet processes, and removes UI items.
   */
  public dispose() {
    Disposable.from(...this.disposables).dispose();
    this.disposables.length = 0;

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
      if (this.sorbetProcess?.pid) {
        stopProcess(this.sorbetProcess, this.context.log);
      }
      this.sorbetProcess = undefined;
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
   * Sorbet language server {@link SorbetServerCapabilities capabilities}. Only
   * available if the server has been initialized.
   */
  public get capabilities(): SorbetServerCapabilities | undefined {
    return <SorbetServerCapabilities | undefined>(
      this.languageClient.initializeResult?.capabilities
    );
  }

  /**
   * Contains error message when {@link status} is {@link ServerStatus.ERROR}.
   */
  public get lastError(): string | undefined {
    return this.wrappedLastError;
  }

  /**
   * Resolves when client is ready to serve requests.
   */
  public onReady(): Promise<void> {
    return this.languageClient.onReady();
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
    token?: CancellationToken,
  ): Promise<TResponse> {
    // Do not pass `token` if undefined, otherwise `param` ends up being passed
    // as `[...param, undefined]` instead of `param`.
    return token
      ? this.languageClient.sendRequest<TResponse>(method, param, token)
      : this.languageClient.sendRequest<TResponse>(method, param);
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
    this.status = ServerStatus.INITIALIZING;
    this.context.log.info("Running Sorbet LSP.");
    const [command, ...args] =
      this.context.configuration.activeLspConfig?.command ?? [];
    if (!command) {
      const msg = `Missing command-line data to start Sorbet. ConfigId:${this.context.configuration.activeLspConfig?.id}`;
      this.context.log.error(msg);
      return Promise.reject(new Error(msg));
    }

    this.context.log.debug(` > ${command} ${args.join(" ")}`);
    this.sorbetProcess = spawn(command, args, {
      cwd: workspace.rootPath,
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

  /**
   * LanguageClient has built-in restart capabilities but if it's broken:
   * * It drops all `onNotification` subscriptions after restarting, so we'll miss ShowNotification updates.
   * * It drops all `onReady` subscriptions after restarting, so we won't know when the Sorbet server is running.
   * * It doesn't reset `onReady` state, so we can't even reset our `onReady` callback.
   */
  public error(): ErrorAction {
    if (this.status !== ServerStatus.ERROR) {
      this.status = ServerStatus.RESTARTING;
      this.restart(RestartReason.CRASH_LC_ERROR);
    }
    return ErrorAction.Shutdown;
  }

  /**
   * Note: If the VPN is disconnected, then Sorbet will repeatedly fail to start.
   */
  public closed(): CloseAction {
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
      }

      this.status = ServerStatus.RESTARTING;
      this.restart(reason);
    }

    return CloseAction.DoNotRestart;
  }
}
