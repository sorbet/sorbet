import * as path from "path";
import * as sinon from "sinon";
import { commands } from "vscode";

import { createLogStub } from "./testUtils";
import {
  METRIC_PREFIX,
  MetricsClient,
  NoOpApi,
  NoOpMetricsEmitter,
} from "../metricsClient";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("emitCountMetric calls MetricsEmitter.increment once with correct params ", async () => {
    const expectedMetricName = "metricClient.test.emitCountMetric";
    const expectedCount = 123;
    const expectedTags = { foo: "bar" };

    const incrementStub = sinon.stub(NoOpMetricsEmitter.prototype, "increment");
    testRestorables.push(incrementStub);
    const client = new MetricsClient(
      <SorbetExtensionContext>{
        configuration: {
          activeLspConfig: undefined,
        },
        log: createLogStub(),
      },
      new NoOpApi(),
    );

    await client.emitCountMetric(
      expectedMetricName,
      expectedCount,
      expectedTags,
    );

    sinon.assert.calledTwice(incrementStub);
    sinon.assert.calledWithMatch(
      incrementStub.secondCall,
      `${METRIC_PREFIX}${expectedMetricName}`,
      expectedCount,
      expectedTags,
    );
  });

  test("emitTimingMetric calls MetricsEmitter.timing once with correct params ", async () => {
    const expectedMetricName = "metricClient.test.emitCountMetric";
    const expectedCount = 123;
    const expectedTags = { foo: "bar" };

    const timingStub = sinon.stub(NoOpMetricsEmitter.prototype, "timing");
    testRestorables.push(timingStub);
    const client = new MetricsClient(
      <SorbetExtensionContext>{
        configuration: {
          activeLspConfig: undefined,
        },
        log: createLogStub(),
      },
      new NoOpApi(),
    );

    await client.emitTimingMetric(
      expectedMetricName,
      expectedCount,
      expectedTags,
    );

    sinon.assert.calledOnce(timingStub);
    sinon.assert.calledWithMatch(
      timingStub,
      `${METRIC_PREFIX}${expectedMetricName}`,
      expectedCount,
      expectedTags,
    );
  });

  test("logs at info level, not error, when the metrics command is not registered", async () => {
    const executeCommandStub = sinon
      .stub(commands, "executeCommand")
      .rejects(new Error("command 'sorbet.metrics.getExportedApi' not found"));
    testRestorables.push(executeCommandStub);

    const log = createLogStub();
    const infoSpy = sinon.spy(log, "info");
    const errorSpy = sinon.spy(log, "error");
    testRestorables.push(infoSpy, errorSpy);

    const client = new MetricsClient(<SorbetExtensionContext>{
      configuration: { activeLspConfig: undefined },
      log,
    });
    // `initSorbetMetricsApi` isn't awaited by the constructor, so wait on
    // the same promise indirectly via a metric emission.
    await client.emitCountMetric("metricClient.test.notRegistered", 1);

    sinon.assert.calledOnce(infoSpy);
    sinon.assert.notCalled(errorSpy);
  });
});
