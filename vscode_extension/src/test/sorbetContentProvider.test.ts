import * as vscode from "vscode";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "./testUtils";
import { SorbetLanguageClient } from "../languageClient";
import { LogLevel } from "../log";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { SorbetContentProvider } from "../sorbetContentProvider";
import { SorbetStatusProvider } from "../sorbetStatusProvider";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("provideTextDocumentContent succeeds", async () => {
    const fileUri = vscode.Uri.parse("sorbet:/test/file", true);
    const expectedContents = "";

    const sendRequestSpy = sinon.spy(async (_method, _params) => ({
      text: expectedContents,
    }));
    const statusProvider = <SorbetStatusProvider>{
      activeLanguageClient: <SorbetLanguageClient>(<unknown>{
        sendRequest: sendRequestSpy,
      }),
    };
    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
      statusProvider,
    };

    const provider = new SorbetContentProvider(context);
    assert.strictEqual(
      await provider.provideTextDocumentContent(fileUri),
      expectedContents,
    );

    sinon.assert.calledOnce(sendRequestSpy);
    sinon.assert.calledWith(sendRequestSpy, "sorbet/readFile", {
      uri: fileUri.toString(),
    });
  });

  test("provideTextDocumentContent handles no activeLanguageClient", async () => {
    const fileUri = vscode.Uri.parse("sorbet:/test/file", true);
    const statusProvider = <SorbetStatusProvider>{};
    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
      statusProvider,
    };

    const provider = new SorbetContentProvider(context);
    assert.strictEqual(await provider.provideTextDocumentContent(fileUri), "");
  });
});
