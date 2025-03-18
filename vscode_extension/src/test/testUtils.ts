import { Log, LogLevel } from "../log";

/**
 * Stub {@link Log} interface.
 * @param level Default log-level.
 */
export function createLogStub(level = LogLevel.Critical): Log {
  return {
    debug: () => {},
    error: () => {},
    info: () => {},
    trace: () => {},
    warn: () => {},
    logLevel: level,
  };
}
