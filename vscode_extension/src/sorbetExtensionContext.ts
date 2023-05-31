import { Disposable, ExtensionContext, OutputChannel, window } from "vscode";
import { DefaultSorbetWorkspaceContext, SorbetExtensionConfig } from "./config";
import { MetricClient } from "./metricsClient";
import { SorbetStatusProvider } from "./sorbetStatusProvider";

export class SorbetExtensionContext implements Disposable {
  public readonly config: SorbetExtensionConfig;
  private readonly disposable: Disposable;
  public readonly extensionContext: ExtensionContext;
  public readonly metrics: MetricClient;
  public readonly outputChannel: OutputChannel;
  public readonly statusProvider: SorbetStatusProvider;

  constructor(context: ExtensionContext) {
    this.config = new SorbetExtensionConfig(
      new DefaultSorbetWorkspaceContext(context),
    );
    this.extensionContext = context;
    this.metrics = new MetricClient(this);
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
}
