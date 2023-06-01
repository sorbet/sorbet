import { QuickPickItem, window } from "vscode";
import { SorbetExtensionConfig, SorbetLspConfig } from "../config";
import { SorbetExtensionContext } from "../sorbetExtensionContext";

interface SorbetQuickPickItem extends QuickPickItem {
  lspConfig?: SorbetLspConfig;
}

/**
 * Show Sorbet Configuration picker.
 */
export class ShowSorbetConfigurationPicker {
  private readonly configuration: SorbetExtensionConfig;

  public constructor(context: SorbetExtensionContext) {
    this.configuration = context.configuration;
  }

  public async execute(): Promise<void> {
    const { activeLspConfig, lspConfigs } = this.configuration;
    const items: SorbetQuickPickItem[] = lspConfigs.map((config) => ({
      label: `${config.isEqualTo(activeLspConfig) ? "• " : ""}${config.name}`,
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
        this.configuration.setActiveLspConfigId(lspConfig.id);
      } else {
        this.configuration.setEnabled(false);
      }
    }
  }
}
