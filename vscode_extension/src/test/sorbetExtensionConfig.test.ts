import * as assert from "assert";
import * as fs from "fs";
import * as path from "path";
import * as sinon from "sinon";
import * as vscode from "vscode";

import { mockConfiguration } from "./testUtils";
import { SorbetExtensionConfig } from "../sorbetExtensionConfig";
import { SorbetLspConfig } from "../sorbetLspConfig";

suite(`Test Suite: ${path.basename(__filename, ".test.js")}`, () => {
  const expectedWorkspacePath = "/fake/path/to/project";
  const fooLspConfig = new SorbetLspConfig({
    id: "foo",
    name: "FooFoo",
    description: "The foo config",
    // eslint-disable-next-line no-template-curly-in-string
    cwd: "${workspaceFolder}",
    command: ["foo", "on", "you"],
  });
  const barLspConfig = new SorbetLspConfig({
    id: "bar",
    name: "BarBar",
    description: "The bar config",
    // eslint-disable-next-line no-template-curly-in-string
    cwd: "${workspaceFolder}/bar",
    command: ["I", "heart", "bar", "bee", "que"],
  });

  let testRestorables: { restore: () => void }[];
  let extensionContext: vscode.ExtensionContext & {
    backingStore: Map<string, any>;
  };

  setup(() => {
    testRestorables = [];
    const backingStore = new Map<string, any>();
    extensionContext = <any>{
      backingStore,
      workspaceState: <vscode.Memento>{
        get<T>(section: string, defaultValue: T): T {
          return backingStore.has(section)
            ? backingStore.get(section)
            : defaultValue;
        },
        update(section: string, value: any): Thenable<void> {
          backingStore.set(section, value);
          return Promise.resolve();
        },
      },
    };
  });

  teardown(() => {
    testRestorables.forEach((r) => r.restore());
  });

  suite("settings", () => {
    suite("when no sorbet settings", () => {
      test("sorbet is disabled (no workspace open)", () => {
        const configurationStub = mockConfiguration({
          "sorbet.enabled": (defaultValue) => defaultValue,
          "sorbet.lspConfigs": () => [fooLspConfig],
        });
        testRestorables.push(configurationStub);
        const workspaceFoldersStub = sinon
          .stub(vscode.workspace, "workspaceFolders")
          .value(undefined);
        testRestorables.push(workspaceFoldersStub);

        const sorbetConfig = new SorbetExtensionConfig(extensionContext);
        assert.strictEqual(sorbetConfig.enabled, false);
        sinon.assert.calledOnce(configurationStub);

        assert.deepStrictEqual(
          sorbetConfig.getSelectedLspConfig(),
          fooLspConfig,
          "LSP configs should have been set to first default",
        );

        assert.strictEqual(
          sorbetConfig.getActiveLspConfig(),
          undefined,
          "should not have an active LSP config",
        );
      });

      test("sorbet is enabled (sorbet/config exists)", () => {
        const workspaceFoldersStub = sinon
          .stub(vscode.workspace, "workspaceFolders")
          .value([{ uri: { fsPath: expectedWorkspacePath } }]);
        testRestorables.push(workspaceFoldersStub);
        const configurationStub = mockConfiguration({
          "sorbet.enabled": (defaultValue) => defaultValue,
        });
        testRestorables.push(configurationStub);
        const existsSyncStub = sinon.stub(fs, "existsSync").returns(true);
        testRestorables.push(existsSyncStub);

        const sorbetConfig = new SorbetExtensionConfig(extensionContext);
        assert.strictEqual(sorbetConfig.enabled, true, "should be enabled");
        sinon.assert.calledOnce(configurationStub);

        sinon.assert.calledOnceWithExactly(
          existsSyncStub,
          `${expectedWorkspacePath}/sorbet/config`,
        );
      });
    });

    suite("when sorbet settings", () => {
      test("sorbet is enabled (via configuration)", () => {
        const workspaceFoldersStub = sinon
          .stub(vscode.workspace, "workspaceFolders")
          .value([{ uri: { fsPath: expectedWorkspacePath } }]);
        testRestorables.push(workspaceFoldersStub);
        const configurationStub = mockConfiguration({
          "sorbet.enabled": () => true,
        });
        testRestorables.push(configurationStub);
        const existsSyncStub = sinon.stub(fs, "existsSync").returns(false);
        testRestorables.push(existsSyncStub);

        const sorbetConfig = new SorbetExtensionConfig(extensionContext);
        assert.strictEqual(sorbetConfig.enabled, true);

        sinon.assert.calledOnce(configurationStub);
        sinon.assert.calledOnceWithExactly(
          existsSyncStub,
          `${expectedWorkspacePath}/sorbet/config`,
        );
      });

      test("Sorbet LSP configs are populated correctly", () => {
        const expectedConfigs = [fooLspConfig, barLspConfig];
        const configurationStub = mockConfiguration({
          "sorbet.enabled": () => true,
          "sorbet.lspConfigs": () => expectedConfigs,
        });
        testRestorables.push(configurationStub);
        extensionContext.backingStore.set(
          "selectedLspConfigId",
          barLspConfig.id,
        );

        const sorbetConfig = new SorbetExtensionConfig(extensionContext);
        assert.strictEqual(sorbetConfig.enabled, true);

        const configs = sorbetConfig.getLspConfigs();
        assert.strictEqual(configs.length, expectedConfigs.length);

        const selectedConfig = sorbetConfig.getSelectedLspConfig();
        assert.deepStrictEqual(selectedConfig?.id, barLspConfig.id);

        const activeConfig = sorbetConfig.getActiveLspConfig();
        assert.deepStrictEqual(activeConfig, selectedConfig);
      });

      test("Sorbet LSP config selection", async () => {
        const expectedConfigs = [fooLspConfig, barLspConfig];
        const configurationStub = mockConfiguration({
          "sorbet.enabled": () => true,
          "sorbet.lspConfigs": () => expectedConfigs,
        });
        testRestorables.push(configurationStub);

        const sorbetConfig = new SorbetExtensionConfig(extensionContext);
        assert.strictEqual(sorbetConfig.enabled, true);

        // No selection should default to first config
        const firstConfig = sorbetConfig.getSelectedLspConfig();
        assert.deepStrictEqual(firstConfig?.id, expectedConfigs[0].id);
        assert.deepStrictEqual(sorbetConfig.getActiveLspConfig(), firstConfig);

        // No selection and no first fallback means noConfig
        const noConfig = sorbetConfig.getSelectedLspConfig(false);
        assert.ok(!noConfig);

        // Select valid Id
        await sorbetConfig.setSelectedLspConfig(barLspConfig.id);
        const newSelectedConfig = sorbetConfig.getSelectedLspConfig();
        assert.deepStrictEqual(newSelectedConfig, barLspConfig);
        assert.notStrictEqual(newSelectedConfig, firstConfig);

        // Select invalid Id, check behavior with/without fallback
        await sorbetConfig.setSelectedLspConfig("baz");
        assert.deepStrictEqual(
          sorbetConfig.getSelectedLspConfig(),
          firstConfig,
        );
        assert.ok(!sorbetConfig.getSelectedLspConfig(false));
      });

      test("multiple instances of SorbetExtensionConfig stay in sync", async () => {
        const expectedConfigs = [fooLspConfig, barLspConfig];
        const configurationStub = mockConfiguration({
          "sorbet.enabled": () => true,
          "sorbet.lspConfigs": () => expectedConfigs,
        });
        testRestorables.push(configurationStub);
        extensionContext.backingStore.set(
          "selectedLspConfigId",
          fooLspConfig.id,
        );

        const sorbetConfig1 = new SorbetExtensionConfig(extensionContext);
        const sorbetConfig2 = new SorbetExtensionConfig(extensionContext);

        assert.strictEqual(
          sorbetConfig2.getActiveLspConfig()?.id,
          fooLspConfig.id,
          "Precondition: config2 should be set to 'foo'",
        );

        await sorbetConfig1.setActiveLspConfigId(barLspConfig.id);

        assert.strictEqual(
          sorbetConfig2.getActiveLspConfig()?.id,
          barLspConfig.id,
          "changing config1 should also change config2",
        );
      });
    });

    suite("when `sorbet.userLspConfigs` is specified", () => {
      test("when values are distinct from `lspConfigs`", () => {
        const configurationStub = mockConfiguration({
          "sorbet.enabled": () => true,
          "sorbet.lspConfigs": () => [fooLspConfig],
          "sorbet.userLspConfigs": () => [barLspConfig],
        });
        testRestorables.push(configurationStub);

        const sorbetConfig = new SorbetExtensionConfig(extensionContext);

        assert.deepStrictEqual(
          sorbetConfig.getLspConfigs(),
          [barLspConfig, fooLspConfig],
          "items from userLspConfigs should precede items from lspConfigs",
        );
        assert.deepStrictEqual(
          sorbetConfig.getSelectedLspConfig(),
          barLspConfig,
          "First element of userLspConfigs should be the selected/default config",
        );
      });

      test("when values overlap with `lspConfigs`", async () => {
        const userFooLspConfig = new SorbetLspConfig({
          ...barLspConfig,
          id: fooLspConfig.id,
        });
        assert.notDeepStrictEqual(
          userFooLspConfig,
          fooLspConfig,
          "Precondition: userFooLspConfig and foo should be different",
        );
        assert.strictEqual(
          userFooLspConfig.id,
          fooLspConfig.id,
          "Precondition: userFoo and foo should have the same id",
        );

        const configurationStub = mockConfiguration({
          "sorbet.enabled": () => true,
          "sorbet.lspConfigs": () => [fooLspConfig],
          "sorbet.userLspConfigs": () => [userFooLspConfig],
        });
        testRestorables.push(configurationStub);

        const sorbetConfig = new SorbetExtensionConfig(extensionContext);
        assert.deepStrictEqual(
          sorbetConfig.getLspConfigs(),
          [userFooLspConfig],
          "Item from userLspConfigs should override same id from lspConfigs",
        );
        assert.deepStrictEqual(
          sorbetConfig.getSelectedLspConfig(),
          userFooLspConfig,
          "Selected config should be first of userLspConfigs",
        );
      });
    });
  });

  suite("onDidChangeActiveLspConfig event", () => {
    test("fires event when enabling the extension", async () => {
      const configurationStub = mockConfiguration({
        "sorbet.enabled": () => false,
        "sorbet.lspConfigs": () => [fooLspConfig, barLspConfig],
      });
      testRestorables.push(configurationStub);
      extensionContext.backingStore.set("selectedLspConfigId", barLspConfig.id);
      const listener = sinon.spy();

      const sorbetConfig = new SorbetExtensionConfig(extensionContext);
      sorbetConfig.onDidChangeActiveLspConfig(listener);

      await sorbetConfig.setEnabled(true);
      assert.ok(
        listener.called,
        "should have called listener onLspConfigChange",
      );
      assert.deepStrictEqual(
        listener.getCall(0).args[0],
        {
          current: barLspConfig.id,
          previous: undefined,
        },
        "should have transitioned from no config to bar config",
      );
    });

    test("fires event when disabling the extension", async () => {
      const configurationStub = mockConfiguration({
        "sorbet.enabled": () => true,
        "sorbet.lspConfigs": () => [fooLspConfig, barLspConfig],
      });
      testRestorables.push(configurationStub);
      extensionContext.backingStore.set("selectedLspConfigId", barLspConfig.id);
      const listener = sinon.spy();

      const sorbetConfig = new SorbetExtensionConfig(extensionContext);
      sorbetConfig.onDidChangeActiveLspConfig(listener);

      await sorbetConfig.setEnabled(false);
      assert.ok(
        listener.called,
        "should have called listener onLspConfigChange",
      );
      assert.deepStrictEqual(
        listener.getCall(0).args[0],
        {
          current: undefined,
          previous: barLspConfig.id,
        },
        "should have transitioned from bar config to no config",
      );
    });

    test("fires event when changing LSPConfig", async () => {
      const configurationStub = mockConfiguration({
        "sorbet.enabled": () => true,
        "sorbet.lspConfigs": () => [fooLspConfig, barLspConfig],
      });
      testRestorables.push(configurationStub);
      extensionContext.backingStore.set("selectedLspConfigId", barLspConfig.id);
      const listener = sinon.spy();

      const sorbetConfig = new SorbetExtensionConfig(extensionContext);
      sorbetConfig.onDidChangeActiveLspConfig(listener);

      await sorbetConfig.setSelectedLspConfig(fooLspConfig.id);

      assert.ok(listener.called, "should be notified");
      assert.deepStrictEqual(
        listener.getCall(0).args[0],
        {
          current: fooLspConfig.id,
          previous: barLspConfig.id,
        },
        "should have transitioned from bar config to foo config",
      );
    });

    test("does not fire event when unchanged", async () => {
      const configurationStub = mockConfiguration({
        "sorbet.enabled": () => true,
        "sorbet.lspConfigs": () => [fooLspConfig, barLspConfig],
      });
      testRestorables.push(configurationStub);
      extensionContext.backingStore.set("selectedLspConfigId", barLspConfig.id);
      const listener = sinon.spy();

      const sorbetConfig = new SorbetExtensionConfig(extensionContext);
      sorbetConfig.onDidChangeActiveLspConfig(listener);
      assert.strictEqual(sorbetConfig.selectedLspConfigId, barLspConfig.id);

      await sorbetConfig.setSelectedLspConfig(barLspConfig);
      assert.strictEqual(listener.called, false, "should not be notified");
    });

    test("does not fire event when extension disabled", async () => {
      const configurationStub = mockConfiguration({
        "sorbet.enabled": () => false,
        "sorbet.lspConfigs": () => [fooLspConfig, barLspConfig],
      });
      testRestorables.push(configurationStub);
      extensionContext.backingStore.set("selectedLspConfigId", barLspConfig.id);
      const listener = sinon.spy();

      const sorbetConfig = new SorbetExtensionConfig(extensionContext);
      sorbetConfig.onDidChangeActiveLspConfig(listener);

      await sorbetConfig.setSelectedLspConfig(fooLspConfig);
      assert.ok(!listener.called, "should not be notified");
    });

    test("fires when active LSP config is modified", async () => {
      // const modifiedBarLspConfig = new SorbetLspConfig({
      //   ...barLspConfig,
      //   command: ["different", "command", "here"],
      // });

      // const configurationStub = mockConfiguration({
      //   "sorbet.enabled": () => true,
      //   "sorbet.lspConfigs": () => [fooLspConfig, barLspConfig],
      // });
      // testRestorables.push(configurationStub);
      // extensionContext.backingStore.set("selectedLspConfigId", barLspConfig.id);
      // const listener = sinon.spy();
      // const onDidChangeConfigurationStub = sinon.stub(
      //   vscode.workspace,
      //   "onDidChangeConfiguration",
      // );
      // testRestorables.push(onDidChangeConfigurationStub);

      // const sorbetConfig = new SorbetExtensionConfig(extensionContext);
      // sorbetConfig.onDidChangeActiveLspConfig(listener);

      // workspaceConfig.update("lspConfigs", [
      //   fooLspConfig,
      //   modifiedBarLspConfig,
      // ]);
      // assert.strictEqual(listener.called, true, "should be notified");
      // assert.deepStrictEqual(
      //   listener.getCall(0).args[0],
      //   {
      //     oldLspConfig: barLspConfig,
      //     newLspConfig: modifiedBarLspConfig,
      //   },
      //   "should have transitioned from old bar config to modified bar config",
      // );
    });
  });

  // suite(".setSelectedLspConfigId", async () => {
  //

  //   test("does not fire when non-active LSP config is modified", async () => {
  //     const modifiedFooLspConfig = new SorbetLspConfig({
  //       ...fooLspConfig,
  //       command: ["different", "command", "here"],
  //     });
  //     const workspaceConfig = new FakeWorkspaceConfiguration([
  //       ["enabled", true],
  //       ["lspConfigs", [fooLspConfig, barLspConfig]],
  //       ["selectedLspConfigId", barLspConfig.id],
  //     ]);
  //     const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
  //     const listener = sinon.spy();
  //     sorbetConfig.onLspConfigChange(listener);
  //     workspaceConfig.update("lspConfigs", [
  //       modifiedFooLspConfig,
  //       barLspConfig,
  //     ]);
  //     assert.strictEqual(listener.called, false, "should not be notified");
  //   });

  //   test("does not fire when a new (non-active) LSP config is added", async () => {
  //     const workspaceConfig = new FakeWorkspaceConfiguration([
  //       ["enabled", true],
  //       ["lspConfigs", [barLspConfig]],
  //       ["selectedLspConfigId", barLspConfig.id],
  //     ]);
  //     const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
  //     const listener = sinon.spy();
  //     sorbetConfig.onLspConfigChange(listener);
  //     workspaceConfig.update("lspConfigs", [fooLspConfig, barLspConfig]);
  //     assert.strictEqual(listener.called, false, "should not be notified");
  //   });

  //   test("does not fire when a non-active LSP config is removed", async () => {
  //     const workspaceConfig = new FakeWorkspaceConfiguration([
  //       ["enabled", true],
  //       ["lspConfigs", [fooLspConfig, barLspConfig]],
  //       ["selectedLspConfigId", barLspConfig.id],
  //     ]);
  //     const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
  //     const listener = sinon.spy();
  //     sorbetConfig.onLspConfigChange(listener);
  //     workspaceConfig.update("lspConfigs", [barLspConfig]);
  //     assert.strictEqual(
  //       listener.called,
  //       false,
  //       "should not have called listener onLspConfigChange",
  //     );
  //   });

  //   test("fires when the active LSP changes by nature of being the first lspConfig", async () => {
  //     const workspaceConfig = new FakeWorkspaceConfiguration([
  //       ["enabled", true],
  //       ["lspConfigs", [fooLspConfig, barLspConfig]],
  //     ]);
  //     const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
  //     const listener = sinon.spy();
  //     sorbetConfig.onLspConfigChange(listener);
  //     workspaceConfig.update("lspConfigs", [barLspConfig, fooLspConfig]);
  //     assert.strictEqual(
  //       listener.called,
  //       true,
  //       "should have called listener onLspConfigChange",
  //     );
  //     assert.deepStrictEqual(
  //       listener.getCall(0).args[0],
  //       {
  //         oldLspConfig: fooLspConfig,
  //         newLspConfig: barLspConfig,
  //       },
  //       "should have transitioned from foo config to bar config",
  //     );
  //   });
  // });
});
