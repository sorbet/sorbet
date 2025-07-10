import * as assert from "assert";
import * as vscode from "vscode";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import {
  configureUntypedCodeHighlightingDiagnosticSeverity,
  DiagnosticSeverityQuickPickItem,
} from "../../commands/configureUntypedCodeHighlightingDiagnosticSeverity";
import { SorbetExtensionConfig } from "../../config";
import { SorbetExtensionContext } from "../../sorbetExtensionContext";
import { SorbetStatusProvider } from "../../sorbetStatusProvider";
import { RestartReason } from "../../types";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("configureUntypedCodeHighlightingDiagnosticSeverity", async () => {
    const initialState = vscode.DiagnosticSeverity.Information;
    let currentState = initialState;

    const log = createLogStub();

    const setHighlightUntypedDiagnosticSeveritySpy = sinon.spy(
      (value: vscode.DiagnosticSeverity) => {
        currentState = value;
      },
    );
    const configuration = <SorbetExtensionConfig>(<unknown>{
      get highlightUntypedDiagnosticSeverity() {
        return currentState;
      },
      setHighlightUntypedDiagnosticSeverity: setHighlightUntypedDiagnosticSeveritySpy,
    });

    const restartSorbetSpy = sinon.spy((_reason: RestartReason) => {});
    const statusProvider = <SorbetStatusProvider>(<unknown>{
      restartSorbet: restartSorbetSpy,
    });

    const context = <SorbetExtensionContext>{
      log,
      configuration,
      statusProvider,
    };

    const expectedSeverityStr = "Warning";
    const expectedSeverity = vscode.DiagnosticSeverity[expectedSeverityStr] + 1;
    const showQuickPickSingleStub = sinon
      .stub(vscode.window, "showQuickPick")
      .resolves(<DiagnosticSeverityQuickPickItem>{
        label: expectedSeverityStr,
        severity: expectedSeverity,
      });
    testRestorables.push(showQuickPickSingleStub);

    assert.strictEqual(
      await configureUntypedCodeHighlightingDiagnosticSeverity(context),
      expectedSeverity,
    );
    sinon.assert.calledOnce(setHighlightUntypedDiagnosticSeveritySpy);
    sinon.assert.calledWithExactly(
      setHighlightUntypedDiagnosticSeveritySpy,
      expectedSeverity,
    );
  });
});
