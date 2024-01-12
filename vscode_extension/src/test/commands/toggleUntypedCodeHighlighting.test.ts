import * as assert from "assert";
import * as vscode from "vscode";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import {
  toggleUntypedCodeHighlighting,
  configureUntypedCodeHighlighting,
  TrackUntypedQuickPickItem,
} from "../../commands/toggleUntypedCodeHighlighting";
import {
  SorbetExtensionConfig,
  TrackUntyped,
  labelForTrackUntypedSetting,
} from "../../config";
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

  test("toggleUntypedCodeHighlighting", async () => {
    const initialState = "everywhere";
    let currentState = initialState;

    const log = createLogStub();

    const setHighlightUntypedSpy = sinon.spy((value: TrackUntyped) => {
      currentState = value;
    });
    const configuration = <SorbetExtensionConfig>(<unknown>{
      get highlightUntyped() {
        return currentState;
      },
      setHighlightUntyped: setHighlightUntypedSpy,
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

    assert.strictEqual(await toggleUntypedCodeHighlighting(context), "nowhere");

    sinon.assert.calledOnce(setHighlightUntypedSpy);
    sinon.assert.calledWithExactly(setHighlightUntypedSpy, "nowhere");
  });

  test("configureUntypedCodeHighlighting", async () => {
    const initialState = "everywhere";
    let currentState = initialState;

    const log = createLogStub();

    const setHighlightUntypedSpy = sinon.spy((value: TrackUntyped) => {
      currentState = value;
    });
    const configuration = <SorbetExtensionConfig>(<unknown>{
      get highlightUntyped() {
        return currentState;
      },
      setHighlightUntyped: setHighlightUntypedSpy,
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

    const expectedTrackWhere = "everywhere-but-tests";
    const showQuickPickSingleStub = sinon
      .stub(vscode.window, "showQuickPick")
      .resolves(<TrackUntypedQuickPickItem>{
        label: labelForTrackUntypedSetting(expectedTrackWhere),
        trackWhere: expectedTrackWhere,
      });
    testRestorables.push(showQuickPickSingleStub);

    assert.strictEqual(
      await configureUntypedCodeHighlighting(context),
      expectedTrackWhere,
    );
    sinon.assert.calledOnce(setHighlightUntypedSpy);
    sinon.assert.calledWithExactly(setHighlightUntypedSpy, expectedTrackWhere);
  });

  test("toggle is sticky", async () => {
    const initialState = "everywhere-but-tests";
    let currentState = initialState;

    const log = createLogStub();

    const setHighlightUntypedSpy = sinon.spy((value: TrackUntyped) => {
      currentState = value;
    });
    const configuration = <SorbetExtensionConfig>(<unknown>{
      get highlightUntyped() {
        return currentState;
      },
      setHighlightUntyped: setHighlightUntypedSpy,
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

    assert.strictEqual(await toggleUntypedCodeHighlighting(context), "nowhere");
    assert.strictEqual(
      await toggleUntypedCodeHighlighting(context),
      initialState,
    );

    sinon.assert.calledTwice(setHighlightUntypedSpy);
    sinon.assert.calledWithExactly(
      setHighlightUntypedSpy.getCall(0),
      "nowhere",
    );
    sinon.assert.calledWithExactly(
      setHighlightUntypedSpy.getCall(1),
      initialState,
    );
  });
});
