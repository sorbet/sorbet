import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import { toggleTypedFalseCompletionNudges } from "../../commands/toggleTypedFalseCompletionNudges";
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

  test("toggleTypedFalseCompletionNudges", async () => {
    const initialState = true;
    let currentState = initialState;

    const log = createLogStub();

    const setTypedFalseCompletionNudgesSpy = sinon.spy((value: boolean) => {
      currentState = value;
    });
    const configuration = <SorbetExtensionConfig>(<unknown>{
      get typedFalseCompletionNudges() {
        return currentState;
      },
      setTypedFalseCompletionNudges: setTypedFalseCompletionNudgesSpy,
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
      await toggleTypedFalseCompletionNudges(context),
      !initialState,
    );

    sinon.assert.calledOnce(setTypedFalseCompletionNudgesSpy);
    sinon.assert.calledWithExactly(
      setTypedFalseCompletionNudgesSpy,
      !initialState,
    );

    sinon.assert.calledOnce(restartSorbetSpy);
    sinon.assert.calledWithExactly(
      restartSorbetSpy,
      RestartReason.CONFIG_CHANGE,
    );
  });
});
