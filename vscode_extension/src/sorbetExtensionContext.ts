import { Disposable, ExtensionContext, OutputChannel, window } from "vscode";
import { DefaultSorbetWorkspaceContext, SorbetExtensionConfig } from "./config";
import { Tags, emitCountMetric, emitTimingMetric } from "./veneur";
import { SorbetStatusProvider } from "./sorbetStatusProvider";

export class SorbetExtensionContext implements Disposable {
  public readonly config: SorbetExtensionConfig;
  private readonly disposable: Disposable;
  public readonly extensionContext: ExtensionContext;
  public readonly outputChannel: OutputChannel;
  public readonly statusProvider: SorbetStatusProvider;

  constructor(context: ExtensionContext) {
    this.config = new SorbetExtensionConfig(
      new DefaultSorbetWorkspaceContext(context),
    );
    this.extensionContext = context;
    this.outputChannel = window.createOutputChannel("Sorbet");
    this.statusProvider = new SorbetStatusProvider(this);

    this.disposable = Disposable.from(this.config, this.outputChannel);
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    this.disposable.dispose();
  }

  /**
   * Emit a count metric.
   */
  public emitCountMetric(metric: string, count: number) {
    emitCountMetric(this.config, this.outputChannel, metric, count);
  }

  /**
   * Emit a timing metric.
   */
  public emitTimingMetric(
    metric: string,
    time: number | Date,
    extraTags: Tags = {},
  ) {
    emitTimingMetric(this.config, this.outputChannel, metric, time, extraTags);
  }
}
