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
        console.log(
          `status changed: ${ServerStatus[e.status]} (error: '${e.error}')`,
        );
        this.context.metrics.emitTimingMetric("status.changed", new Date(), {
          status: ServerStatus[e.status],
          error: e.error || "",
        });
      }),
      this.context.statusProvider.onShowOperation((_params) => {
        console.log(
          `operation changed: ${_params.operationName} went to ${_params.status}`,
        );
        this.context.metrics.emitTimingMetric("status.operation", new Date(), {
          operationName: _params.operationName,
          status: _params.status,
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
