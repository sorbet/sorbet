import * as Spinner from "elegant-spinner";
import { Disposable, OutputChannel, StatusBarAlignment, window } from "vscode";

import { SHOW_ACTIONS_COMMAND_ID } from "./commandIds";
import { SorbetExtensionConfig } from "./config";
import { ShowOperationParams, ServerStatus, RestartReason } from "./types";

export default class SorbetStatusBarEntry implements Disposable {
  private readonly _statusBarItem = window.createStatusBarItem(
    StatusBarAlignment.Left,
    10,
  );

  private _operationStack: ShowOperationParams[] = [];
  private _serverStatus: ServerStatus = ServerStatus.DISABLED;
  private _lastError = "";
  private readonly _spinner = Spinner();
  private _spinnerTimer?: NodeJS.Timer;

  constructor(
    private readonly _outputChannel: OutputChannel,
    private readonly _extensionConfig: SorbetExtensionConfig,
    private readonly _restartSorbet: (reason: RestartReason) => Thenable<void>,
  ) {
    this._statusBarItem.command = SHOW_ACTIONS_COMMAND_ID;
    this._render();
    this._statusBarItem.show();
    _extensionConfig.onLspConfigChange(() => this._render());
  }

  /**
   * Return current {@link ServerStatus server status}.
   */
  public get serverStatus(): ServerStatus {
    return this._serverStatus;
  }

  public async changeServerStatus(
    status: ServerStatus,
    lastError: string = "",
  ): Promise<void> {
    const isError =
      this._serverStatus !== status && status === ServerStatus.ERROR;
    this._serverStatus = status;
    this._lastError = lastError;
    this._render();
    if (isError) {
      await this._restartSorbet(RestartReason.CRASH_EXT_ERROR);
    }
  }

  public handleShowOperation(p: ShowOperationParams) {
    if (p.status === "end") {
      this._operationStack = this._operationStack.filter(
        (otherP) => otherP.operationName !== p.operationName,
      );
    } else {
      this._operationStack.push(p);
    }
    this._render();
  }

  public clearOperations() {
    this._operationStack = [];
    this._render();
  }

  public dispose() {
    this._statusBarItem.dispose();
  }

  private _getSpinner() {
    if (this._spinnerTimer) {
      clearTimeout(this._spinnerTimer);
    }
    // Animate the spinner with setTimeout.
    this._spinnerTimer = setTimeout(() => this._render(), 100);
    return this._spinner();
  }

  private _render() {
    const numOperations = this._operationStack.length;
    const { activeLspConfig } = this._extensionConfig;
    const sorbetName = activeLspConfig ? activeLspConfig.name : "Sorbet";
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
          tooltip = `${this._lastError} Click for remediation items.`;
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
          this._outputChannel.appendLine(
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
