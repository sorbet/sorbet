import * as vscode from "vscode";
import * as sinon from "sinon";
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

/**
 * Mock {@link vscode.workspace.getConfiguration}.
 * @param values Map of expected values.
 * @param name Configuration name.
 */
export function mockConfiguration(
  values: {
    [section: string]: (defaultValue: any) => any;
  },
  name: string = "sorbet",
) {
  return sinon
    .stub(vscode.workspace, "getConfiguration")
    .returns(<vscode.WorkspaceConfiguration>(<any>{
      get: (section: string, defaultValue: any) => {
        const getter = values[`${name}.${section}`];
        return getter ? getter(defaultValue) : defaultValue;
      },
      has: (section: string) => values.hasOwnProperty(section),
      update: (section: string, value: any) => {
        values[`${name}.${section}`] = () => value;
      },
    }));
}
