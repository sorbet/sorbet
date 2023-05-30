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

  public get activeSorbetLanguageClient(): SorbetLanguageClient | undefined {
    return this._activeSorbetLanguageClient;
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
    const timeToSleep = Math.max(
      0,
      MIN_TIME_BETWEEN_RETRIES_MS - (Date.now() - this._lastSorbetRetryTime),
    );
    if (timeToSleep > 0) {
      // Wait timeToSleep ms. Use mutex, as this yields the event loop for future events.
      console.log(
        `Waiting ${timeToSleep.toFixed(0)}ms before restarting Sorbet…`,
      );
      this._isStarting = true;
      await new Promise((res) => setTimeout(res, timeToSleep));
      this._isStarting = false;
    }

    this._lastSorbetRetryTime = Date.now();
    const sorbet = new SorbetLanguageClient(
      this._context,
      filterUpdatesFromOldClients(
        this._activeSorbetLanguageClient,
        this.restartSorbet,
      ),
    );
    this._activeSorbetLanguageClient = sorbet;
    this._disposables.push(this._activeSorbetLanguageClient);

    this._onStatusChangedEmitter.fire({
      status: sorbet.status,
      error: sorbet.lastError,
    });

    sorbet.onStatusChange = filterUpdatesFromOldClients(
      this._activeSorbetLanguageClient,
      (status: ServerStatus) => {
        this._onStatusChangedEmitter.fire({
          status,
          error: sorbet.lastError,
        });
      },
    );

    sorbet.languageClient.onReady().then(
      filterUpdatesFromOldClients(this._activeSorbetLanguageClient, () => {
        sorbet.languageClient.onNotification("sorbet/showOperation", () => {
          filterUpdatesFromOldClients(
            this._activeSorbetLanguageClient,
            (params: ShowOperationParams) =>
              this._onShowOperationEmitter.fire(params),
          );
        });
      }),
    );

    // Helper function. Drops any status updates and operations from old clients
    // that are in the process of shutting down.
    function filterUpdatesFromOldClients<TS extends any[]>(
      knownClient: SorbetLanguageClient | undefined,
      fn: (...args: TS) => void,
    ): (...args: TS) => void {
      return (...args: TS): void => {
        if (knownClient !== sorbet) {
          return;
        }
        return fn(...args);
      };
    }
  }

  /**
   * Stop Sorbet.
   * @param newStatus Status to report.
   */
  public async stopSorbet(newStatus: ServerStatus): Promise<void> {
    if (this._activeSorbetLanguageClient) {
      this._activeSorbetLanguageClient.dispose();
      // Garbage collect the language client.
      const i = this._disposables.indexOf(this._activeSorbetLanguageClient);
      if (i !== -1) {
        this._disposables.splice(i, 1);
      }
      this._activeSorbetLanguageClient = undefined;
    }
    this._onStatusChangedEmitter.fire({ status: newStatus, stopped: true });
  }
}
