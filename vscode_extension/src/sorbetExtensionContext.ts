import { Disposable, ExtensionContext, OutputChannel } from "vscode";
import { DefaultSorbetWorkspaceContext, SorbetExtensionConfig } from "./config";
import { Log, OutputChannelLog } from "./log";
import { MetricClient } from "./metricsClient";
import { SorbetStatusProvider } from "./sorbetStatusProvider";

export class SorbetExtensionContext implements Disposable {
  public readonly configuration: SorbetExtensionConfig;
  private readonly disposable: Disposable;
  public readonly extensionContext: ExtensionContext;
  public readonly metrics: MetricClient;
  public readonly statusProvider: SorbetStatusProvider;
  private readonly wrappedLog: OutputChannelLog;

  constructor(context: ExtensionContext) {
    const sorbetWorkspaceContext = new DefaultSorbetWorkspaceContext(context);
    this.configuration = new SorbetExtensionConfig(sorbetWorkspaceContext);
    this.extensionContext = context;
    this.wrappedLog = new OutputChannelLog("Sorbet");
    this.metrics = new MetricClient(this);
    this.statusProvider = new SorbetStatusProvider(this);

    this.disposable = Disposable.from(
      sorbetWorkspaceContext,
      this.configuration,
      this.statusProvider,
      this.wrappedLog,
    );
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    this.disposable.dispose();
  }

  /**
   * Logger.
   */
  public get log(): Log {
    return this.wrappedLog;
  }

  /**
   * Output channel used by {@link log}. This is exposed separately to promote
   * use of the {@link Log} interface instead of accessing the UI component.
   */
  public get logOutputChannel(): OutputChannel {
    return this.wrappedLog.outputChannel;
  }
}
