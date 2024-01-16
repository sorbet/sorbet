import * as Spinner from "elegant-spinner";
import { Disposable, StatusBarAlignment, StatusBarItem, window } from "vscode";

import { SHOW_ACTIONS_COMMAND_ID } from "./commandIds";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { StatusChangedEvent } from "./sorbetStatusProvider";
import { RestartReason, ServerStatus } from "./types";

export class SorbetStatusBarEntry implements Disposable {
  private readonly context: SorbetExtensionContext;
  private readonly disposable: Disposable;
  private serverStatus: ServerStatus;
  private readonly spinner: () => string;
  private spinnerTimer?: NodeJS.Timer;
  private readonly statusBarItem: StatusBarItem;

  constructor(context: SorbetExtensionContext) {
    this.context = context;
    this.serverStatus = ServerStatus.DISABLED;
    this.spinner = Spinner();
    this.statusBarItem = window.createStatusBarItem(
      StatusBarAlignment.Left,
      10,
    );
    this.statusBarItem.command = SHOW_ACTIONS_COMMAND_ID;

    this.disposable = Disposable.from(
      this.context.configuration.onLspConfigChange(() => this.render()),
      this.context.statusProvider.onStatusChanged((e) =>
        this.onServerStatusChanged(e),
      ),
      this.context.statusProvider.onShowOperation((_params) => this.render()),
      this.statusBarItem,
    );

    this.render();
    this.statusBarItem.show();
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    this.disposable.dispose();
  }

  private async onServerStatusChanged(e: StatusChangedEvent): Promise<void> {
    const isError =
      this.serverStatus !== e.status && e.status === ServerStatus.ERROR;
    this.serverStatus = e.status;
    this.render();
    if (isError) {
      await this.context.statusProvider.restartSorbet(
        RestartReason.CRASH_EXT_ERROR,
      );
    }
  }

  private getSpinner() {
    if (this.spinnerTimer) {
      clearTimeout(this.spinnerTimer);
    }
    // Animate the spinner with setTimeout.
    this.spinnerTimer = setTimeout(() => this.render(), 250);
    return this.spinner();
  }

  private render() {
    const { operations } = this.context.statusProvider;
    const { activeLspConfig } = this.context.configuration;
    const sorbetName = activeLspConfig?.name ?? "Sorbet";

    let text: string;
    let tooltip: string;
    // Errors should suppress operation animations / feedback.
    if (
      activeLspConfig &&
      this.serverStatus !== ServerStatus.ERROR &&
      operations.length > 0
    ) {
      const latestOp = operations[operations.length - 1];
      text = `${sorbetName}: ${latestOp.description} ${this.getSpinner()}`;
      tooltip = "The Sorbet server is currently running.";
    } else {
      switch (this.serverStatus) {
        case ServerStatus.DISABLED:
          text = `${sorbetName}: Disabled`;
          tooltip = "The Sorbet server is disabled.";
          break;
        case ServerStatus.ERROR:
          text = `${sorbetName}: Error`;
          tooltip = "Click for remediation items.";
          const { serverError } = this.context.statusProvider;
          if (serverError) {
            tooltip = `${serverError}\n${tooltip}`;
          }
          break;
        case ServerStatus.INITIALIZING:
          text = `${sorbetName}: Initializing ${this.getSpinner()}`;
          tooltip = "The Sorbet server is initializing.";
          break;
        case ServerStatus.RESTARTING:
          text = `${sorbetName}: Restarting ${this.getSpinner()}`;
          tooltip = "The Sorbet server is restarting.";
          break;
        case ServerStatus.RUNNING:
          text = `${sorbetName}: Idle`;
          tooltip = "The Sorbet server is currently running.";
          break;
        default:
          this.context.log.error(`Invalid ServerStatus: ${this.serverStatus}`);
          text = "";
          tooltip = "";
          break;
      }
    }

    this.statusBarItem.text = text;
    this.statusBarItem.tooltip = tooltip;
  }
}
