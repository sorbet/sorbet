import { QuickPickItem, window } from "vscode";
import { isEqual } from "lodash";
import { SorbetExtensionConfig, SorbetLspConfig } from "../config";

interface SorbetQuickPickItem extends QuickPickItem {
  lspConfig?: SorbetLspConfig;
}

/**
 * Show Sorbet Configuration picker.
 */
export default class ShowSorbetConfigurationPicker {
  private readonly _extensionConfig: SorbetExtensionConfig;
  public constructor(c: SorbetExtensionConfig) {
    this._extensionConfig = c;
  }

  public async execute(): Promise<void> {
    const { activeLspConfig, lspConfigs } = this._extensionConfig;
    const pickOptions: SorbetQuickPickItem[] = lspConfigs.map((config) => ({
      label: `${isEqual(activeLspConfig, config) ? "• " : ""}${config.name}`,
      description: config.description,
      detail: config.command.join(" "),
      lspConfig: config,
    }));
    pickOptions.push({
      label: `${activeLspConfig ? "" : "• "}Disable Sorbet`,
      description: "Disable the Sorbet extension",
    });

    const selectedItem = await window.showQuickPick(pickOptions, {
      placeHolder: "Select a Sorbet configuration",
    });
    if (selectedItem) {
      const { lspConfig } = selectedItem;
      if (lspConfig) {
        this._extensionConfig.setActiveLspConfigId(lspConfig.id);
      } else {
        this._extensionConfig.setEnabled(false);
      }
    }
  }
}
