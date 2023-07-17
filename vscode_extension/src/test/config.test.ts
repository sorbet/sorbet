import * as assert from "assert";
import * as sinon from "sinon";
import {
  EventEmitter,
  ConfigurationTarget,
  ConfigurationChangeEvent,
  Uri,
  WorkspaceFolder,
  extensions,
} from "vscode";

import * as fs from "fs";
import {
  SorbetExtensionConfig,
  SorbetLspConfig,
  ISorbetWorkspaceContext,
} from "../config";

// Helpers

/** Imitate the WorkspaceConfiguration. */
class FakeWorkspaceConfiguration implements ISorbetWorkspaceContext {
  public readonly backingStore: Map<String, any>;
  public readonly defaults: Map<String, any>;
  private readonly configurationChangeEmitter: EventEmitter<
    ConfigurationChangeEvent
  >;

  constructor(properties: Iterable<[String, any]> = []) {
    this.backingStore = new Map<String, any>(properties);
    this.configurationChangeEmitter = new EventEmitter<
      ConfigurationChangeEvent
    >();
    const defaultProperties = extensions.getExtension(
      "sorbet.sorbet-vscode-extension",
    )!.packageJSON.contributes.configuration.properties;
    const defaultValues: Iterable<[String, any]> = Object.keys(
      defaultProperties,
    ).map((settingName) => {
      let value = defaultProperties[settingName].default;

      if (
        defaultProperties[settingName].type === "boolean" &&
        value === undefined
      ) {
        value = false;
      }

      return [settingName.replace("sorbet.", ""), value];
    });
    this.defaults = new Map<String, any>(defaultValues);
  }

  dispose() {}

  get<T>(section: string, defaultValue: T): T {
    if (this.backingStore.has(section)) {
      return this.backingStore.get(section);
    } else if (this.defaults.has(section)) {
      return this.defaults.get(section);
    } else {
      return defaultValue;
    }
  }

  update(
    section: string,
    value: any,
    configurationTarget?: boolean | ConfigurationTarget | undefined,
  ): Thenable<void> {
    if (configurationTarget) {
      assert.fail(
        `fake does not (yet) support ConfigurationTarget, given: ${configurationTarget}`,
      );
    }
    this.backingStore.set(section, value);
    return Promise.resolve(
      this.configurationChangeEmitter.fire({
        affectsConfiguration: (s: string, _?: Uri) => {
          return section.startsWith(`${s}.`);
        },
      }),
    );
  }

  get onDidChangeConfiguration() {
    return this.configurationChangeEmitter.event;
  }

  workspaceFolders() {
    return [{ uri: { fsPath: "/fake/path/to/project" } }] as WorkspaceFolder[];
  }

  initializeEnabled(enabled: boolean): void {
    const stateEnabled = this.backingStore.get("enabled");

    if (stateEnabled === undefined) {
      this.update("enabled", enabled);
    }
  }
}

const fooLspConfig = new SorbetLspConfig({
  id: "foo",
  name: "FooFoo",
  description: "The foo config",
  cwd: "${workspaceFolder}", // eslint-disable-line no-template-curly-in-string
  command: ["foo", "on", "you"],
});

const barLspConfig = new SorbetLspConfig({
  id: "bar",
  name: "BarBar",
  description: "The bar config",
  cwd: "${workspaceFolder}/bar", // eslint-disable-line no-template-curly-in-string
  command: ["I", "heart", "bar", "bee", "que"],
});

suite("SorbetLspConfig", () => {
  test("modifications to ctor arg should not modify object", () => {
    const ctorArg = {
      id: "one",
      name: "two",
      description: "three",
      cwd: "four",
      command: ["five", "six"],
    };
    const lspConfig = new SorbetLspConfig(ctorArg);
    ctorArg.id = "modified";
    ctorArg.name = "modified";
    ctorArg.description = "modified";
    ctorArg.cwd = "modified";
    ctorArg.command.push("modified");
    assert.strictEqual(
      lspConfig.id,
      "one",
      "Modifying ctor id should not affect object",
    );
    assert.strictEqual(
      lspConfig.name,
      "two",
      "Modifying ctor name should not affect object",
    );
    assert.strictEqual(
      lspConfig.description,
      "three",
      "Modifying ctor description should not affect object",
    );
    assert.strictEqual(
      lspConfig.cwd,
      "four",
      "Modifying ctor cwd should not affect object",
    );
    assert.deepStrictEqual(
      lspConfig.command,
      ["five", "six"],
      "Modifying ctor command should not affect object",
    );
  });

  suite("equality", () => {
    const json = {
      id: "one",
      name: "two",
      description: "three",
      cwd: "four",
      command: ["five", "six"],
    };
    const config1 = new SorbetLspConfig(json);
    const config2 = new SorbetLspConfig(json);
    const differentConfigs = [
      new SorbetLspConfig({ ...json, id: "different id" }),
      new SorbetLspConfig({ ...json, name: "different name" }),
      new SorbetLspConfig({ ...json, description: "different description" }),
      new SorbetLspConfig({ ...json, cwd: "different cwd" }),
      new SorbetLspConfig({ ...json, command: ["different", "command"] }),
      undefined,
      null,
    ];

    test(".isEqualTo(other)", () => {
      assert.ok(config1.isEqualTo(config2));
      differentConfigs.forEach((c) => {
        assert.ok(!config1.isEqualTo(c), `Should not equal: ${c}`);
      });
      assert.ok(!config1.isEqualTo(json), "Should not equal JSON");
      assert.ok(
        !config1.isEqualTo(JSON.stringify(json)),
        "Should not equal stringified JSON",
      );
      assert.ok(
        !config1.isEqualTo(JSON.stringify(config1)),
        "Should not equal stringified config",
      );
    });

    test(".areEqual(config1, config2)", () => {
      assert.ok(SorbetLspConfig.areEqual(config1, config2));
      differentConfigs.forEach((c) => {
        assert.ok(
          !SorbetLspConfig.areEqual(config1, c),
          `Should not equal: ${c}`,
        );
      });
    });
  });
});

