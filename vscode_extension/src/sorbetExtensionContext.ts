import { Disposable, ExtensionContext } from "vscode";
import { DefaultSorbetWorkspaceContext, SorbetExtensionConfig } from "./config";
import { OutputChannelLog } from "./log";
import { MetricClient } from "./metricsClient";
import { SorbetStatusProvider } from "./sorbetStatusProvider";

export class SorbetExtensionContext implements Disposable {
  public readonly configuration: SorbetExtensionConfig;
  private readonly disposable: Disposable;
  public readonly extensionContext: ExtensionContext;
  public readonly log: OutputChannelLog;
  public readonly metrics: MetricClient;
  public readonly statusProvider: SorbetStatusProvider;

  constructor(context: ExtensionContext) {
    this.configuration = new SorbetExtensionConfig(
      new DefaultSorbetWorkspaceContext(context),
    );
    this.extensionContext = context;
    this.log = new OutputChannelLog("Sorbet");
    this.metrics = new MetricClient(this);
    this.statusProvider = new SorbetStatusProvider(this);

    this.disposable = Disposable.from(
      this.configuration,
      this.log,
      this.statusProvider,
    );
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    this.disposable.dispose();
  }
}
