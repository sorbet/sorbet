import { Disposable, ExtensionContext, OutputChannel } from "vscode";
import { Log, OutputChannelLog } from "./log";
import { MetricClient } from "./metricsClient";
import { SorbetExtensionConfig } from "./sorbetExtensionConfig";
import { SorbetStatusProvider } from "./sorbetStatusProvider";

export class SorbetExtensionContext implements Disposable {
  public readonly configuration: SorbetExtensionConfig;
  private readonly disposable: Disposable;
  public readonly extensionContext: ExtensionContext;
  public readonly metrics: MetricClient;
  public readonly statusProvider: SorbetStatusProvider;
  private readonly wrappedLog: OutputChannelLog;

  constructor(context: ExtensionContext) {
    this.configuration = new SorbetExtensionConfig(context);
    this.extensionContext = context;
    this.metrics = new MetricClient(this);
    this.statusProvider = new SorbetStatusProvider(this);
    this.wrappedLog = new OutputChannelLog("Sorbet");

    this.disposable = Disposable.from(
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
   * Output channel used by {@link log}.
   *
   * This is exposed separately to promote use of the {@link Log} interface
   * and prevent accessing the UI component, unless necessary.
   */
  public get logOutputChannel(): OutputChannel {
    return this.wrappedLog.outputChannel;
  }
}
