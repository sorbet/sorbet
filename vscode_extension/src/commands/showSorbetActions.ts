import { commands, window } from "vscode";
import {
  SHOW_CONFIG_PICKER_COMMAND_ID,
  SHOW_OUTPUT_COMMAND_ID,
  SORBET_DISABLE_COMMAND_ID,
  SORBET_ENABLE_COMMAND_ID,
  SORBET_RESTART_COMMAND_ID,
} from "../commandIds";
import { SorbetExtensionContext } from "../sorbetExtensionContext";
import { SorbetStatusProvider } from "../sorbetStatusProvider";
import { RestartReason, ServerStatus } from "../types";

export const enum Action {
  ConfigureSorbet = "Configure Sorbet",
  DisableSorbet = "Disable Sorbet",
  EnableSorbet = "Enable Sorbet",
  RestartSorbet = "Restart Sorbet",
  ViewOutput = "View Output",
}

/**
 * Show available actions in a drop-down.
 */
export class ShowSorbetActions {
  private readonly statusProvider: SorbetStatusProvider;

  constructor(context: SorbetExtensionContext) {
    this.statusProvider = context.statusProvider;
  }

  public async execute(): Promise<void> {
    const actions = this.getAvailableActions();
    const selectedAction = await window.showQuickPick(actions, {
      placeHolder: "Sorbet: Select action...",
    });

    switch (selectedAction) {
      case Action.ConfigureSorbet:
        await commands.executeCommand(SHOW_CONFIG_PICKER_COMMAND_ID);
        break;
      case Action.DisableSorbet:
        await commands.executeCommand(SORBET_DISABLE_COMMAND_ID);
        break;
      case Action.EnableSorbet:
        await commands.executeCommand(SORBET_ENABLE_COMMAND_ID);
        break;
      case Action.RestartSorbet:
        await commands.executeCommand(
          SORBET_RESTART_COMMAND_ID,
          RestartReason.STATUS_BAR_BUTTON,
        );
        break;
      case Action.ViewOutput:
        await commands.executeCommand(SHOW_OUTPUT_COMMAND_ID);
        break;
      default:
        break; // User canceled
    }
  }

  /**
   * Get available {@link Action actions} based on Sorbet' status.
   */
  public getAvailableActions(): Action[] {
    const actions = [Action.ViewOutput];
    switch (this.statusProvider.serverStatus) {
      case ServerStatus.ERROR:
        actions.push(Action.RestartSorbet);
        break;
      case ServerStatus.DISABLED:
        actions.push(Action.EnableSorbet);
        break;
      default:
        actions.push(Action.RestartSorbet, Action.DisableSorbet);
        break;
    }
    actions.push(Action.ConfigureSorbet);
    return actions;
  }
}