// Actual tests

suite("SorbetExtensionConfig", async () => {
  suite("initialization", async () => {
    suite("when no sorbet settings", async () => {
      test("sorbet is disabled", async () => {
        const workspaceConfig = new FakeWorkspaceConfiguration();
        const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
        assert.strictEqual(
          sorbetConfig.enabled,
          false,
          "should not be enabled",
        );
        assert.deepStrictEqual(
          sorbetConfig.selectedLspConfig,
          new SorbetLspConfig(workspaceConfig.defaults.get("lspConfigs")[0]),
          "LSP configs should have been set to first default",
        );
        assert.strictEqual(
          sorbetConfig.activeLspConfig,
          null,
          "should not have an active LSP config",
        );
      });
    });

    suite("when a sorbet/config file exists", async () => {
      test("sorbet is enabled", async () => {
        sinon
          .stub(fs, "existsSync")
          .withArgs("/fake/path/to/project/sorbet/config")
          .returns(true);

        const workspaceConfig = new FakeWorkspaceConfiguration();
        const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);

        assert.strictEqual(sorbetConfig.enabled, true, "should be enabled");
        sinon.restore();
      });
    });

    suite("when workspace has fully-populated sorbet settings", async () => {
      test("populates SorbetExtensionConfig", async () => {
        const workspaceConfig = new FakeWorkspaceConfiguration([
          ["enabled", true],
          ["lspConfigs", [fooLspConfig, barLspConfig]],
          ["selectedLspConfigId", "bar"],
        ]);
        const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
        assert.strictEqual(
          sorbetConfig.enabled,
          true,
          "extension should be enabled",
        );
        const { lspConfigs, selectedLspConfig, activeLspConfig } = sorbetConfig;
        assert.strictEqual(
          lspConfigs.length,
          2,
          "Should have two configs (foo and bar)",
        );
        assert.strictEqual(
          selectedLspConfig && selectedLspConfig.id,
          barLspConfig.id,
          "selected config should be bar",
        );
        assert.strictEqual(
          activeLspConfig,
          selectedLspConfig,
          "Active config should be same as selected",
        );
      });
    });

    suite("when workspace has *some* sorbet settings", async () => {
      test("when `sorbet.enabled` is missing", async () => {
        sinon
          .stub(fs, "existsSync")
          .withArgs("/fake/path/to/project/sorbet/config")
          .returns(false);

        const workspaceConfig = new FakeWorkspaceConfiguration([
          ["lspConfigs", [fooLspConfig, barLspConfig]],
          ["selectedLspConfigId", barLspConfig.id],
        ]);
        const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
        assert.strictEqual(
          sorbetConfig.enabled,
          false,
          "should not be enabled",
        );
        const { selectedLspConfig } = sorbetConfig;
        assert.strictEqual(
          selectedLspConfig && selectedLspConfig.id,
          barLspConfig.id,
          "should have a selected LSP config",
        );
        assert.strictEqual(
          sorbetConfig.activeLspConfig,
          null,
          "but should not have an active LSP config",
        );

        sinon.restore();
      });

      test("when `sorbet.selectedLspConfigId` is missing", async () => {
        const workspaceConfig = new FakeWorkspaceConfiguration([
          ["enabled", true],
          ["lspConfigs", [fooLspConfig, barLspConfig]],
        ]);
        const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
        assert.strictEqual(sorbetConfig.enabled, true, "should be enabled");
        const { selectedLspConfig } = sorbetConfig;
        assert.strictEqual(
          selectedLspConfig && selectedLspConfig.id,
          fooLspConfig.id,
          "selectedLspConfig should default to first config",
        );
        assert.strictEqual(
          sorbetConfig.activeLspConfig,
          selectedLspConfig,
          "activeLspConfig should also default to first config",
        );
      });

      [undefined, ""].forEach((configId) => {
        test(`when \`sorbet.selectedLspConfigId\` is '${configId}', picks first available configuration`, async () => {
          const workspaceConfig = new FakeWorkspaceConfiguration([
            ["enabled", true],
            ["lspConfigs", [fooLspConfig, barLspConfig]],
            ["selectedLspConfigId", configId],
          ]);
          const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
          assert.strictEqual(sorbetConfig.enabled, true, "should be enabled");
          assert.strictEqual(
            sorbetConfig.selectedLspConfig,
            sorbetConfig.lspConfigs[0],
            "selectedLspConfig should be undefined",
          );
          assert.strictEqual(
            sorbetConfig.activeLspConfig,
            sorbetConfig.lspConfigs[0],
            "activeLspConfig should be undefined",
          );
        });
      });

      test("when `sorbet.selectedLspConfigId` matches none of the defined `sorbet.lspConfigs`", async () => {
        const workspaceConfig = new FakeWorkspaceConfiguration([
          ["enabled", true],
          ["lspConfigs", [fooLspConfig]],
          ["selectedLspConfigId", barLspConfig.id],
        ]);
        const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
        assert.strictEqual(sorbetConfig.enabled, true, "should be enabled");
        assert.strictEqual(
          sorbetConfig.selectedLspConfig,
          undefined,
          "selectedLspConfig should be undefined",
        );
        assert.strictEqual(
          sorbetConfig.activeLspConfig,
          undefined,
          "activeLspConfig should be undefined",
        );
      });

      test("multiple instances of SorbetExtensionConfig stay in sync with each other", async () => {
        const workspaceConfig = new FakeWorkspaceConfiguration([
          ["enabled", true],
          ["lspConfigs", [fooLspConfig, barLspConfig]],
          ["selectedLspConfigId", fooLspConfig.id],
        ]);
        const sorbetConfig1 = new SorbetExtensionConfig(workspaceConfig);
        const sorbetConfig2 = new SorbetExtensionConfig(workspaceConfig);
        assert.ok(sorbetConfig2.activeLspConfig);
        assert.strictEqual(
          sorbetConfig2.activeLspConfig!.id,
          fooLspConfig.id,
          "Precondition: config2 should be set to 'foo'",
        );
        await sorbetConfig1.setActiveLspConfigId(barLspConfig.id);
        assert.ok(sorbetConfig2.activeLspConfig);
        assert.strictEqual(
          sorbetConfig2.activeLspConfig!.id,
          barLspConfig.id,
          "changing config1 should also change config2",
        );
      });
    });

    suite("when `sorbet.userLspConfigs` is specified", async () => {
      test("when values are distinct from `lspConfigs`", async () => {
        const workspaceConfig = new FakeWorkspaceConfiguration([
          ["lspConfigs", [fooLspConfig]],
          ["userLspConfigs", [barLspConfig]],
        ]);
        const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
        const { selectedLspConfig, lspConfigs } = sorbetConfig;
        assert.deepStrictEqual(
          lspConfigs,
          [barLspConfig, fooLspConfig],
          "items from userLspConfigs should precede items from lspConfigs",
        );
        assert.deepStrictEqual(
          selectedLspConfig,
          barLspConfig,
          "First element of userLspConfigs should be the selected/default config",
        );
      });
      test("when values overlap with `lspConfigs`", async () => {
        const userConfig = new SorbetLspConfig({
          ...barLspConfig,
          id: fooLspConfig.id,
        });
        assert.notDeepEqual(
          userConfig,
          fooLspConfig,
          "Precondition: userFoo and foo should be different configs",
        );
        assert.strictEqual(
          userConfig.id,
          fooLspConfig.id,
          "Precondition: userFoo and foo should have the same id",
        );
        const workspaceConfig = new FakeWorkspaceConfiguration([
          ["lspConfigs", [fooLspConfig]],
          ["userLspConfigs", [userConfig]],
        ]);
        const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
        const { selectedLspConfig, lspConfigs } = sorbetConfig;
        assert.deepStrictEqual(
          lspConfigs,
          [userConfig],
          "Item from userLspConfigs should override same id from lspConfigs",
        );
        assert.deepStrictEqual(
          selectedLspConfig,
          userConfig,
          "Selected config should be first of userLspConfigs",
        );
      });
    });
  });

  suite(".enabled", async () => {
    test("fires onLspConfigChange event when enabling the extension", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", false],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      sorbetConfig.setEnabled(true).then(() => {
        assert.strictEqual(
          listener.called,
          true,
          "should have called listener onLspConfigChange",
        );
        assert.deepStrictEqual(
          listener.getCall(0).args[0],
          {
            oldLspConfig: null,
            newLspConfig: barLspConfig,
          },
          "should have transitioned from no config to bar config",
        );
      });
    });

    test("fires onLspConfigChange event when disabling the extension", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", true],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      sorbetConfig.setEnabled(false).then(() => {
        assert.strictEqual(
          listener.called,
          true,
          "should have called listener onLspConfigChange",
        );
        assert.deepStrictEqual(
          listener.getCall(0).args[0],
          {
            oldLspConfig: barLspConfig,
            newLspConfig: null,
          },
          "should have transitioned from bar config to no config",
        );
      });
    });
  });

  suite(".setSelectedLspConfigId", async () => {
    test("fires onLspConfigChange event when changing LSPConfig", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", true],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      sorbetConfig.setSelectedLspConfigId(fooLspConfig.id).then(() => {
        assert.strictEqual(listener.called, true, "should be notified");
        assert.deepStrictEqual(
          listener.getCall(0).args[0],
          {
            oldLspConfig: barLspConfig,
            newLspConfig: fooLspConfig,
          },
          "should have transitioned from bar config to foo config",
        );
      });
    });

    test("does not fire onLspConfigChange event when unchanged", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", true],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      assert.strictEqual(sorbetConfig.selectedLspConfig!.id, barLspConfig.id);
      sorbetConfig.setSelectedLspConfigId(barLspConfig!.id).then(() => {
        assert.strictEqual(listener.called, false, "should not be notified");
      });
    });

    test("does not fire onLspConfigChange event when extension disabled", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", false],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      sorbetConfig.setSelectedLspConfigId(fooLspConfig.id).then(() => {
        assert.strictEqual(listener.called, false, "should not be notified");
      });
    });
  });

  suite(".onLspConfigChange when WorkspaceConfiguration changes", async () => {
    test("does not fire when extension is disabled", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", false],
        ["lspConfigs", [fooLspConfig]],
        ["selectedLspConfigId", fooLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      workspaceConfig.update("lspConfigs", [barLspConfig]);
      workspaceConfig.update("selectedLspConfigId", barLspConfig.id);
      assert.strictEqual(listener.called, false, "should not be notified");
    });

    test("fires when active LSP config is modified", async () => {
      const modifiedBarLspConfig = new SorbetLspConfig({
        ...barLspConfig,
        command: ["different", "command", "here"],
      });
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", true],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      workspaceConfig.update("lspConfigs", [
        fooLspConfig,
        modifiedBarLspConfig,
      ]);
      assert.strictEqual(listener.called, true, "should be notified");
      assert.deepStrictEqual(
        listener.getCall(0).args[0],
        {
          oldLspConfig: barLspConfig,
          newLspConfig: modifiedBarLspConfig,
        },
        "should have transitioned from old bar config to modified bar config",
      );
    });

    test("does not fire when non-active LSP config is modified", async () => {
      const modifiedFooLspConfig = new SorbetLspConfig({
        ...fooLspConfig,
        command: ["different", "command", "here"],
      });
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", true],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      workspaceConfig.update("lspConfigs", [
        modifiedFooLspConfig,
        barLspConfig,
      ]);
      assert.strictEqual(listener.called, false, "should not be notified");
    });

    test("does not fire when a new (non-active) LSP config is added", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", true],
        ["lspConfigs", [barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      workspaceConfig.update("lspConfigs", [fooLspConfig, barLspConfig]);
      assert.strictEqual(listener.called, false, "should not be notified");
    });

    test("does not fire when a non-active LSP config is removed", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", true],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
        ["selectedLspConfigId", barLspConfig.id],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      workspaceConfig.update("lspConfigs", [barLspConfig]);
      assert.strictEqual(
        listener.called,
        false,
        "should not have called listener onLspConfigChange",
      );
    });

    test("fires when the active LSP changes by nature of being the first lspConfig", async () => {
      const workspaceConfig = new FakeWorkspaceConfiguration([
        ["enabled", true],
        ["lspConfigs", [fooLspConfig, barLspConfig]],
      ]);
      const sorbetConfig = new SorbetExtensionConfig(workspaceConfig);
      const listener = sinon.spy();
      sorbetConfig.onLspConfigChange(listener);
      workspaceConfig.update("lspConfigs", [barLspConfig, fooLspConfig]);
      assert.strictEqual(
        listener.called,
        true,
        "should have called listener onLspConfigChange",
      );
      assert.deepStrictEqual(
        listener.getCall(0).args[0],
        {
          oldLspConfig: fooLspConfig,
          newLspConfig: barLspConfig,
        },
        "should have transitioned from foo config to bar config",
      );
    });
  });
});
