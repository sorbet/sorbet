import * as vscode from "vscode";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import { showSorbetConfigurationPicker } from "../../commands/showSorbetConfigurationPicker";
import { SorbetExtensionConfig, SorbetLspConfig } from "../../config";
import { SorbetExtensionContext } from "../../sorbetExtensionContext";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("showSorbetConfigurationPicker: Shows dropdown (no-selection)", async () => {
    const activeLspConfig = new SorbetLspConfig({
      id: "test-config-id-active",
      name: "test-config-id-active",
      description: "",
      cwd: "",
      command: [],
    });
    const otherLspConfig = new SorbetLspConfig({
      id: "test-config-id",
      name: "test-config-id",
      description: "",
      cwd: "",
      command: [],
    });

    const showQuickPickSingleStub = sinon
      .stub(vscode.window, "showQuickPick")
      .resolves(undefined); // User canceled
    testRestorables.push(showQuickPickSingleStub);

    const log = createLogStub();
    const configuration = <SorbetExtensionConfig>(<unknown>{
      activeLspConfig,
      lspConfigs: [activeLspConfig, otherLspConfig],
    });
    const context = <SorbetExtensionContext>{ log, configuration };

    await assert.doesNotReject(showSorbetConfigurationPicker(context));

    sinon.assert.calledOnce(showQuickPickSingleStub);
    assert.deepStrictEqual(await showQuickPickSingleStub.firstCall.args[0], [
      {
        label: `• ${activeLspConfig.name}`,
        description: activeLspConfig.description,
        detail: activeLspConfig.command.join(" "),
        lspConfig: activeLspConfig,
      },
      {
        label: otherLspConfig.name,
        description: otherLspConfig.description,
        detail: otherLspConfig.command.join(" "),
        lspConfig: otherLspConfig,
      },
      {
        label: "Disable Sorbet",
        description: "Disable the Sorbet extension",
      },
    ]);
    assert.deepStrictEqual(await showQuickPickSingleStub.firstCall.args[1], {
      placeHolder: "Select a Sorbet configuration",
    });
  });
});
