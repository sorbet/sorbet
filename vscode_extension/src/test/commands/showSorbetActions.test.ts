import * as vscode from "vscode";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import {
  Action,
  getAvailableActions,
  showSorbetActions,
} from "../../commands/showSorbetActions";
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

  test("getAvailableActions", () => {
    assert.deepStrictEqual(getAvailableActions(ServerStatus.DISABLED), [
      Action.ViewOutput,
      Action.EnableSorbet,
      Action.ConfigureSorbet,
    ]);

    assert.deepStrictEqual(getAvailableActions(ServerStatus.ERROR), [
      Action.ViewOutput,
      Action.RestartSorbet,
      Action.ConfigureSorbet,
    ]);

    assert.deepStrictEqual(getAvailableActions(ServerStatus.INITIALIZING), [
      Action.ViewOutput,
      Action.RestartSorbet,
      Action.DisableSorbet,
      Action.ConfigureSorbet,
    ]);

    assert.deepStrictEqual(getAvailableActions(ServerStatus.RESTARTING), [
      Action.ViewOutput,
      Action.RestartSorbet,
      Action.DisableSorbet,
      Action.ConfigureSorbet,
    ]);

    assert.deepStrictEqual(getAvailableActions(ServerStatus.RUNNING), [
      Action.ViewOutput,
      Action.RestartSorbet,
      Action.DisableSorbet,
      Action.ConfigureSorbet,
    ]);
  });

  test("showSorbetActions: Shows dropdown (no-selection)", async () => {
    const showQuickPickSingleStub = sinon
      .stub(vscode.window, "showQuickPick")
      .resolves(undefined); // User canceled
    testRestorables.push(showQuickPickSingleStub);

    const log = createLogStub();
    const statusProvider = <SorbetStatusProvider>{
      serverStatus: ServerStatus.RUNNING,
    };
    const context = <SorbetExtensionContext>{ log, statusProvider };

    await assert.doesNotReject(showSorbetActions(context));

    sinon.assert.calledOnce(showQuickPickSingleStub);
    assert.deepStrictEqual(await showQuickPickSingleStub.firstCall.args[0], [
      Action.ViewOutput,
      Action.RestartSorbet,
      Action.DisableSorbet,
      Action.ConfigureSorbet,
    ]);
    assert.deepStrictEqual(await showQuickPickSingleStub.firstCall.args[1], {
      placeHolder: "Select an action",
    });
  });
});
