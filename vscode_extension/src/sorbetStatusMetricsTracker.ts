import { Disposable } from "vscode";

import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { ServerStatus } from "./types";

/**
 * A Disposable that listens to status changes through the SorbetStatusProvider,
 * and emit metrics about them through the configured MetricsEmitter
 */
export class SorbetStatusMetricsTracker implements Disposable {
  private readonly context: SorbetExtensionContext;
  private readonly disposable: Disposable;

  constructor(context: SorbetExtensionContext) {
    this.context = context;

    this.disposable = Disposable.from(
      this.context.statusProvider.onStatusChanged((e) => {
        this.context.metrics.emitTimingMetric("status.changed", new Date(), {
          status: ServerStatus[e.status],
          error: e.error || "",
        });
      }),
      this.context.statusProvider.onShowOperation((params) => {
        this.context.metrics.emitTimingMetric("status.operation", new Date(), {
          operationName: params.operationName,
          status: params.status,
        });
      }),
    );
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    this.disposable.dispose();
  }
}
