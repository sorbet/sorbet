import { QuickPickItem, window } from "vscode";
import { isEqual } from "lodash";
import { SorbetExtensionConfig, SorbetLspConfig } from "../config";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

interface SorbetQuickPickItem extends QuickPickItem {
  lspConfig?: SorbetLspConfig;
}

/**
 * Show Sorbet Configuration picker.
 */
export default class ShowSorbetConfigurationPicker {
  private readonly _extensionConfig: SorbetExtensionConfig;

  public constructor(context: SorbetExtensionContext) {
    this._extensionConfig = context.config;
  }

  public async execute(): Promise<void> {
    const { activeLspConfig, lspConfigs } = this._extensionConfig;
    const items: SorbetQuickPickItem[] = lspConfigs.map((config) => ({
      label: `${isEqual(activeLspConfig, config) ? "• " : ""}${config.name}`,
      description: config.description,
      detail: config.command.join(" "),
      lspConfig: config,
    }));
    items.push({
      label: `${activeLspConfig ? "" : "• "}Disable Sorbet`,
      description: "Disable the Sorbet extension",
    });

    const selectedItem = await window.showQuickPick(items, {
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
