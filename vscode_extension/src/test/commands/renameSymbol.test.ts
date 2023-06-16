import * as vscode from "vscode";
import * as vsclc from "vscode-languageclient";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import { renameSymbol } from "../../commands/renameSymbol";
import { LogLevel } from "../../log";
import { SorbetExtensionContext } from "../../sorbetExtensionContext";
import { SorbetStatusProvider } from "../../sorbetStatusProvider";
import { ServerStatus } from "../../types";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("renameSymbol: does nothing if client is not ready", async () => {
    const executeStub = sinon.stub(vscode.commands, "executeCommand");
    testRestorables.push(executeStub);

    const statusProvider = <SorbetStatusProvider>{
      serverStatus: ServerStatus.ERROR,
    };
    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
      statusProvider,
    };
    await renameSymbol(context, <vsclc.TextDocumentPositionParams>{});

    sinon.assert.notCalled(executeStub);
  });

  test("renameSymbol: invokes `editor.action.rename`", async () => {
    const expectedLine = 77;
    const expectedCharacter = 99;
    const expectedUri = vscode.Uri.parse("file://workspace/test.rb");

    const executeStub = sinon
      .stub(vscode.commands, "executeCommand")
      .resolves();
    testRestorables.push(executeStub);

    const statusProvider = <SorbetStatusProvider>{
      serverStatus: ServerStatus.RUNNING,
    };
    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
      statusProvider,
    };
    await renameSymbol(context, <vsclc.TextDocumentPositionParams>{
      textDocument: { uri: expectedUri.toString() },
      position: { line: expectedLine, character: expectedCharacter },
    });

    sinon.assert.calledOnce(executeStub);
    const [commandName, [uri, position]] = executeStub.firstCall.args;
    assert.strictEqual(commandName, "editor.action.rename");
    assert.strictEqual(uri.toString(), expectedUri.toString());
    assert.strictEqual(position.line, expectedLine);
    assert.strictEqual(position.character, expectedCharacter);
  });
});
