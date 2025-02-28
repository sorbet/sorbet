import { ExtensionMode, OutputChannel, workspace } from "vscode";
import {
  LanguageClientOptions,
  RevealOutputChannelOn,
} from "vscode-languageclient";
import { LanguageClient, ServerOptions } from "vscode-languageclient/node";
import { SorbetLanguageClientErrorHandler } from "./sorbetLanguageClientErrorHandler";
import {
  backwardsCompatibleTrackUntyped,
  SorbetExtensionConfig,
} from "../config";
import { Log } from "../log";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { SorbetLspConfig } from "../sorbetLspConfig";

export function createLanguageClient(
  context: SorbetExtensionContext,
  errorHandler: SorbetLanguageClientErrorHandler,
  forceDebugLog?: boolean,
): LanguageClient {
  const { activeLspConfig: lspConfig } = context.configuration;
  if (!lspConfig) {
    throw new Error("No active LSP configuration");
  }

  const serverOptions = createServerOptions(lspConfig, context.log);
  const clientOptions = createClientOptions(
    context.configuration,
    context.logOutputChannel,
    errorHandler,
    context.log,
  );

  const client = new CustomLanguageClient(
    "ruby",
    "Sorbet",
    serverOptions,
    clientOptions,
    forceDebugLog ??
      context.extensionContext.extensionMode !== ExtensionMode.Production,
  );
  return client;
}

function createClientOptions(
  config: SorbetExtensionConfig,
  outputChannel: OutputChannel,
  errorHandler: SorbetLanguageClientErrorHandler,
  log: Log,
): LanguageClientOptions {
  const initializationOptions = {
    enableTypedFalseCompletionNudges: config.typedFalseCompletionNudges,
    highlightUntyped: backwardsCompatibleTrackUntyped(
      log,
      config.highlightUntyped,
    ),
    supportsOperationNotifications: true,
    supportsSorbetURIs: true,
  };
  return {
    documentSelector: [
      { language: "ruby", scheme: "file" },
      { language: "ruby", scheme: "sorbet" },
    ],
    errorHandler,
    initializationOptions,
    initializationFailedHandler: (error) =>
      errorHandler.initializationError(error),
    outputChannel,
    revealOutputChannelOn: config.revealOutputOnError
      ? RevealOutputChannelOn.Error
      : RevealOutputChannelOn.Never,
  };
}

function createServerOptions(
  lspConfig: SorbetLspConfig,
  log: Log,
): ServerOptions {
  const cwd = workspace.workspaceFolders?.[0]?.uri.fsPath;
  if (!cwd) {
    log.debug("No workspace to determine CWD");
  } else if (workspace.workspaceFolders!.length > 1) {
    log.debug("Multi-root workspace, using first workspace as CWD");
  }

  return {
    args: lspConfig.command.slice(1),
    command: lspConfig.command[0],
    options: {
      cwd,
      env: { ...process.env, ...lspConfig.env },
    },
  };
}

class CustomLanguageClient extends LanguageClient {
  error(
    message: string,
    data?: any,
    showNotification?: boolean | "force",
  ): void {
    super.error(
      message,
      data,
      showNotification === "force" ? true : showNotification,
    );
  }

  protected async handleConnectionClosed(): Promise<void> {
    try {
      await super.handleConnectionClosed();
    } catch (e) {
      console.error(e);
    }
  }
}
