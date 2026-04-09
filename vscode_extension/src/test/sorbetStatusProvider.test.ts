import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { SorbetStatusProvider } from "../sorbetStatusProvider";
import { RestartReason, ServerStatus } from "../types";
import { createLogStub } from "./testUtils";

function createDeferredPromise(): {
  promise: Promise<void>;
  resolve: () => void;
} {
  let resolve = () => {};
  const promise = new Promise<void>((res) => {
    resolve = res;
  });
  return { promise, resolve };
}

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  teardown(() => {
    sinon.restore();
  });

  test("restartSorbet waits for the active client to stop before starting", async () => {
    const context = <SorbetExtensionContext>(<unknown>{
      log: createLogStub(),
      metrics: {
        emitCountMetric: sinon.stub().resolves(),
      },
    });
    const provider = new SorbetStatusProvider(context);
    const startSorbetStub = sinon.stub(provider, "startSorbet").resolves();
    const stopped = createDeferredPromise();
    const fakeClient = <any>{
      stop: sinon.stub().returns(stopped.promise),
      status: ServerStatus.RUNNING,
    };

    (<any>provider).wrappedActiveLanguageClient = fakeClient;
    (<any>provider).disposables.push(fakeClient);

    const restartPromise = provider.restartSorbet(RestartReason.CONFIG_CHANGE);
    await Promise.resolve();

    sinon.assert.calledOnce(fakeClient.stop);
    sinon.assert.notCalled(startSorbetStub);
    assert.strictEqual(provider.activeLanguageClient, undefined);
    assert.strictEqual(provider.serverStatus, ServerStatus.STOPPING);
    assert.ok(!(<any>provider).disposables.includes(fakeClient));

    stopped.resolve();
    await restartPromise;

    sinon.assert.calledOnce(startSorbetStub);
  });
});
