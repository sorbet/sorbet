import { QuickPickItem, window } from "vscode";
import { isEqual } from "lodash";
import { SorbetExtensionConfig, SorbetLspConfig } from "./config";

interface SorbetQuickPickItem extends QuickPickItem {
  lspConfig: SorbetLspConfig | null;
}

export default class SorbetConfigPicker {
  private readonly _extensionConfig: SorbetExtensionConfig;
  public constructor(c: SorbetExtensionConfig) {
    this._extensionConfig = c;
  }

  public async show(): Promise<void> {
    const { activeLspConfig, lspConfigs } = this._extensionConfig;
    const pickOptions: SorbetQuickPickItem[] = lspConfigs.map((c) => {
      return {
        label: `${isEqual(activeLspConfig, c) ? "• " : ""}${c.name}`,
        description: c.description,
        detail: c.command.join(" "),
        lspConfig: c,
      };
    });
    pickOptions.push({
      label: `${activeLspConfig ? "" : "• "}Disable Sorbet`,
      description: "Disable the Sorbet extension",
      lspConfig: null,
    });
    const selected = await window.showQuickPick<SorbetQuickPickItem>(
      pickOptions,
      {
        placeHolder: "Select a Sorbet configuration",
        ignoreFocusOut: false,
      },
    );
    if (selected) {
      const { lspConfig } = selected;
      if (lspConfig) {
        this._extensionConfig.setActiveLspConfigId(lspConfig.id);
      } else {
        this._extensionConfig.setEnabled(false);
      }
    }
  }
}
