import { extensions, commands } from "vscode";
import { SorbetExtensionContext } from "./sorbetExtensionContext";

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

export class MetricClient {
  private apiPromise: Promise<Api | undefined>;
  private readonly context: SorbetExtensionContext;
  private readonly sorbetExtensionVersion: string;

  constructor(context: SorbetExtensionContext) {
    this.apiPromise = this.initSorbetMetricsApi();
    this.context = context;
    const sorbetExtension = extensions.getExtension("sorbet-vscode-extension");
    this.sorbetExtensionVersion =
      sorbetExtension?.packageJSON.version ?? "unknown";
  }

  /**
   * Build a tag set.
   * @param tags Tags to add to, or override, default ones.
   * @returns Tag set.
   */
  private buildTags(tags: Tags) {
    return {
      config_id: this.context.configuration.activeLspConfig?.id ?? "disabled",
      sorbet_extension_version: this.sorbetExtensionVersion,
      ...tags,
    };
  }

  private async initSorbetMetricsApi(): Promise<Api> {
    let sorbetMetricsApi: Api;
    try {
      const api = await commands.executeCommand(
        "sorbet.metrics.getExportedApi",
      );
      if (api) {
        this.context.log.info("Metrics-gathering initialized.");
        sorbetMetricsApi = api as Api;
        if (!sorbetMetricsApi.metricsEmitter.timing) {
          this.context.log.info("Timer metrics disabled (unsupported API).");
        }
      } else {
        this.context.log.info("Metrics-gathering disabled (no API)");
        sorbetMetricsApi = NoOpApi.INSTANCE;
      }
    } catch (reason) {
      sorbetMetricsApi = NoOpApi.INSTANCE;
      const adjustedReason =
        (<any>reason)?.message ===
        "command 'sorbet.metrics.getExportedApi' not found"
          ? "Define the 'sorbet.metrics.getExportedApi' command to enable metrics gathering"
          : (<any>reason).message;

      this.context.log.error(
        `Metrics-gathering disabled (error): ${adjustedReason}`,
      );
    }
    return sorbetMetricsApi;
  }

  /**
   * Emit a count metric.
   * @param metric Metric name.
   * @param count Metric count.
   * @param extraTags Tags to attach to metric.
   */
  public async emitCountMetric(
    metric: string,
    count: number,
    extraTags: Tags = {},
  ): Promise<void> {
    const api = await this.apiPromise;
    if (!api) {
      return;
    }

    const fullName = `${METRIC_PREFIX}${metric}`;
    const tags = this.buildTags(extraTags);
    api.metricsEmitter.increment(fullName, count, tags);
  }

  /**
   * Emit a time metric.
   * @param metric Metric name.
   * @param time Time.
   * @param extraTags Tags to attach to metric.
   */
  public async emitTimingMetric(
    metric: string,
    time: number | Date,
    extraTags: Tags = {},
  ): Promise<void> {
    const api = await this.apiPromise;
    if (!api?.metricsEmitter.timing) {
      // Ignore timers if metrics extension does not support them.
      return;
    }

    const fullName = `${METRIC_PREFIX}${metric}`;
    const tags = this.buildTags(extraTags);
    api.metricsEmitter.timing(fullName, time, tags);
  }

  /**
   * Set {@link API} instance to use. This is intended for tests only.
   * @param api API instance
   */
  public setSorbetMetricsApi(api: Api): void {
    this.apiPromise = Promise.resolve(api);
  }
}
