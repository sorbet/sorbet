import { Log, LogLevel } from "../log";

/**
 * Stub {@link Log} interface.
 * @param level Default log-level.
 */
export function createLogStub(level = LogLevel.Critical): Log {
  return {
    debug: (_message: string) => {},
    error: (_messageOrError: string | Error) => {},
    info: (_message: string) => {},
    trace: (_message: string) => {},
    warning: (_message: string) => {},
    level,
  };
}
