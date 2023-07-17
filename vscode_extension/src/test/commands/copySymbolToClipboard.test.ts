import * as vscode from "vscode";
import * as vsclc from "vscode-languageclient/node";
import * as assert from "assert";
import * as path from "path";
import * as sinon from "sinon";

import { createLogStub } from "../testUtils";
import { copySymbolToClipboard } from "../../commands/copySymbolToClipboard";
import { SorbetLanguageClient } from "../../languageClient";
import { LogLevel } from "../../log";
import { SorbetExtensionContext } from "../../sorbetExtensionContext";
import { SorbetStatusProvider } from "../../sorbetStatusProvider";
import { ServerStatus } from "../../types";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  let testRestorables: { restore: () => void }[];

  setup(() => {
    testRestorables = [];
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  test("copySymbolToClipboard: does nothing if client is not present", async () => {
    const writeTextSpy = sinon.spy(() => assert.fail());
    const envClipboardStub = sinon.stub(vscode, "env").value(<any>{
      clipboard: <any>{
        writeText: writeTextSpy,
      },
    });
    testRestorables.push(envClipboardStub);

    const statusProvider = <SorbetStatusProvider>{
      activeLanguageClient: undefined,
    };
    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
      statusProvider,
    };
    await copySymbolToClipboard(context);

    sinon.assert.notCalled(writeTextSpy);
  });

  test("copySymbolToClipboard: does nothing if client is not ready", async () => {
    const writeTextSpy = sinon.spy(() => assert.fail());
    const envClipboardStub = sinon.stub(vscode, "env").value(<any>{
      clipboard: <any>{
        writeText: writeTextSpy,
      },
    });
    testRestorables.push(envClipboardStub);

    const statusProvider = <SorbetStatusProvider>{
      activeLanguageClient: <SorbetLanguageClient>{
        status: ServerStatus.DISABLED,
      },
    };
    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
      statusProvider,
    };
    await copySymbolToClipboard(context);

    sinon.assert.notCalled(writeTextSpy);
  });

  test("copySymbolToClipboard: does nothing if client does not support `sorbetShowSymbolProvider`", async () => {
    const writeTextSpy = sinon.spy(() => assert.fail());
    const envClipboardStub = sinon.stub(vscode, "env").value(<any>{
      clipboard: <any>{
        writeText: writeTextSpy,
      },
    });
    testRestorables.push(envClipboardStub);

    const statusProvider = <SorbetStatusProvider>{
      activeLanguageClient: <SorbetLanguageClient>{
        capabilities: {
          sorbetShowSymbolProvider: false,
        },
        status: ServerStatus.RUNNING,
      },
    };

    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
      statusProvider,
    };
    await copySymbolToClipboard(context);

    sinon.assert.notCalled(writeTextSpy);
  });

  test("copySymbolToClipboard: copies symbol to clipboard whne there is a valid selection", async () => {
    const expectedUri = vscode.Uri.parse("file://workspace/test.rb");
    const expectedSymbolName = "test_symbol_name";

    const writeTextSpy = sinon.spy((value: String) =>
      assert.strictEqual(value, expectedSymbolName),
    );
    const envClipboardStub = sinon.stub(vscode, "env").value(<any>{
      clipboard: <any>{
        writeText: writeTextSpy,
      },
    });
    testRestorables.push(envClipboardStub);

    const activeTextEditorStub = sinon
      .stub(vscode.window, "activeTextEditor")
      .get(
        () =>
          <vscode.TextEditor>{
            document: { uri: expectedUri },
            selection: new vscode.Selection(1, 1, 1, 1),
          },
      );
    testRestorables.push(activeTextEditorStub);
    testRestorables.push(activeTextEditorStub);
    const sendRequestSpy = sinon.spy(
      (_method: string, _param: vsclc.TextDocumentPositionParams) =>
        <vsclc.SymbolInformation>{
          name: expectedSymbolName,
        },
    );

    const statusProvider = <SorbetStatusProvider>{
      activeLanguageClient: <SorbetLanguageClient>{
        capabilities: {
          sorbetShowSymbolProvider: true,
        },
        sendRequest: <any>sendRequestSpy,
        status: ServerStatus.RUNNING,
      },
    };
    const context = <SorbetExtensionContext>{
      log: createLogStub(LogLevel.Info),
      statusProvider,
    };
    await copySymbolToClipboard(context);

    sinon.assert.calledOnce(writeTextSpy);
    sinon.assert.calledWith(writeTextSpy, expectedSymbolName);
  });
});
