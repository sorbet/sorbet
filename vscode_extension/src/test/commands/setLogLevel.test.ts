import * as vscode from "vscode";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { LogLevelQuickPickItem, SetLogLevel } from "../../commands/setLogLevel";
import { LogLevel, OutputChannelLog } from "../../log";
import { SorbetExtensionContext } from "../../sorbetExtensionContext";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("Shows dropdown when target-level argument is NOT provided", async () => {
    const expectedLogLevel = LogLevel.Warning;
    const createOutputChannelStub = sinon
      .stub(vscode.window, "createOutputChannel")
      .returns(<vscode.OutputChannel>{
        appendLine(_value: string) {},
      });
    testRestorables.push(createOutputChannelStub);

    const showQuickPickSingleStub = sinon
      .stub(vscode.window, "showQuickPick")
      .resolves(<LogLevelQuickPickItem>{
        label: LogLevel[expectedLogLevel],
        level: expectedLogLevel,
      });
    testRestorables.push(showQuickPickSingleStub);

    const log = new OutputChannelLog("Test", LogLevel.Info);
    const context = <SorbetExtensionContext>{ log };

    const command = new SetLogLevel(context);
    await assert.doesNotReject(command.execute());
    assert.strictEqual(log.level, expectedLogLevel);

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
    sinon.assert.calledOnce(createOutputChannelStub);
  });

  test("Shows no-dropdown when target-level argument is provided ", async () => {
    const expectedLogLevel = LogLevel.Warning;
    const createOutputChannelStub = sinon
      .stub(vscode.window, "createOutputChannel")
      .returns(<vscode.OutputChannel>{
        appendLine(_value: string) {},
      });
    testRestorables.push(createOutputChannelStub);

    const showQuickPickSingleStub = sinon.stub(vscode.window, "showQuickPick");
    testRestorables.push(showQuickPickSingleStub);

    const log = new OutputChannelLog("Test", LogLevel.Info);
    const context = <SorbetExtensionContext>{ log };

    const command = new SetLogLevel(context);
    await assert.doesNotReject(command.execute(expectedLogLevel));
    assert.strictEqual(log.level, expectedLogLevel);

    sinon.assert.notCalled(showQuickPickSingleStub);
    sinon.assert.calledOnce(createOutputChannelStub);
  });
});
