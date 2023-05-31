import { Disposable, Event, EventEmitter } from "vscode";
import SorbetLanguageClient from "./LanguageClient";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { RestartReason, ServerStatus, ShowOperationParams } from "./types";

const MIN_TIME_BETWEEN_RETRIES_MS = 7000;

export type StatusChangedEvent = {
  status: ServerStatus;
  stopped?: true;
  error?: string;
};

export class SorbetStatusProvider implements Disposable {
  private _activeSorbetLanguageClient?: SorbetLanguageClient;
  private readonly _context: SorbetExtensionContext;
  private readonly _disposables: Disposable[];
  /** Mutex for startSorbet. Prevents us from starting multiple processes at once. */
  private _isStarting: boolean;
  private _lastSorbetRetryTime: number;
  private readonly _onShowOperationEmitter: EventEmitter<ShowOperationParams>;
  private readonly _onStatusChangedEmitter: EventEmitter<StatusChangedEvent>;

  constructor(context: SorbetExtensionContext) {
    this._context = context;
    this._isStarting = false;
    this._lastSorbetRetryTime = 0;
    this._onShowOperationEmitter = new EventEmitter<ShowOperationParams>();
    this._onStatusChangedEmitter = new EventEmitter<StatusChangedEvent>();

    this._disposables = [
      this._onShowOperationEmitter,
      this._onStatusChangedEmitter,
    ];
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    Disposable.from(...this._disposables).dispose();
  }

  /**
   * Current Sorbet client.
   */
  public get activeSorbetLanguageClient(): SorbetLanguageClient | undefined {
    return this._activeSorbetLanguageClient;
  }

  private set activeSorbetLanguageClient(
    value: SorbetLanguageClient | undefined,
  ) {
    if (this._activeSorbetLanguageClient === value) {
      return;
    }

    // Clean-up existing client, if any.
    if (this._activeSorbetLanguageClient) {
      this._activeSorbetLanguageClient.dispose();
      const i = this._disposables.indexOf(this._activeSorbetLanguageClient);
      if (i !== -1) {
        this._disposables.splice(i, 1);
      }
    }

    // Hook-up new client for clean-up, if any.
    if (value) {
      const i = this._disposables.indexOf(value);
      if (i === -1) {
        this._disposables.push(value);
      }
    }

    this._activeSorbetLanguageClient = value;
  }

  /**
   * Event raised on a {@link ShowOperationParams show-operation} event.
   */
  public get onShowOperation(): Event<ShowOperationParams> {
    return this._onShowOperationEmitter.event;
  }

  /**
   * Event raised on {@link ServerStatus status} changes.
   */
  public get onStatusChanged(): Event<StatusChangedEvent> {
    return this._onStatusChangedEmitter.event;
  }

  /**
   * Restart Sorbet.
   * @param reason Telemetry reason.
   */
  public async restartSorbet(reason: RestartReason): Promise<void> {
    await this.stopSorbet(ServerStatus.RESTARTING);
    // `reason` is an enum type with a small and finite number of values.
    this._context.emitCountMetric(`restart.${reason}`, 1);
    await this.startSorbet();
  }

  /**
   * Start Sorbet.
   */
  public async startSorbet() {
    if (this._isStarting) {
      return;
    }

    // Debounce by MIN_TIME_BETWEEN_RETRIES_MS. Returns 0 if the calculated time to sleep is negative.
    const sleepMS =
      MIN_TIME_BETWEEN_RETRIES_MS - (Date.now() - this._lastSorbetRetryTime);
    if (sleepMS > 0) {
      // Wait timeToSleep ms. Use mutex, as this yields the event loop for future events.
      console.log(`Waiting ${sleepMS.toFixed(0)}ms before restarting Sorbet…`);
      this._isStarting = true;
      await new Promise((res) => setTimeout(res, sleepMS));
      this._isStarting = false;
    }
    this._lastSorbetRetryTime = Date.now();

    // Create client. Wait for `ready` before accessing it.
    const newClient = new SorbetLanguageClient(
      this._context,
      (reason: RestartReason) => this.restartSorbet(reason),
    );
    await newClient.languageClient.onReady();

    // Use property-setter to ensure proper clean-up.
    this.activeSorbetLanguageClient = newClient;

    newClient.languageClient.onNotification(
      "sorbet/showOperation",
      (params: ShowOperationParams) => {
        // Ignore event if this is not the current client (e.g. client is being shut down).
        if (this.activeSorbetLanguageClient === newClient) {
          this._onShowOperationEmitter.fire(params);
        }
      },
    );
    newClient.onStatusChange = (status: ServerStatus) => {
      // Ignore event if this is not the current client (e.g. client is being shut down).
      if (this.activeSorbetLanguageClient !== newClient) {
        this._onStatusChangedEmitter.fire({
          status,
          error: newClient.lastError,
        });
      }
    };

    this._onStatusChangedEmitter.fire({
      status: newClient.status,
      error: newClient.lastError,
    });
  }

  /**
   * Stop Sorbet.
   * @param newStatus Status to report.
   */
  public async stopSorbet(newStatus: ServerStatus): Promise<void> {
    // Use property-setter to ensure proper clean-up.
    this.activeSorbetLanguageClient = undefined;
    this._onStatusChangedEmitter.fire({ status: newStatus, stopped: true });
  }
}
