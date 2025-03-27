import { Disposable, Event, EventEmitter } from "vscode";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { ServerStatus } from "./types";

/**
 * Status changes reported by extension.
 */
const enum Status {
  /**
   * Sorbet Language Server has been disabled.
   */
  Disabled = "disabled",
  /**
   * Sorbet Language Server encountered an error. This state does not correlate
   * to code typing errors.
   */
  Error = "error",
  /**
   * Sorbet Language Server is running.
   */
  Running = "running",
  /**
   * Sorbet server is being started. The event might repeat in case of error or
   * if the server was previously stopped.
   */
  Start = "start",
}

/**
 * Sorbet Extension API.
 *
 * !!! IMPORTANT !!!
 * Maintain backward and forward compatibility in all changes to this interface
 * to prevent breaking consumer extensions:
 *  1. Use optional properties.
 *  2. NEVER expose internal types directly.
 */
export interface SorbetExtensionApi {
  status?: Status;
  readonly onStatusChanged?: Event<Status>;
}

export class SorbetExtensionApiImpl implements Disposable {
  private readonly disposables: Disposable[];
  private readonly onStatusChangedEmitter: EventEmitter<Status>;
  private status?: Status;

  constructor({ statusProvider }: SorbetExtensionContext) {
    this.onStatusChangedEmitter = new EventEmitter();
    this.status = this.mapStatus(statusProvider.serverStatus);

    this.disposables = [
      this.onStatusChangedEmitter,
      statusProvider.onStatusChanged((e) => {
        const mappedStatus = this.mapStatus(e.status);
        if (mappedStatus && this.status !== mappedStatus) {
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
      case ServerStatus.ERROR:
        return Status.Error;
      case ServerStatus.INITIALIZING:
      case ServerStatus.RESTARTING:
        return Status.Start;
      case ServerStatus.RUNNING:
        return Status.Running;
      default:
        return undefined;
    }
  }

  /**
   * Public API.
   */
  public toApi(): SorbetExtensionApi {
    // API returned to other extensions should be specific not use `this` as
    // that would expose internal implementation, e.g. `onStatusChangedEmitter`.
    return {
      status: this.status,
      onStatusChanged: this.onStatusChangedEmitter.event,
    };
  }
}
