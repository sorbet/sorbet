import { Disposable, Event, EventEmitter } from "vscode";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { ServerStatus } from "./types";
import { Log } from "./log";

/**
 * Status changes reported by extension.
 */
const enum Status {
  /**
   * Extension starts in disabled state. It then can enter this state after
   * user explicitly disables Sorbet.
   */
  Disabled = "disabled",
  /**
   * Sorbet Language Server has started up and it is ready to process.
   */
  Ready = "ready",
  /**
   * Sorbet server is being started, event might repeat in case of error or if
   * the server was shutdown.
   */
  Start = "start",
}

/**
 * Sorbet Extension API.
 *
 * IMPORTANT!
 * Maintain backward and forward compatibility in all changes to prevent breaking
 * consumer extensions:
 *  1. Use optional properties.
 *  2. NEVER expose internal types directly.
 */
export interface SorbetExtensionApi {
  status?(): Status | undefined;
  readonly onStatusChanged?: Event<Status>;
}

export class SorbetExtensionApiImpl implements Disposable {
  private readonly disposables: Disposable[];
  private readonly onStatusChangedEmitter: EventEmitter<Status>;
  private readonly log: Log;
  public status?: Status;

  constructor({ statusProvider, log }: SorbetExtensionContext) {
    this.log = log;
    this.onStatusChangedEmitter = new EventEmitter();
    this.status = this.mapStatus(statusProvider.serverStatus);
    this.disposables = [
      this.onStatusChangedEmitter,
      statusProvider.onStatusChanged((e) => {
        const mappedStatus = this.mapStatus(e.status);
        this.log.trace("EVENT", this.status, mappedStatus);
        if (mappedStatus) {
          this.status = mappedStatus;
          this.onStatusChangedEmitter.fire(mappedStatus);
        }
      }),
    ];
  }

  dispose() {
    Disposable.from(...this.disposables).dispose();
  }

  private mapStatus(status: ServerStatus): Status | undefined {
    switch (status) {
      case ServerStatus.DISABLED:
        return Status.Disabled;
      case ServerStatus.INITIALIZING:
      case ServerStatus.RESTARTING:
        return Status.Start;
      case ServerStatus.RUNNING:
        return Status.Ready;
      default:
        return undefined;
    }
  }

  public get onStatusChanged(): Event<Status> | undefined {
    return this.onStatusChangedEmitter.event;
  }

  public toApi(): SorbetExtensionApi {
    return {
      status: () => this.status,
      onStatusChanged: this.onStatusChanged
    };
  }
}
