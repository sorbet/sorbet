import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import { toggleUntypedCodeHighlighting } from "../../commands/toggleUntypedCodeHighlighting";
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

  test("toggleUntypedCodeHighlighting", async () => {
    const initialState = true;
    let currentState = initialState;

    const log = createLogStub();

    const setHighlightUntypedSpy = sinon.spy((value: boolean) => {
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

    assert.strictEqual(
      await toggleUntypedCodeHighlighting(context),
      !initialState,
    );

    sinon.assert.calledOnce(setHighlightUntypedSpy);
    sinon.assert.calledWithExactly(setHighlightUntypedSpy, !initialState);

    sinon.assert.calledOnce(restartSorbetSpy);
    sinon.assert.calledWithExactly(
      restartSorbetSpy,
      RestartReason.CONFIG_CHANGE,
    );
  });
});
