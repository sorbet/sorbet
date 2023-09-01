/**
 * The parameters set along with a sorbet/showOperation notification from the server.
 */
export interface ShowOperationParams {
  operationName: string;
  description: string;
  status: "start" | "end";
}

// Reasons why Sorbet might be restarted.
export enum RestartReason {
  // Command manuallyinvoked from command palette
  COMMAND = "command",
  // Manually invoked from the user clicking on "Restart Sorbet" in status bar popup
  STATUS_BAR_BUTTON = "status_bar_button",
  // For environments where a wrapper script protects the `sorbet` invocation,
  // and fails to start it under certain circumstances (for example, an rsync
  // client not running in the background, or a VPN not being connected).
  WRAPPER_REFUSED_SPAWN = "wrapper_refused_spawn",
  // For situations where `sorbet` died because it was sent the TERM signal
  FORCIBLY_TERMINATED = "forcibly_terminated",
  // LanguageClient closed callback
  CRASH_LC_CLOSED = "crash_lc_closed",
  // LanguageClient error callback
  CRASH_LC_ERROR = "crash_lc_error",
  // Extension (non-LanguageClient) error
  CRASH_EXT_ERROR = "crash_ext_error",
  CONFIG_CHANGE = "config_change",
}

// Note: Sorbet is either running/in the process or running, or in an error state. There's no benign idle/not running state.
export const enum ServerStatus {
  // The language client is disabled.
  DISABLED,
  // The language client is restarting.
  RESTARTING,
  // The language client is initializing.
  INITIALIZING,
  // The language client is running, and so is Sorbet.
  RUNNING,
  // An error has occurred. The user must dismiss the error.
  ERROR,
}
