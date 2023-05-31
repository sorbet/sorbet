import * as Spinner from "elegant-spinner";
import { Disposable, StatusBarAlignment, StatusBarItem, window } from "vscode";

import { SHOW_ACTIONS_COMMAND_ID } from "./commandIds";
import { SorbetExtensionContext } from "./sorbetExtensionContext";
import { StatusChangedEvent } from "./sorbetStatusProvider";
import { ShowOperationParams, ServerStatus, RestartReason } from "./types";

export class SorbetStatusBarEntry implements Disposable {
  private readonly _context: SorbetExtensionContext;
  private readonly _disposable: Disposable;
  private _operationStack: ShowOperationParams[];
  private _serverStatus: ServerStatus;
  private readonly _spinner: () => string;
  private _spinnerTimer?: NodeJS.Timer;
  private readonly _statusBarItem: StatusBarItem;

  constructor(context: SorbetExtensionContext) {
    this._context = context;
    this._operationStack = [];
    this._serverStatus = ServerStatus.DISABLED;
    this._spinner = Spinner();
    this._statusBarItem = window.createStatusBarItem(
      StatusBarAlignment.Left,
      10,
    );
    this._statusBarItem.command = SHOW_ACTIONS_COMMAND_ID;

    this._disposable = Disposable.from(
      this._context.config.onLspConfigChange(() => this._render()),
      this._context.statusProvider.onStatusChanged((e) =>
        this.onServerStatusChanged(e),
      ),
      this._context.statusProvider.onShowOperation((params) =>
        this.onServerShowOperation(params),
      ),
      this._statusBarItem,
    );

    this._render();
    this._statusBarItem.show();
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    this._disposable.dispose();
  }

  private async onServerStatusChanged(e: StatusChangedEvent): Promise<void> {
    const isError =
      this._serverStatus !== e.status && e.status === ServerStatus.ERROR;
    this._serverStatus = e.status;
    if (e.stopped) {
      this._operationStack = [];
    }
    this._render();
    if (isError) {
      await this._context.statusProvider.restartSorbet(
        RestartReason.CRASH_EXT_ERROR,
      );
    }
  }

  private onServerShowOperation(p: ShowOperationParams) {
    if (p.status === "end") {
      this._operationStack = this._operationStack.filter(
        (otherP) => otherP.operationName !== p.operationName,
      );
    } else {
      this._operationStack.push(p);
    }
    this._render();
  }

  private _getSpinner() {
    if (this._spinnerTimer) {
      clearTimeout(this._spinnerTimer);
    }
    // Animate the spinner with setTimeout.
    this._spinnerTimer = setTimeout(() => this._render(), 250);
    return this._spinner();
  }

  private _render() {
    const numOperations = this._operationStack.length;
    const { activeLspConfig } = this._context.config;
    const sorbetName = activeLspConfig?.name ?? "Sorbet";

    let text: string;
    let tooltip: string;
    // Errors should suppress operation animations / feedback.
    if (
      activeLspConfig &&
      this._serverStatus !== ServerStatus.ERROR &&
      numOperations > 0
    ) {
      const latestOp = this._operationStack[numOperations - 1];
      text = `${sorbetName}: ${latestOp.description} ${this._getSpinner()}`;
      tooltip = latestOp.description;
    } else {
      switch (this._serverStatus) {
        case ServerStatus.DISABLED:
          text = `${sorbetName}: Disabled`;
          tooltip = "The Sorbet server is disabled.";
          break;
        case ServerStatus.ERROR:
          text = `${sorbetName}: Error`;
          tooltip = "Click for remediation items.";
          const { serverError } = this._context.statusProvider;
          if (serverError) {
            tooltip = `${serverError}\n${tooltip}`;
          }
          break;
        case ServerStatus.INITIALIZING:
          text = `${sorbetName}: Initializing ${this._getSpinner()}`;
          tooltip = "The Sorbet server is initializing.";
          break;
        case ServerStatus.RESTARTING:
          text = `${sorbetName}: Restarting ${this._getSpinner()}`;
          tooltip = "The Sorbet server is restarting.";
          break;
        case ServerStatus.RUNNING:
          text = `${sorbetName}: Idle`;
          tooltip = "The Sorbet server is currently running.";
          break;
        default:
          this._context.outputChannel.appendLine(
            `Invalid ServerStatus: ${this._serverStatus}`,
          );
          text = "";
          tooltip = "";
          break;
      }
    }

    this._statusBarItem.text = text;
    this._statusBarItem.tooltip = tooltip;
  }
}
