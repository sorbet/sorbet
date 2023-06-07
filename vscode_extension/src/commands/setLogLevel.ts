import { QuickPickItem, window } from "vscode";
import { LogLevel } from "../log";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

export type LogLevelQuickPickItem = QuickPickItem & {
  level: LogLevel;
};

/**
 * Set logging level on associated 'Log' instance.
 * @param context Sorbet extension context.
 * @param level Log level. If not provided, user will be prompted for it.
 */
export async function setLogLevel(
  context: SorbetExtensionContext,
  level?: LogLevel,
): Promise<void> {
  const newLevel = level ?? (await getLogLevel(context.log.level));
  if (newLevel === undefined) {
    return; // User canceled
  }
  context.log.level = newLevel;
}

async function getLogLevel(
  currentLogLevel: LogLevel,
): Promise<LogLevel | undefined> {
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
      label: `${currentLogLevel === logLevel ? "• " : ""}${LogLevel[logLevel]}`,
      level: logLevel,
    };
    return item;
  });

  const selectedLevel = await window.showQuickPick(items, {
    placeHolder: "Select log level",
  });
  return selectedLevel?.level;
}
