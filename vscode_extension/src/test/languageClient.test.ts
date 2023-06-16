import {
  LanguageClient,
  LanguageClientOptions,
  ServerOptions,
  TransportKind,
} from "vscode-languageclient/node";
import { RequestType } from "vscode-languageserver-protocol";
import * as assert from "assert";
import { shimLanguageClient } from "../languageClient";
import { TestLanguageServerSpecialURIs } from "./testLanguageServerSpecialURIs";
import { MetricsEmitter, Tags } from "../metricsClient";

const enum MetricType {
  Increment,
  Gauge,
  Timing,
}

class RecordingMetricsEmitter implements MetricsEmitter {
  private metrics: [MetricType, string, number, Tags][] = [];

  getAndResetMetrics(): [MetricType, string, number, Tags][] {
    const rv = this.metrics;
    this.metrics = [];
    return rv;
  }

  async increment(
    metricName: string,
    count: number = 1,
    tags: Readonly<{ [metric: string]: string }> = {},
  ): Promise<void> {
    this.metrics.push([MetricType.Increment, metricName, count, tags]);
  }

  async gauge(
    metricName: string,
    value: number,
    tags: Readonly<{ [metric: string]: string }> = {},
  ): Promise<void> {
    this.metrics.push([MetricType.Gauge, metricName, value, tags]);
  }

  async timing(
    metricName: string,
    value: number | Date,
    tags: Tags = {},
  ): Promise<void> {
    const rawValue =
      typeof value === "number" ? value : Date.now() - value.valueOf();
    this.metrics.push([MetricType.Timing, metricName, rawValue, tags]);
  }

  async flush(): Promise<void> {
    // No-op
  }
}

function createLanguageClient(): LanguageClient {
  // The server is implemented in node
  const serverModule = require.resolve("./testLanguageServer");
  // The debug options for the server
  const debugOptions = { execArgv: [] };

  // If the extension is launched in debug mode then the debug server options are used
  // Otherwise the run options are used
  const serverOptions: ServerOptions = {
    run: { module: serverModule, transport: TransportKind.ipc },
    debug: {
      module: serverModule,
      transport: TransportKind.ipc,
      options: debugOptions,
    },
  };

  // Options to control the language client
  const clientOptions: LanguageClientOptions = {
    // Register the server for plain text documents
    documentSelector: [{ scheme: "file", language: "plaintext" }],
    synchronize: {},
  };

  // Create the language client and start the client.
  const client = new LanguageClient(
    "languageServerExample",
    "Language Server Example",
    serverOptions,
    clientOptions,
  );

  // Start the client. This will also launch the server
  client.start();
  return client;
}

let metricsEmitter = new RecordingMetricsEmitter();
suite("LanguageClient", () => {
  suite("Metrics", () => {
    suiteSetup(() => {
      metricsEmitter = new RecordingMetricsEmitter();
    });
    test("Shims language clients and records latency metrics", async () => {
      const client = shimLanguageClient(
        createLanguageClient(),
        metricsEmitter.timing.bind(metricsEmitter),
      );
      await client.onReady();
      {
        const successResponse = await client.sendRequest("textDocument/hover", {
          textDocument: {
            uri: TestLanguageServerSpecialURIs.SUCCESS,
          },
          position: { line: 1, character: 1 },
        });
        assert.equal(
          (successResponse as any).contents,
          TestLanguageServerSpecialURIs.SUCCESS,
        );
        const metrics = metricsEmitter.getAndResetMetrics();
        assert.equal(metrics.length, 1);
        const m = metrics[0];
        assert.equal(m[0], MetricType.Timing);
        assert.equal(m[1], `latency.textDocument_hover_ms`);
        assert.equal(m[3].success, "true");
      }

      {
        const successResponse = await client.sendRequest(
          new RequestType("textDocument/hover"),
          {
            textDocument: {
              uri: TestLanguageServerSpecialURIs.SUCCESS,
            },
            position: { line: 1, character: 1 },
          },
        );
        assert.equal(
          (successResponse as any).contents,
          TestLanguageServerSpecialURIs.SUCCESS,
        );
        const metrics = metricsEmitter.getAndResetMetrics();
        assert.equal(metrics.length, 1);
        const m = metrics[0];
        assert.equal(m[0], MetricType.Timing);
        assert.equal(m[1], `latency.textDocument_hover_ms`);
        assert.equal(m[3].success, "true");
      }

      try {
        await client.sendRequest("textDocument/hover", {
          textDocument: {
            uri: TestLanguageServerSpecialURIs.FAILURE,
          },
          position: { line: 1, character: 1 },
        });
        assert.fail("Request should have failed.");
      } catch (e) {
        assert(
          ((e as any).message as string).indexOf(
            TestLanguageServerSpecialURIs.FAILURE,
          ) !== -1,
        );
        const metrics = metricsEmitter.getAndResetMetrics();
        assert.equal(metrics.length, 1);
        const m = metrics[0];
        assert.equal(m[0], MetricType.Timing);
        assert.equal(m[1], `latency.textDocument_hover_ms`);
        assert.equal(m[3].success, "false");
      }

      try {
        await client.sendRequest("textDocument/hover", {
          textDocument: {
            uri: TestLanguageServerSpecialURIs.EXIT,
          },
          position: { line: 1, character: 1 },
        });
        assert.fail("Request should have failed.");
      } catch (e) {
        const metrics = metricsEmitter.getAndResetMetrics();
        assert.equal(metrics.length, 1);
        const m = metrics[0];
        assert.equal(m[0], MetricType.Timing);
        assert.equal(m[1], `latency.textDocument_hover_ms`);
        assert.equal(m[3].success, "false");
      }
    });
  });
});
