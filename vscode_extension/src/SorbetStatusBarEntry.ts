import * as Spinner from "elegant-spinner";
import { commands, OutputChannel, StatusBarAlignment, window } from "vscode";

import { ShowOperationParams, ServerStatus, RestartReason } from "./types";
import { SorbetExtensionConfig } from "./config";
import SorbetConfigPicker from "./ConfigPicker";

/**
 * Actions provided when 'Sorbet' fails to start.
 */
const enum Action {
  EnableSorbet = "Enable Sorbet",
  ConfigureSorbet = "Configure Sorbet",
  RestartSorbet = "Restart Sorbet",
  DisableSorbet = "Disable Sorbet",
  ViewOutput = "View Output",
}

export default class SorbetStatusBarEntry {
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
    private readonly _sorbetExtensionConfig: SorbetExtensionConfig,
    private readonly _restartSorbet: (reason: RestartReason) => void,
  ) {
    // Note: Internal command. Not advertised to users in `package.json`.
    const statusBarClickedCommand = "_sorbet.statusBarClicked";
    this._statusBarItem.command = statusBarClickedCommand;
    this._render();
    this._statusBarItem.show();
    commands.registerCommand(statusBarClickedCommand, () =>
      this.handleStatusBarClicked(),
    );
    _sorbetExtensionConfig.onLspConfigChange(() => this._render());
  }

  private _runAction(action?: string) {
    switch (action) {
      case Action.ViewOutput:
        this._outputChannel.show();
        break;
      case Action.ConfigureSorbet:
        new SorbetConfigPicker(this._sorbetExtensionConfig).show();
        break;
      case Action.EnableSorbet:
        this._sorbetExtensionConfig.setEnabled(true);
        break;
      case Action.DisableSorbet:
        this._sorbetExtensionConfig.setEnabled(false);
        break;
      case Action.RestartSorbet:
        this._restartSorbet(RestartReason.STATUS_BAR_BUTTON);
        break;
      default:
        // Nothing selected.
        break;
    }
  }

  public async handleStatusBarClicked(): Promise<void> {
    switch (this._serverStatus) {
      case ServerStatus.ERROR: {
        const actions = [
          Action.ViewOutput,
          Action.ConfigureSorbet,
          Action.RestartSorbet,
        ];
        const message = await window.showErrorMessage(
          this._lastError,
          ...actions,
        );
        return this._runAction(message);
      }
      case ServerStatus.DISABLED: {
        // Switch to Sorbet option.
        return this._runAction(
          await window.showInformationMessage(
            "Sorbet: Select action...",
            Action.ViewOutput,
            Action.ConfigureSorbet,
            Action.EnableSorbet,
          ),
        );
      }
      default: {
        return this._runAction(
          await window.showInformationMessage(
            "Sorbet: Select action...",
            Action.ViewOutput,
            Action.ConfigureSorbet,
            Action.RestartSorbet,
            Action.DisableSorbet,
          ),
        );
      }
    }
  }

  public changeServerStatus(s: ServerStatus, lastError: string = "") {
    const isError = this._serverStatus !== s && s === ServerStatus.ERROR;
    this._serverStatus = s;
    this._lastError = lastError;
    this._render();
    if (isError) {
      this._restartSorbet(RestartReason.CRASH_EXT_ERROR);
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
    const { activeLspConfig } = this._sorbetExtensionConfig;
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
