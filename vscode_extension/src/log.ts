import { Disposable, OutputChannel, window } from "vscode";

/**
 * The severity level of a log message.
 * Based on $/src/vs/vscode.proposed.d.ts
 */
export enum LogLevel {
  Trace = 1,
  Debug = 2,
  Info = 3,
  Warning = 4,
  Error = 5,
  Critical = 6,
  Off = 7,
}
/**
 * Environment variable defining log level.
 */
export const VSCODE_SORBETEXT_LOG_LEVEL = "VSCODE_SORBETEXT_LOG_LEVEL";

/**
 * Get log-level as defined in env, otherwise returns `defaultLevel`.
 * @param name Environment variable name.
 * @param defaultLevel Default value if environment does not define a valid one.
 */
export function getLogLevelFromEnvironment(
  name: string = VSCODE_SORBETEXT_LOG_LEVEL,
  defaultLevel: LogLevel = LogLevel.Info,
): LogLevel {
  let logLevel = defaultLevel;
  const envLogLevel = process.env[name]?.trim();
  if (envLogLevel) {
    const parsedLogLevel = getLogLevelFromString(envLogLevel);
    if (parsedLogLevel !== undefined) {
      logLevel = parsedLogLevel;
    }
  }
  return logLevel;
}

/**
 * Get `LogLevel` from name, case-insensitively.
 * @param name Log level name.
 */
export function getLogLevelFromString(name: string): LogLevel | undefined {
  const upcaseLevel = name.toUpperCase();
  const entry = Object.entries(LogLevel).find(
    (e) => e[0].toUpperCase() === upcaseLevel,
  );
  return entry && <LogLevel>entry[1];
}

/**
 * Logger.
 */
export interface Log {
  /**
   * Appends a new debug message to the log.
   * @param message Log message.
   * @param args Additional values to log.
   */
  debug(message: string, ...args: any[]): void;

  /**
   * Appends a new error message to the log.
   * @param message Log message.
   * @param error Error.
   * @param args Additional values to log.
   */
  error(message: string, error: Error, ...args: any[]): void;

  /**
   * Appends a new error message to the log.
   * @param errorOrMessage Error or log message.
   * @param args Additional values to log.
   */
  error(errorOrMessage: string | Error, ...args: any[]): void;

  /**
   * Appends a new information message to the log.
   * @param message Log message.
   * @param args Additional values to log.
   */
  info(message: string, ...args: any[]): void;

  /**
   * Log level.
   */
  logLevel: LogLevel;

  /**
   * Appends a new trace message to the log.
   * @param message Log message.
   * @param args Additional values to log.
   */
  trace(message: string, ...args: any[]): void;

  /**
   * Appends a new warning message to the log.
   * @param message Log message.
   * @param args Additional values to log.
   */
  warn(message: string, ...args: any[]): void;
}

/**
 * Output Channel-based implementation of logger.
 */
export class OutputChannelLog implements Log, Disposable {
  private wrappedLevel: LogLevel;
  public readonly outputChannel: OutputChannel;

  constructor(name: string, level: LogLevel = LogLevel.Info) {
    this.wrappedLevel = level;
    // Future:
    // - VSCode 1.66 allows to pass-in a `language` to support syntax coloring.
    // - VSCode 1.75 allows to create a `LogOutputChannel`.
    this.outputChannel = window.createOutputChannel(name);
  }

  private appendLine(level: string, message: string, args: any[]): void {
    const formattedMessage = [
      new Date().toISOString(),
      `[${level.toLowerCase()}]`,
      message,
      ...args,
    ].join(" ");
    this.outputChannel.appendLine(formattedMessage);
  }

  /**
   * Appends a new debug message to the log.
   * @param message Log message.
   * @param args Additional values to log.
   */
  public debug(message: string, ...args: any[]): void {
    if (this.logLevel <= LogLevel.Debug) {
      this.appendLine("Debug", message, args);
    }
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    this.outputChannel.dispose();
  }

  /**
   * Appends a new error message to the log.
   * @param errorOrMessage Error or log message.
   * @param error Error (only used when `errorOrMessage` is not a `string`).
   * @param args Additional values to log.
   */
  public error(
    errorOrMessage: string | Error,
    error?: Error,
    ...args: any[]
  ): void {
    if (this.logLevel <= LogLevel.Error) {
      let message: string;
      if (typeof errorOrMessage === "string") {
        message = errorOrMessage;
        if (typeof error === "string") {
          args.unshift(error);
        } else if (error) {
          args.unshift(err2Str(error));
          args.unshift("Error:");
        }
      } else {
        message = err2Str(errorOrMessage);
      }
      this.appendLine("Error", message, args);
    }

    function err2Str(err: Error) {
      return err.message || err.name || `\n${err.stack || ""}`;
    }
  }

  /**
   * Appends a new information message to the log.
   * @param message Log message.
   * @param args Additional values to log.
   */
  public info(message: string, ...args: any[]): void {
    if (this.logLevel <= LogLevel.Info) {
      this.appendLine("Info", message, args);
    }
  }

  /**
   * Log level.
   */
  public get logLevel(): LogLevel {
    return this.wrappedLevel;
  }

  public set logLevel(level: LogLevel) {
    if (this.wrappedLevel !== level) {
      this.wrappedLevel = level;
      this.outputChannel.appendLine(`Log level changed to: ${LogLevel[level]}`);
    }
  }

  /**
   * Appends a new trace message to the log.
   * @param message Log message.
   * @param args Additional values to log.
   */
  public trace(message: string, ...args: any[]): void {
    if (this.logLevel <= LogLevel.Trace) {
      this.appendLine("Trace", message, args);
    }
  }

  /**
   * Appends a new warning message to the log.
   * @param message Log message.
   * @param args Additional values to log.
   */
  public warn(message: string, ...args: any[]): void {
    if (this.logLevel <= LogLevel.Warning) {
      this.appendLine("Warning", message, args);
    }
  }
}
