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
  COMMAND = "command",
  CRASH = "crash",
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
