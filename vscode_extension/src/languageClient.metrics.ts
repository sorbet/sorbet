import { LanguageClient } from "vscode-languageclient/node";
import { MetricsClient } from "./metricsClient";

/**
 * Shims the language client object so that all requests sent get timed.
 * @returns The instrumented language client.
 */
export function instrumentLanguageClient(
  client: LanguageClient,
  metrics: MetricsClient,
): LanguageClient {
  const originalSendRequest = client.sendRequest;

  client.sendRequest = (method: any, ...args: any[]) => {
    const now = new Date();

    const requestName = typeof method === "string" ? method : method.method;
    // Replace some special characters with underscores.
    const sanitizedRequestName = requestName.replace(/[/$]/g, "_");
    args.unshift(method);

    const response = originalSendRequest.apply(client, args as any);
    const metricName = `latency.${sanitizedRequestName}_ms`;
    response.then(
      () =>
        // This is request succeeded. If the request is canceled, the promise is rejected.
        metrics.emitTimingMetric(metricName, now, { success: "true" }),
      () =>
        // This request failed or was canceled.
        metrics.emitTimingMetric(metricName, now, { success: "false" }),
    );
    return response;
  };

  return client;
}
