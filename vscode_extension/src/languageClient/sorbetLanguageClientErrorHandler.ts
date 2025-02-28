import {
  CloseHandlerResult,
  ErrorHandler,
  ErrorHandlerResult,
  Message,
} from "vscode-languageclient";
import { SorbetLanguageClient2 } from "./sorbetLanguageClient";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

export class SorbetLanguageClientErrorHandler implements ErrorHandler {
  private readonly client: SorbetLanguageClient2;
  private readonly context: SorbetExtensionContext;

  constructor(context: SorbetExtensionContext, client: SorbetLanguageClient2) {
    this.context = context;
    this.client = client;
  }

  closed(): CloseHandlerResult | Promise<CloseHandlerResult> {
    throw new Error("Method not implemented.");
  }

  initializationError(_error: Error | any): boolean {
    throw new Error("Method not implemented.");
  }

  error(
    _error: Error,
    _message: Message | undefined,
    _count: number | undefined,
  ): ErrorHandlerResult | Promise<ErrorHandlerResult> {
    throw new Error("Method not implemented.");
  }
}
