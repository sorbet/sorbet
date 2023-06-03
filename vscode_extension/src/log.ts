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
 * Output Channel-based implementation of logger.
 */
export class OutputChannelLog implements Disposable {
  private wrappedLevel: LogLevel;
  public readonly outputChannel: OutputChannel;

  constructor(name: string, level: LogLevel = LogLevel.Info) {
    this.wrappedLevel = level;
    // Future:
    // - VSCode 1.66 allows to pass-in a `language` to support syntax coloring.
    // - VSCode 1.75 allows to create a `LogOutputChannel`.
    this.outputChannel = window.createOutputChannel(name);
  }

  private appendLine(level: string, message: string): void {
    const formattedMessage = `${new Date().toISOString()} [${level.toLowerCase()}] ${message}`.trim();
    this.outputChannel.appendLine(formattedMessage);
  }

  /**
   * Appends a new debug message to the log.
   * @param message Log message.
   */
  public debug(message: string): void {
    if (this.level <= LogLevel.Debug) {
      this.appendLine("Debug", message);
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
   */
  public error(errorOrMessage: string | Error, error?: Error): void {
    if (this.level <= LogLevel.Error) {
      let message: string;
      if (typeof errorOrMessage === "string") {
        message = errorOrMessage;
        if (error) {
          message += ` Error: ${err2Str(error)}`;
        }
      } else {
        message = err2Str(errorOrMessage);
      }
      this.appendLine("Error", message);
    }

    function err2Str(err: Error) {
      return err.message || err.name || `\n${err.stack || ""}`;
    }
  }

  /**
   * Appends a new information message to the log.
   * @param message Log message.
   */
  public info(message: string): void {
    if (this.level <= LogLevel.Info) {
      this.appendLine("Info", message);
    }
  }

  /**
   * Log level.
   */
  public get level(): LogLevel {
    return this.wrappedLevel;
  }

  public set level(level: LogLevel) {
    if (this.wrappedLevel !== level) {
      this.wrappedLevel = level;
      this.outputChannel.appendLine(`Log level changed to: ${LogLevel[level]}`);
    }
  }

  /**
   * Appends a new trace message to the log.
   * @param message Log message.
   */
  public trace(message: string): void {
    if (this.level <= LogLevel.Trace) {
      this.appendLine("Trace", message);
    }
  }

  /**
   * Appends a new warning message to the log.
   * @param message Log message.
   */
  public warning(message: string): void {
    if (this.level <= LogLevel.Warning) {
      this.appendLine("Warning", message);
    }
  }
}
