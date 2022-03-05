import { isEqual } from "lodash";
import {
  workspace,
  Event,
  EventEmitter,
  ConfigurationChangeEvent,
  Disposable,
  ExtensionContext,
  Memento,
  FileSystemWatcher,
  Uri,
  WorkspaceFolder,
} from "vscode";
import * as fs from "fs";

interface ISorbetLspConfig {
  readonly id: string;
  /** Display name suitable for short-form fields like menu items or status fields. */
  readonly name: string;
  /** Human-readable long-form description suitable for hover text or help. */
  readonly description: string;
  readonly cwd: string;
  readonly command: ReadonlyArray<string>;
}

export class SorbetLspConfig {
  public readonly id: string;
  public readonly name: string;
  public readonly description: string;
  public readonly cwd: string;
  public readonly command: ReadonlyArray<string>;

  constructor({ id, name, description, cwd, command }: ISorbetLspConfig) {
    this.id = id;
    this.name = name;
    this.description = description;
    this.cwd = cwd;
    this.command = [...command];
  }

  public toJSON(): ISorbetLspConfig {
    return {
      id: this.id,
      name: this.name,
      description: this.description,
      cwd: this.cwd,
      command: this.command,
    };
  }

  public toString(): string {
    return `${this.name}: ${this.description} [cmd: "${this.command.join(
      " ",
    )}"]`;
  }

  /** Deep equality. */
  public isEqualTo(other: any): boolean {
    if (this === other) {
      return true;
    }
    if (!(other instanceof SorbetLspConfig)) {
      return false;
    }
    if (this.id !== other.id) {
      return false;
    }
    if (this.name !== other.name) {
      return false;
    }
    if (this.description !== other.description) {
      return false;
    }
    if (this.cwd !== other.cwd) {
      return false;
    }
    if (!isEqual(this.command, other.command)) {
      return false;
    }
    return true;
  }

  /** Deep equality, suitable for use when left and/or right may be null or undefined. */
  public static areEqual(
    left: SorbetLspConfig | undefined | null,
    right: SorbetLspConfig | undefined | null,
  ) {
    if (left) {
      return left.isEqualTo(right);
    } else {
      return left === right;
    }
  }
}

export class SorbetLspConfigChangeEvent {
  public readonly oldLspConfig: SorbetLspConfig | null | undefined;
  public readonly newLspConfig: SorbetLspConfig | null | undefined;
}

/**
 * Combines references to `extensionContext.workspaceState()`,
 * `workspace.getConfiguration("sorbet")`, and
 * `workspace.onDidChangeConfiguration` for the "sorbet" section
 * to make it easier to stub out behavior in tests.
 */
export interface ISorbetWorkspaceContext {
  /** See `vscode.Memento.get`. */
  get<T>(section: string, defaultValue: T): T;

  /** See `vscode.Memento.update`. */
  update(section: string, value: any): Thenable<void>;

  /** See `vscode.workspace.onDidChangeConfiguration` */
  onDidChangeConfiguration: Event<ConfigurationChangeEvent>;

  /** See `vscode.workspace.workspaceFolders` */
  workspaceFolders(): ReadonlyArray<WorkspaceFolder> | undefined;
}

/** Default implementation accesses `workspace` directly. */
export class DefaultSorbetWorkspaceContext implements ISorbetWorkspaceContext {
  static _workspaceStateChangeEmitter = new EventEmitter<string>();
  private _workspaceState: Memento;
  private _cachedSorbetConfiguration = workspace.getConfiguration("sorbet");
  private _emitter = new EventEmitter<ConfigurationChangeEvent>();
  constructor(extensionContext: ExtensionContext) {
    this._workspaceState = extensionContext.workspaceState;
    workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration("sorbet")) {
        // update the cached configuration before firing
        this._cachedSorbetConfiguration = workspace.getConfiguration("sorbet");
        this._emitter.fire(e);
      }
    });
    DefaultSorbetWorkspaceContext._workspaceStateChangeEmitter.event((k) => {
      this._emitter.fire({
        affectsConfiguration: () => k.startsWith("sorbet."),
      });
    });
  }

  public get<T>(section: string, defaultValue: T): T {
    const workspaceStateValue = this._workspaceState.get<T>(
      `sorbet.${section}`,
    );
    if (workspaceStateValue !== undefined) {
      return workspaceStateValue;
    }
    return this._cachedSorbetConfiguration.get(section, defaultValue);
  }

  public update(section: string, value: any): Thenable<void> {
    const key = `sorbet.${section}`;
    return this._workspaceState
      .update(key, value)
      .then(() =>
        DefaultSorbetWorkspaceContext._workspaceStateChangeEmitter.fire(key),
      );
  }

  public get onDidChangeConfiguration(): Event<ConfigurationChangeEvent> {
    return this._emitter.event;
  }

  public workspaceFolders(): readonly WorkspaceFolder[] | undefined {
    return workspace.workspaceFolders;
  }
}

export class SorbetExtensionConfig implements Disposable {
  private _sorbetWorkspaceContext: ISorbetWorkspaceContext;
  private _onLspConfigChangeEmitter = new EventEmitter<
    SorbetLspConfigChangeEvent
  >();

  /** "Standard" LSP configs. */
  private _lspConfigs: ReadonlyArray<SorbetLspConfig> = [];
  /**
   * "Custom" LSP configs that override/supplement "standard" LSP configs.
   *
   * If there is a '_lspConfig' and a '_userLspConfigs'
   */
  private _userLspConfigs: ReadonlyArray<SorbetLspConfig> = [];
  private _selectedLspConfigId: string | undefined = undefined;

