import { Disposable, Event, EventEmitter } from "vscode";
import { SorbetLanguageClient } from "./languageClient";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { RestartReason, ServerStatus, ShowOperationParams } from "./types";

const MIN_TIME_BETWEEN_RETRIES_MS = 7000;

export type StatusChangedEvent = {
  status: ServerStatus;
  stopped?: true;
  error?: string;
};

export class SorbetStatusProvider implements Disposable {
  private wrappedActiveLanguageClient?: SorbetLanguageClient;
  private readonly context: SorbetExtensionContext;
  private readonly disposables: Disposable[];
  /** Mutex for startSorbet. Prevents us from starting multiple processes at once. */
  private isStarting: boolean;
  private lastSorbetRetryTime: number;
  private readonly onShowOperationEmitter: EventEmitter<ShowOperationParams>;
  private readonly onStatusChangedEmitter: EventEmitter<StatusChangedEvent>;

  constructor(context: SorbetExtensionContext) {
    this.context = context;
    this.isStarting = false;
    this.lastSorbetRetryTime = 0;
    this.onShowOperationEmitter = new EventEmitter<ShowOperationParams>();
    this.onStatusChangedEmitter = new EventEmitter<StatusChangedEvent>();

    this.disposables = [
      this.onShowOperationEmitter,
      this.onStatusChangedEmitter,
    ];
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    Disposable.from(...this.disposables).dispose();
  }

  /**
   * Current Sorbet client, if any.
   */
  public get activeLanguageClient(): SorbetLanguageClient | undefined {
    return this.wrappedActiveLanguageClient;
  }

  private set activeLanguageClient(value: SorbetLanguageClient | undefined) {
    if (this.wrappedActiveLanguageClient === value) {
      return;
    }

    // Clean-up existing client, if any.
    if (this.wrappedActiveLanguageClient) {
      this.wrappedActiveLanguageClient.dispose();
      const i = this.disposables.indexOf(this.wrappedActiveLanguageClient);
      if (i !== -1) {
        this.disposables.splice(i, 1);
      }
    }

    // Hook-up new client for clean-up, if any.
    if (value) {
      const i = this.disposables.indexOf(value);
      if (i === -1) {
        this.disposables.push(value);
      }
    }

    this.wrappedActiveLanguageClient = value;

    // State might have changed based on new client.
    if (this.wrappedActiveLanguageClient) {
      this.onStatusChangedEmitter.fire({
        status: this.wrappedActiveLanguageClient.status,
        error: this.wrappedActiveLanguageClient.lastError,
      });
    }
  }

  /**
   * Event raised on a {@link ShowOperationParams show-operation} event.
   */
  public get onShowOperation(): Event<ShowOperationParams> {
    return this.onShowOperationEmitter.event;
  }

  /**
   * Event raised on {@link ServerStatus status} changes.
   */
  public get onStatusChanged(): Event<StatusChangedEvent> {
    return this.onStatusChangedEmitter.event;
  }

  /**
   * Restart Sorbet.
   * @param reason Telemetry reason.
   */
  public async restartSorbet(reason: RestartReason): Promise<void> {
    await this.stopSorbet(ServerStatus.RESTARTING);
    // `reason` is an enum type with a small and finite number of values.
    this.context.metrics.emitCountMetric(`restart.${reason}`, 1);
    await this.startSorbet();
  }

  /**
   * Error information, if {@link serverStatus} is {@link ServerStatus.ERROR}
   */
  public get serverError(): string | undefined {
    return this.activeLanguageClient?.lastError;
  }

  /**
   * Return current {@link ServerStatus server status}.
   */
  public get serverStatus(): ServerStatus {
    return this.activeLanguageClient?.status || ServerStatus.DISABLED;
  }

  /**
   * Start Sorbet.
   */
  public async startSorbet(): Promise<void> {
    if (this.isStarting) {
      return;
    }

    // Debounce by MIN_TIME_BETWEEN_RETRIES_MS. Returns 0 if the calculated time to sleep is negative.
    const sleepMS =
      MIN_TIME_BETWEEN_RETRIES_MS - (Date.now() - this.lastSorbetRetryTime);
    if (sleepMS > 0) {
      // Wait timeToSleep ms. Use mutex, as this yields the event loop for future events.
      this.context.log.debug(
        `Waiting ${sleepMS.toFixed(0)}ms before restarting Sorbet…`,
      );
      this.isStarting = true;
      await new Promise((res) => setTimeout(res, sleepMS));
      this.isStarting = false;
    }
    this.lastSorbetRetryTime = Date.now();

    // Create client
    const newClient = new SorbetLanguageClient(
      this.context,
      (reason: RestartReason) => this.restartSorbet(reason),
    );
    // Use property-setter to ensure proper setup.
    this.activeLanguageClient = newClient;

    newClient.onStatusChange = (status: ServerStatus) => {
      // Ignore event if this is not the current client (e.g. old client being shut down).
      if (this.activeLanguageClient === newClient) {
        this.onStatusChangedEmitter.fire({
          status,
          error: newClient.lastError,
        });
      }
    };

    // Wait for `ready` before accessing `languageClient`.
    await newClient.languageClient.onReady();
    newClient.languageClient.onNotification(
      "sorbet/showOperation",
      (params: ShowOperationParams) => {
        // Ignore event if this is not the current client (e.g. old client being shut down).
        if (this.activeLanguageClient === newClient) {
          this.onShowOperationEmitter.fire(params);
        }
      },
    );
  }

  /**
   * Stop Sorbet.
   * @param newStatus Status to report.
   */
  public async stopSorbet(newStatus: ServerStatus): Promise<void> {
    // Use property-setter to ensure proper clean-up.
    this.activeLanguageClient = undefined;
    this.onStatusChangedEmitter.fire({ status: newStatus, stopped: true });
  }
}
