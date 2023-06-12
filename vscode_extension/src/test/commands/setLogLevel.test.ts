import * as vscode from "vscode";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import { LogLevelQuickPickItem, setLogLevel } from "../../commands/setLogLevel";
import { LogLevel } from "../../log";
import { SorbetExtensionContext } from "../../sorbetExtensionContext";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("setLogLevel: Shows dropdown when target-level argument is NOT provided", async () => {
    const expectedLogLevel = LogLevel.Warning;
    const showQuickPickSingleStub = sinon
      .stub(vscode.window, "showQuickPick")
      .resolves(<LogLevelQuickPickItem>{
        label: LogLevel[expectedLogLevel],
        level: expectedLogLevel,
      });
    testRestorables.push(showQuickPickSingleStub);

    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
    };
    assert.ok(context.log.level !== expectedLogLevel);
    await assert.doesNotReject(setLogLevel(context));
    assert.strictEqual(context.log.level, expectedLogLevel);

    sinon.assert.calledWithExactly(
      showQuickPickSingleStub,
      <ReadonlyArray<LogLevelQuickPickItem>>[
        {
          level: LogLevel.Trace,
          label: "Trace",
        },
        {
          level: LogLevel.Debug,
          label: "Debug",
        },
        {
          level: LogLevel.Info,
          label: "• Info",
        },
        {
          level: LogLevel.Warning,
          label: "Warning",
        },
        {
          level: LogLevel.Error,
          label: "Error",
        },
        {
          level: LogLevel.Critical,
          label: "Critical",
        },
        {
          level: LogLevel.Off,
          label: "Off",
        },
      ],
      {
        placeHolder: "Select log level",
      },
    );
  });

  test("setLogLevel: Shows no-dropdown when target-level argument is provided ", async () => {
    const expectedLogLevel = LogLevel.Warning;

    const showQuickPickSingleStub = sinon.stub(vscode.window, "showQuickPick");
    testRestorables.push(showQuickPickSingleStub);

    const log = createLogStub();
    const context = <SorbetExtensionContext>{ log };

    await assert.doesNotReject(setLogLevel(context, expectedLogLevel));
    assert.strictEqual(log.level, expectedLogLevel);

    sinon.assert.notCalled(showQuickPickSingleStub);
  });
});
