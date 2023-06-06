import { QuickPickItem, window } from "vscode";
import { LogLevel } from "../log";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

export type LogLevelQuickPickItem = QuickPickItem & {
  level: LogLevel;
};

/**
 * Set logging level on associated 'Log' instance.
 */
export class SetLogLevel {
  private readonly context: SorbetExtensionContext;

  constructor(context: SorbetExtensionContext) {
    this.context = context;
  }

  /**
   * Execute command.
   * @param level Log level. If not provided, user will be prompted for it.
   */
  public async execute(level?: LogLevel): Promise<void> {
    const newLevel = level ?? (await this.getLogLevel());
    if (newLevel === undefined) {
      return; // Canceled
    }
    this.context.log.level = newLevel;
  }

  private async getLogLevel(): Promise<LogLevel | undefined> {
    const items = [
      LogLevel.Trace,
      LogLevel.Debug,
      LogLevel.Info,
      LogLevel.Warning,
      LogLevel.Error,
      LogLevel.Critical,
      LogLevel.Off,
    ].map((logLevel) => {
      const item = <LogLevelQuickPickItem>{
        label: `${this.context.log.level === logLevel ? "• " : ""}${
          LogLevel[logLevel]
        }`,
        level: logLevel,
      };
      return item;
    });

    const selectedLevel = await window.showQuickPick(items, {
      placeHolder: "Select log level",
    });
    return selectedLevel?.level;
  }
}