  private _enabled: boolean;
  private _revealOutputOnError: boolean = false;
  private _configFilePatterns: ReadonlyArray<string> = [];
  private _configFileWatchers: ReadonlyArray<FileSystemWatcher> = [];

  constructor(sorbetWorkspaceContext: ISorbetWorkspaceContext) {
    this._sorbetWorkspaceContext = sorbetWorkspaceContext;
    this._sorbetWorkspaceContext.onDidChangeConfiguration((_) =>
      this._refresh(),
    );

    const workspaceFolders = sorbetWorkspaceContext.workspaceFolders();
    this._enabled = workspaceFolders
      ? fs.existsSync(`${workspaceFolders[0].uri.fsPath}/sorbet/config`)
      : false;

    this._refresh();
  }

  /**
   * Refreshes the configuration from this._sorbetWorkspaceConfiguration,
   * emitting change events as necessary.
   */
  private _refresh(): void {
    const oldLspConfig = this.activeLspConfig;
    const workspaceContext = this._sorbetWorkspaceContext;
    this._enabled = workspaceContext.get("enabled", this._enabled);
    this._revealOutputOnError = workspaceContext.get(
      "revealOutputOnError",
      this.revealOutputOnError,
    );
    const oldConfigFilePatterns = this._configFilePatterns;
    this._configFilePatterns = [
      ...workspaceContext.get("configFilePatterns", this._configFilePatterns),
    ];
    Disposable.from(...this._configFileWatchers).dispose();
    this._configFileWatchers = this._configFilePatterns.map((pattern) => {
      const watcher = workspace.createFileSystemWatcher(pattern);
      const _onConfigChange = (_: Uri) => {
        const c = this.activeLspConfig;
        this._onLspConfigChangeEmitter.fire({
          oldLspConfig: c,
          newLspConfig: c,
        });
      };
      watcher.onDidChange(_onConfigChange);
      watcher.onDidCreate(_onConfigChange);
      watcher.onDidDelete(_onConfigChange);
      return watcher;
    });
    const iLspConfigs = workspaceContext.get("lspConfigs", []);
    this._lspConfigs = iLspConfigs.map((c) => new SorbetLspConfig(c));
    const iUserLspConfigs = workspaceContext.get("userLspConfigs", []);
    this._userLspConfigs = iUserLspConfigs.map((c) => new SorbetLspConfig(c));
    let configId = workspaceContext.get<string | undefined>(
      "selectedLspConfigId",
      undefined,
    );
    if (!configId) {
      const configs = this.lspConfigs;
      if (configs.length > 0) {
        // If no selectedLspConfigId has been explicitly set, but there
        // are configurations, default to the first one.
        configId = configs[0].id;
      }
    }
    this._selectedLspConfigId = configId;
    const newLspConfig = this.activeLspConfig;
    if (
      !SorbetLspConfig.areEqual(oldLspConfig, newLspConfig) ||
      !isEqual(oldConfigFilePatterns, this._configFilePatterns)
    ) {
      this._onLspConfigChangeEmitter.fire({
        oldLspConfig,
        newLspConfig,
      });
    }
  }

  /**
   * An event that fires when the (effective) active configuration changes.
   */
  public get onLspConfigChange(): Event<SorbetLspConfigChangeEvent> {
    return this._onLspConfigChangeEmitter.event;
  }

  /**
   * Returns a copy of the current SorbetLspConfig objects.
   */
  public get lspConfigs(): ReadonlyArray<SorbetLspConfig> {
    const results: Array<SorbetLspConfig> = [];
    const resultIds = new Set<String>();
    [...this._userLspConfigs, ...this._lspConfigs].forEach((c) => {
      if (!resultIds.has(c.id)) {
        results.push(c);
        resultIds.add(c.id);
      }
    });
    return results;
  }

  /**
   * Returns the active `SorbetLspConfig`.
   *
   * If the Sorbet extension is disabled, returns `null`, otherwise
   * returns a `SorbetLspConfig` or `undefined` as per `selectedLspConfig`.
   */
  public get activeLspConfig(): SorbetLspConfig | null | undefined {
    return this.enabled ? this.selectedLspConfig : null;
  }

  /**
   * Returns the selected `SorbetLspConfig`, even if the extension is disabled.
   *
   * If the configuration does not specify a `selectedLspConfigId`, or if
   * the `id` refers to a `SorbetLspConfig` that does not exist, return `undefined`.
   */
  public get selectedLspConfig(): SorbetLspConfig | undefined {
    return this.lspConfigs.find((c) => c.id === this._selectedLspConfigId);
  }

  /**
   * Select the given `SorbetLspConfig`.
   *
   * (Note that if the extension is disabled, this does not *enable* the
   * configuration.)
   */
  public setSelectedLspConfigId(id: string): Thenable<void> {
    return this._sorbetWorkspaceContext
      .update("selectedLspConfigId", id)
      .then(this._refresh.bind(this));
  }

  /**
   * Select the given `SorbetLspConfig` and enable the extension, if
   * the extension is disabled.
   *
   * This is equivalent to calling `selectedLspConfigId = id; enabled=true`.
   */
  public setActiveLspConfigId(id: string): Thenable<void> {
    return Promise.all([
      this._sorbetWorkspaceContext.update("selectedLspConfigId", id),
      this._sorbetWorkspaceContext.update("enabled", true),
    ]).then(this._refresh.bind(this));
  }

  public get revealOutputOnError(): boolean {
    return this._revealOutputOnError;
  }

  public get enabled(): boolean {
    return this._enabled;
  }

  public setEnabled(b: boolean): Thenable<void> {
    return this._sorbetWorkspaceContext
      .update("enabled", b)
      .then(this._refresh.bind(this));
  }

  dispose() {
    Disposable.from(...this._configFileWatchers).dispose();
  }
}
