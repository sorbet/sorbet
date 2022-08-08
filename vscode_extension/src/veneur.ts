import { OutputChannel, extensions, commands } from "vscode";
import { SorbetExtensionConfig } from "./config";

export const METRIC_PREFIX = "ruby_typer.lsp.extension.";

/**
 * Exported by the `sorbet-internal.metrics` extension
 */
export type Tags = Readonly<{ [metric: string]: string }>;

export interface MetricsEmitter {
  /** Increments the given counter metric by the given count (default: 1 if unspecified). */
  increment(metricName: string, count?: number, tags?: Tags): Promise<void>;

  /** Sets the given gauge metric to the given value. */
  gauge(metricName: string, value: number, tags?: Tags): Promise<void>;

  /**
   * Records a runtime for the specific metric. Is not present on older versions of metrics extension.
   * TODO(jvilk): Make non-optional once new version is ~100% rolled out.
   */
  timing?(metricName: string, value: number | Date, tags?: Tags): Promise<void>;

  /** Emits any previously-unsent metrics. */
  flush(): Promise<void>;
}

export interface Api {
  readonly metricsEmitter: MetricsEmitter;
}

class NoOpMetricsEmitter implements MetricsEmitter {
  async increment(
    _metricName: string,
    _count?: number,
    _tags?: Tags,
  ): Promise<void> {} // eslint-disable-line no-empty-function

  async gauge(
    _metricName: string,
    _value: number,
    _tags?: Tags,
  ): Promise<void> {} // eslint-disable-line no-empty-function

  async timing(
    _metricName: string,
    _value: number | Date,
    _tags?: Tags,
  ): Promise<void> {} // eslint-disable-line no-empty-function

  async flush(): Promise<void> {} // eslint-disable-line no-empty-function
}

class NoOpApi implements Api {
  readonly metricsEmitter: MetricsEmitter = new NoOpMetricsEmitter();
  static INSTANCE = new NoOpApi();
}

let sorbetMetricsApi: Api | undefined;
// Exported for tests.
export function setSorbetMetricsApi(api: Api): void {
  sorbetMetricsApi = api;
}

async function initSorbetMetricsApi(outputChannel: OutputChannel) {
  return commands.executeCommand("sorbet.metrics.getExportedApi").then(
    (api) => {
      if (api) {
        outputChannel.appendLine("Sorbet metrics-gathering initialized.");
        sorbetMetricsApi = api as Api;
        if (!sorbetMetricsApi.metricsEmitter.timing) {
          outputChannel.appendLine(
            "Note: Timer metrics will not be reported; metrics extension does not support the timer API.",
          );
        }
      } else {
        sorbetMetricsApi = NoOpApi.INSTANCE;
        outputChannel.appendLine(
          `Sorbet metrics gathering disabled: unrecognized Api object: ${api}`,
        );
      }
    },
    (reason) => {
      sorbetMetricsApi = NoOpApi.INSTANCE;
      const adjustedReason =
        reason.message === "command 'sorbet.metrics.getExportedApi' not found"
          ? "Define the 'sorbet.metrics.getExportedApi' command to enable metrics gathering"
          : reason.message;
      outputChannel.appendLine(
        `Sorbet metrics gathering disabled: ${adjustedReason}`,
      );
    },
  );
}

const sorbetExtension = extensions.getExtension("sorbet-vscode-extension");
const sorbetExtensionVersion =
  (sorbetExtension && `${sorbetExtension.packageJSON.version}`) || "unknown";

/**
 * Emit a metric via Veneur.
 */
export function emitCountMetric(
  sorbetExtensionConfig: SorbetExtensionConfig,
  _outputChannel: OutputChannel,
  metric: string,
  count: number,
) {
  const { activeLspConfig } = sorbetExtensionConfig;
  const fullName = `${METRIC_PREFIX}${metric}`;
  const tags = {
    config_id: activeLspConfig ? activeLspConfig.id : "disabled",
    sorbet_extension_version: sorbetExtensionVersion,
  };
  if (sorbetMetricsApi) {
    sorbetMetricsApi.metricsEmitter.increment(fullName, count, tags);
    return;
  }
  initSorbetMetricsApi(_outputChannel).then(() => {
    if (sorbetMetricsApi) {
      emitCountMetric(sorbetExtensionConfig, _outputChannel, metric, count);
    }
  });
}

export function emitTimingMetric(
  sorbetExtensionConfig: SorbetExtensionConfig,
  _outputChannel: OutputChannel,
  metric: string,
  time: number | Date,
  extraTags: Tags = {},
) {
  const { activeLspConfig } = sorbetExtensionConfig;
  const fullName = `${METRIC_PREFIX}${metric}`;
  const tags = {
    config_id: activeLspConfig ? activeLspConfig.id : "disabled",
    sorbet_extension_version: sorbetExtensionVersion,
    ...extraTags,
  };
  if (sorbetMetricsApi) {
    // Ignore timers if metrics extension does not support them.
    if (sorbetMetricsApi.metricsEmitter.timing) {
      sorbetMetricsApi.metricsEmitter.timing(fullName, time, tags);
    }
    return;
  }
  initSorbetMetricsApi(_outputChannel).then(() => {
    if (sorbetMetricsApi) {
      emitTimingMetric(sorbetExtensionConfig, _outputChannel, metric, time);
    }
  });
}
