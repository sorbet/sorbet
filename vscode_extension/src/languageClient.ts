import { WorkspaceFolder } from "vscode";
import { ErrorHandler, RevealOutputChannelOn } from "vscode-languageclient";
import { LanguageClient, ServerOptions } from "vscode-languageclient/node";
import { backwardsCompatibleTrackUntyped } from "./config";
import { SorbetExtensionContext } from "./sorbetExtensionContext";

/**
 * Create Language Client for Sorber Server.
 */
export function createClient(
  context: SorbetExtensionContext,
  workspaceFolder: WorkspaceFolder | undefined,
  serverOptions: ServerOptions,
  errorHandler: ErrorHandler,
): LanguageClient {
  const initializationOptions = {
    // Opt in to sorbet/showOperation notifications.
    supportsOperationNotifications: true,
    // Let Sorbet know that we can handle sorbet:// URIs for generated files.
    supportsSorbetURIs: true,
    highlightUntyped: backwardsCompatibleTrackUntyped(
      context.log,
      context.configuration.highlightUntyped,
    ),
    enableTypedFalseCompletionNudges:
      context.configuration.typedFalseCompletionNudges,
    highlightUntypedDiagnosticSeverity:
      context.configuration.highlightUntypedDiagnosticSeverity,
    inlayTypeHints: context.configuration.inlayTypeHints,
  };

  context.log.debug(
    "Initializing with initializationOptions",
    ...Object.entries(initializationOptions).map(([k, v]) => `${k}:${v}`),
  );

  const pattern = workspaceFolder
    ? `${workspaceFolder.uri.path}/**/*`
    : undefined;
  const client = new CustomLanguageClient("ruby", "Sorbet", serverOptions, {
    documentSelector: [
      { language: "ruby", scheme: "file", pattern },
      // Support queries on generated files with sorbet:// URIs that do not exist editor-side.
      { language: "ruby", scheme: "sorbet" },
    ],
    errorHandler,
    initializationOptions,
    initializationFailedHandler: (error) => {
      context.log.error(
        "Failed to initialize Sorbet language server.",
        error instanceof Error ? error : undefined,
      );
      return false;
    },
    outputChannel: context.logOutputChannel,
    workspaceFolder,
    revealOutputChannelOn: context.configuration.revealOutputOnError
      ? RevealOutputChannelOn.Error
      : RevealOutputChannelOn.Never,
  });

  return client;
}

// This implementation exists for the sole purpose of overriding the `force` flag
// as the error/closed/init handlers are not used in every case. In particular, if
// the server fails to start, errors are shown as dialog notification, with errors
// similar to:
//
// - Sorbet client: couldn't create connection to server.
// - Connection to server got closed. Server will not be restarted.
//
// By not forcing a UI component, they are just routed to existing loggers.
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
}
