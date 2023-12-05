import * as vscode from "vscode";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import { showSorbetConfigurationPicker } from "../../commands/showSorbetConfigurationPicker";
import { SorbetExtensionContext } from "../../sorbetExtensionContext";
import { SorbetExtensionConfig } from "../../sorbetExtensionConfig";
import { SorbetLspConfig } from "../../sorbetLspConfig";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("showSorbetConfigurationPicker: Shows dropdown (no-selection)", async () => {
    const activeLspConfig = new SorbetLspConfig(
      "test-config-id-active",
      "test-config-name-active",
    );
    const otherLspConfig = new SorbetLspConfig(
      "test-config-id",
      "test-config-name",
    );

    const showQuickPickSingleStub = sinon
      .stub(vscode.window, "showQuickPick")
      .resolves(undefined); // User canceled
    testRestorables.push(showQuickPickSingleStub);

    const log = createLogStub();
    const configuration = <SorbetExtensionConfig>(<unknown>{
      getActiveLspConfig: () => activeLspConfig,
      getLspConfigs: () => [activeLspConfig, otherLspConfig],
    });
    const context = <SorbetExtensionContext>{ log, configuration };

    await assert.doesNotReject(showSorbetConfigurationPicker(context));

    sinon.assert.calledOnce(showQuickPickSingleStub);
    assert.deepStrictEqual(showQuickPickSingleStub.firstCall.args[0], [
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
    assert.deepStrictEqual(showQuickPickSingleStub.firstCall.args[1], {
      placeHolder: "Select a Sorbet configuration",
    });
  });
});
