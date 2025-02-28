import { EventEmitter, Disposable } from "vscode";
import { LanguageClient } from "vscode-languageclient/node";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { ServerStatus } from "../types";
import { createLanguageClient } from "./languageClient";
import { instrumentLanguageClient } from "./metrics";
import { SorbetLanguageClientErrorHandler } from "./sorbetLanguageClientErrorHandler";
import { SorbetServerCapabilities } from "./sorbetServerCapabilities";

export class SorbetLanguageClient2 implements Disposable {
  private readonly context: SorbetExtensionContext;
  private readonly languageClient: LanguageClient;
  private readonly onStatusChangeEmitter: EventEmitter<ServerStatus>;
  private wrappedStatus: ServerStatus;

  constructor(context: SorbetExtensionContext) {
    this.context = context;
    this.languageClient = instrumentLanguageClient(
      createLanguageClient(
        this.context,
        new SorbetLanguageClientErrorHandler(this.context, this),
      ),
      this.context.metrics,
    );
    this.onStatusChangeEmitter = new EventEmitter();
    this.wrappedStatus = ServerStatus.DISABLED;
  }

  dispose(): void {
    this.onStatusChangeEmitter.dispose();
  }

  /**
   * Sorbet language server {@link SorbetServerCapabilities capabilities}. Only
   * available when the server has been initialized.
   */
  public get capabilities(): SorbetServerCapabilities | undefined {
    return <SorbetServerCapabilities | undefined>(
      this.languageClient.initializeResult?.capabilities
    );
  }

  /**
   * Send a request to the language server. See {@link LanguageClient.sendRequest}.
   */
  public sendRequest<TResponse>(
    method: string,
    param: any,
  ): Promise<TResponse> {
    return this.languageClient.sendRequest<TResponse>(method, param);
  }

  /**
   * Send a notification to the language server. See {@link LanguageClient.sendNotification}.
   */
  public sendNotification(method: string, param: any): Promise<void> {
    return this.languageClient.sendNotification(method, param);
  }

  /**
   * Start client. Note that sending a {@link sendRequest request} or
   * {@link sendNotification notification} causes for the client to be started.
   */
  public async start(): Promise<void> {
    if (!this.languageClient.needsStart()) {
      this.context.log.debug("Ignored unnecessary start request");
      return;
    }

    this.status = ServerStatus.INITIALIZING;
    try {
      await this.languageClient.start();
      this.context.log.info("Sorbet has started.");
    } catch (error) {
      this.status = ServerStatus.ERROR;
      throw error;
    }
  }

  public async stop() {
    if (!this.languageClient.needsStop()) {
      this.context.log.debug("Ignored unnecessary stop request");
      return;
    }

    this.status = ServerStatus.DISABLED;
    this.context.log.info("Sorbet has stopped.");
  }

  public get status(): ServerStatus {
    return this.wrappedStatus;
  }

  private set status(value: ServerStatus) {
    if (this.wrappedStatus !== value) {
      this.wrappedStatus = value;
      this.onStatusChangeEmitter.fire(value);
    }
  }
}
