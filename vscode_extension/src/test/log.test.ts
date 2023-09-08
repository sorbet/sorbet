import * as vscode from "vscode";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import {
  getLogLevelFromEnvironment,
  getLogLevelFromString,
  LogLevel,
  OutputChannelLog,
  VSCODE_SORBETEXT_LOG_LEVEL,
} from "../log";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("getLogLevelFromEnvironment", () => {
    const restorableValue = process.env[VSCODE_SORBETEXT_LOG_LEVEL];
    const restoreProcessEnv = {
      restore: () => {
        if (restorableValue !== undefined) {
          process.env[VSCODE_SORBETEXT_LOG_LEVEL] = restorableValue;
        } else {
          delete process.env.VSCODE_PAYEXT_LOG_LEVEL;
        }
      },
    };
    testRestorables.push(restoreProcessEnv);

    assert.strictEqual(
      LogLevel.Info,
      getLogLevelFromEnvironment(),
      "Defaults to LogLevel.Info when undefined",
    );

    process.env[VSCODE_SORBETEXT_LOG_LEVEL] = "Debug";
    assert.strictEqual(
      LogLevel.Debug,
      getLogLevelFromEnvironment(VSCODE_SORBETEXT_LOG_LEVEL, LogLevel.Debug),
      "Defaults to provided default when undefined",
    );

    process.env[VSCODE_SORBETEXT_LOG_LEVEL] = "Not a LogLevel";
    assert.strictEqual(
      LogLevel.Info,
      getLogLevelFromEnvironment(),
      "Defaults to LogLevel.Info when invalid",
    );

    process.env[VSCODE_SORBETEXT_LOG_LEVEL] = "Error";
    assert.strictEqual(
      LogLevel.Error,
      getLogLevelFromEnvironment(),
      "Defaults to LogLevel.Error when invalid",
    );
  });

  test("getLogLevelFromString", () => {
    // Literal conversion
    assert.strictEqual(LogLevel.Critical, getLogLevelFromString("Critical"));
    assert.strictEqual(LogLevel.Debug, getLogLevelFromString("Debug"));
    assert.strictEqual(LogLevel.Error, getLogLevelFromString("Error"));
    assert.strictEqual(LogLevel.Info, getLogLevelFromString("Info"));
    assert.strictEqual(LogLevel.Off, getLogLevelFromString("Off"));
    assert.strictEqual(LogLevel.Trace, getLogLevelFromString("Trace"));
    assert.strictEqual(LogLevel.Warning, getLogLevelFromString("Warning"));
    // Case insensitive
    assert.strictEqual(LogLevel.Critical, getLogLevelFromString("CRITICAL"));
    assert.strictEqual(LogLevel.Critical, getLogLevelFromString("critical"));
    assert.strictEqual(LogLevel.Critical, getLogLevelFromString("cRiTiCal"));
    // Invalid string
    assert.strictEqual(undefined, getLogLevelFromString("Random Value"));
    assert.strictEqual(undefined, getLogLevelFromString("  Critical  "));
  });

  test("OutputChannel is initialized correctly", () => {
    const expectedName = "Test";

    const createOutputChannelStub = sinon
      .stub(vscode.window, "createOutputChannel")
      .returns(<vscode.OutputChannel>{
        appendLine(_value: string) {},
      });
    testRestorables.push(createOutputChannelStub);

    const log = new OutputChannelLog(expectedName);
    assert.doesNotThrow(() => log.info("test message"));

    sinon.assert.calledWithExactly(createOutputChannelStub, expectedName);
  });

  test("OutputChannel.logLevel can be updated", () => {
    const expectedLogLevel = LogLevel.Warning;
    const createOutputChannelStub = sinon
      .stub(vscode.window, "createOutputChannel")
      .returns(<vscode.OutputChannel>{
        appendLine(_value: string) {},
      });
    testRestorables.push(createOutputChannelStub);

    const log = new OutputChannelLog("Test", LogLevel.Info);
    assert.strictEqual(log.level, LogLevel.Info, "Expected default state");

    log.level = expectedLogLevel;
    assert.strictEqual(log.level, expectedLogLevel, "Expected new state");

    sinon.assert.calledOnce(createOutputChannelStub);
  });

  test("All log methods write", () => {
    const logMessage = "Test log entry";
    let callCount = 0;
    const createOutputChannelStub = sinon
      .stub(vscode.window, "createOutputChannel")
      .returns(<vscode.OutputChannel>(<any>{
        appendLine(value: string) {
          assert.ok(value.endsWith(logMessage), `Found: ${value}`);
          callCount++;
        },
      }));
    testRestorables.push(createOutputChannelStub);

    const log = new OutputChannelLog("Test", LogLevel.Trace);
    log.error(logMessage);
    assert.strictEqual(1, callCount);
    log.info(logMessage);
    assert.strictEqual(2, callCount);
    log.warning(logMessage);
    assert.strictEqual(3, callCount);
    log.debug(logMessage);
    assert.strictEqual(4, callCount);
    log.trace(logMessage);
    assert.strictEqual(5, callCount);

    sinon.assert.calledOnce(createOutputChannelStub);
  });

  test("Only log methods of appropriate level write", () => {
    const logMessage = "Test log entry";
    let callCount = 0;
    const createOutputChannelStub = sinon
      .stub(vscode.window, "createOutputChannel")
      .returns(<vscode.OutputChannel>{
        appendLine(value: string) {
          assert.ok(value.endsWith(logMessage), `Found: ${value}`);
          callCount++;
        },
      });
    testRestorables.push(createOutputChannelStub);

    const log = new OutputChannelLog("Test", LogLevel.Info);
    log.debug(logMessage);
    log.trace(logMessage);
    assert.strictEqual(
      0,
      callCount,
      "No calls to OutputChannel.appendLine expected below LogLevel.Info",
    );

    log.info(logMessage);
    assert.strictEqual(1, callCount);
    sinon.assert.calledOnce(createOutputChannelStub);
  });

  test("Log.error handles message and error", () => {
    const logMessage = "TestError";
    const logError = new Error("Test log entry");
    let callCount = 0;
    const createOutputChannelStub = sinon
      .stub(vscode.window, "createOutputChannel")
      .returns(<vscode.OutputChannel>{
        appendLine(value: string) {
          assert.ok(
            value.endsWith(`${logMessage} Error: ${logError.message}`),
            `Found: ${value}`,
          );
          callCount++;
        },
      });
    testRestorables.push(createOutputChannelStub);

    const log = new OutputChannelLog("Test", LogLevel.Info);
    log.error(logMessage, logError);

    assert.strictEqual(1, callCount);
    sinon.assert.calledOnce(createOutputChannelStub);
  });
});
