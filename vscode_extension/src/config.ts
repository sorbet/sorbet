import {
  ConfigurationChangeEvent,
  Disposable,
  Event,
  EventEmitter,
  ExtensionContext,
  FileSystemWatcher,
  Memento,
  Uri,
  workspace,
  WorkspaceFolder,
} from "vscode";
import * as fs from "fs";

/**
 * Compare two `string` arrays for deep, in-order equality.
 */
function deepEqual(a: ReadonlyArray<string>, b: ReadonlyArray<string>) {
  return a.length === b.length && a.every((itemA, index) => itemA === b[index]);
}

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
    if (!deepEqual(this.command, other.command)) {
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

  initializeEnabled(enabled: boolean): void;
}

/** Default implementation accesses `workspace` directly. */
export class DefaultSorbetWorkspaceContext implements ISorbetWorkspaceContext {
  static workspaceStateChangeEmitter = new EventEmitter<string>();
  private workspaceState: Memento;
  private cachedSorbetConfiguration = workspace.getConfiguration("sorbet");
  private onConfigurationChangeEmitter = new EventEmitter<
    ConfigurationChangeEvent
  >();

  constructor(extensionContext: ExtensionContext) {
    this.workspaceState = extensionContext.workspaceState;
    workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration("sorbet")) {
        // update the cached configuration before firing
        this.cachedSorbetConfiguration = workspace.getConfiguration("sorbet");
        this.onConfigurationChangeEmitter.fire(e);
      }
    });
    DefaultSorbetWorkspaceContext.workspaceStateChangeEmitter.event((k) => {
      this.onConfigurationChangeEmitter.fire({
        affectsConfiguration: () => k.startsWith("sorbet."),
      });
    });
  }

  public get<T>(section: string, defaultValue: T): T {
    const workspaceStateValue = this.workspaceState.get<T>(`sorbet.${section}`);
    if (workspaceStateValue !== undefined) {
      return workspaceStateValue;
    }
    return this.cachedSorbetConfiguration.get(section, defaultValue);
  }

  public update(section: string, value: any): Thenable<void> {
    const key = `sorbet.${section}`;
    return this.workspaceState
      .update(key, value)
      .then(() =>
        DefaultSorbetWorkspaceContext.workspaceStateChangeEmitter.fire(key),
      );
  }

  public get onDidChangeConfiguration(): Event<ConfigurationChangeEvent> {
    return this.onConfigurationChangeEmitter.event;
  }

  public workspaceFolders(): readonly WorkspaceFolder[] | undefined {
    return workspace.workspaceFolders;
  }

  /**
   * This function is a workaround to make it possible to enable Sorbet on first launch.
   *
   * The `sorbet.enabled` setting always has its default value set to `false` from `package.json` and cannot be
   * undefined. That means that invoking `workspaceContext.get("enabled", this.enabled)` will always return `false` on
   * first launch regardless of the value of `this.enabled`.
   *
   * To workaround this, we check if `sorbet.enabled` is still undefined in the workspace state and in every type of
   * configuration other than the `defaultValue`. If that's the case, then we can update the workspace state and enable
   * Sorbet on first launch.
   */
  public initializeEnabled(enabled: boolean) {
    const stateEnabled = this.workspaceState.get("sorbet.enabled");

    if (stateEnabled === undefined) {
      const cachedConfig = this.cachedSorbetConfiguration.inspect("enabled");

      if (
        cachedConfig?.globalValue === undefined &&
        cachedConfig?.workspaceValue === undefined &&
        cachedConfig?.workspaceFolderValue === undefined &&
        cachedConfig?.globalLanguageValue === undefined &&
        cachedConfig?.workspaceFolderLanguageValue === undefined &&
        cachedConfig?.workspaceLanguageValue === undefined
      ) {
        this.update("enabled", enabled);
      }
    }
  }
}

export class SorbetExtensionConfig implements Disposable {
  private sorbetWorkspaceContext: ISorbetWorkspaceContext;
  private readonly onLspConfigChangeEmitter = new EventEmitter<
    SorbetLspConfigChangeEvent
  >();

  /** "Standard" LSP configs. */
  private wrappedLspConfigs: ReadonlyArray<SorbetLspConfig> = [];
  /**
   * "Custom" LSP configs that override/supplement "standard" LSP configs.
   *
   * If there is a '_lspConfig' and a '_userLspConfigs'
   */
  private userLspConfigs: ReadonlyArray<SorbetLspConfig> = [];
  private selectedLspConfigId: string | undefined = undefined;

  private wrappedEnabled: boolean;
  private wrappedRevealOutputOnError: boolean = false;
  private wrappedHighlightUntyped: boolean = false;
  private configFilePatterns: ReadonlyArray<string> = [];
  private configFileWatchers: ReadonlyArray<FileSystemWatcher> = [];

  constructor(sorbetWorkspaceContext: ISorbetWorkspaceContext) {
    this.sorbetWorkspaceContext = sorbetWorkspaceContext;
    this.sorbetWorkspaceContext.onDidChangeConfiguration((_) => this.refresh());

    const workspaceFolders = sorbetWorkspaceContext.workspaceFolders();
    this.wrappedEnabled = workspaceFolders
      ? fs.existsSync(`${workspaceFolders[0].uri.fsPath}/sorbet/config`)
      : false;

    this.sorbetWorkspaceContext.initializeEnabled(this.wrappedEnabled);

    this.refresh();
  }

  /**
   * Refreshes the configuration from this._sorbetWorkspaceConfiguration,
   * emitting change events as necessary.
   */
  private refresh(): void {
    const oldLspConfig = this.activeLspConfig;
    const workspaceContext = this.sorbetWorkspaceContext;
    this.wrappedEnabled = workspaceContext.get("enabled", this.wrappedEnabled);
    this.wrappedRevealOutputOnError = workspaceContext.get(
      "revealOutputOnError",
      this.revealOutputOnError,
    );
    this.wrappedHighlightUntyped = workspaceContext.get(
      "highlightUntyped",
      this.highlightUntyped,
    );

    const oldConfigFilePatterns = this.configFilePatterns;
    this.configFilePatterns = [
      ...workspaceContext.get("configFilePatterns", this.configFilePatterns),
    ];
    Disposable.from(...this.configFileWatchers).dispose();
    this.configFileWatchers = this.configFilePatterns.map((pattern) => {
      const watcher = workspace.createFileSystemWatcher(pattern);
      const onConfigChange = (_uri: Uri) => {
        const c = this.activeLspConfig;
        this.onLspConfigChangeEmitter.fire({
          oldLspConfig: c,
          newLspConfig: c,
        });
      };
      watcher.onDidChange(onConfigChange);
      watcher.onDidCreate(onConfigChange);
      watcher.onDidDelete(onConfigChange);
      return watcher;
    });
    const iLspConfigs = workspaceContext.get("lspConfigs", []);
    this.wrappedLspConfigs = iLspConfigs.map((c) => new SorbetLspConfig(c));
    const iUserLspConfigs = workspaceContext.get("userLspConfigs", []);
    this.userLspConfigs = iUserLspConfigs.map((c) => new SorbetLspConfig(c));
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
    this.selectedLspConfigId = configId;
    const newLspConfig = this.activeLspConfig;
    if (
      !SorbetLspConfig.areEqual(oldLspConfig, newLspConfig) ||
      !deepEqual(oldConfigFilePatterns, this.configFilePatterns)
    ) {
      this.onLspConfigChangeEmitter.fire({
        oldLspConfig,
        newLspConfig,
      });
    }
  }

  /**
   * An event that fires when the (effective) active configuration changes.
   */
  public get onLspConfigChange(): Event<SorbetLspConfigChangeEvent> {
    return this.onLspConfigChangeEmitter.event;
  }

  /**
   * Returns a copy of the current SorbetLspConfig objects.
   */
  public get lspConfigs(): ReadonlyArray<SorbetLspConfig> {
    const results: Array<SorbetLspConfig> = [];
    const resultIds = new Set<String>();
    [...this.userLspConfigs, ...this.wrappedLspConfigs].forEach((c) => {
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
    return this.lspConfigs.find((c) => c.id === this.selectedLspConfigId);
  }

  /**
   * Select the given `SorbetLspConfig`.
   *
   * (Note that if the extension is disabled, this does not *enable* the
   * configuration.)
   */
  public setSelectedLspConfigId(id: string): Thenable<void> {
    return this.sorbetWorkspaceContext
      .update("selectedLspConfigId", id)
      .then(this.refresh.bind(this));
  }

  /**
   * Select the given `SorbetLspConfig` and enable the extension, if
   * the extension is disabled.
   *
   * This is equivalent to calling `selectedLspConfigId = id; enabled=true`.
   */
  public setActiveLspConfigId(id: string): Thenable<void> {
    return Promise.all([
      this.sorbetWorkspaceContext.update("selectedLspConfigId", id),
      this.sorbetWorkspaceContext.update("enabled", true),
    ]).then(this.refresh.bind(this));
  }

  public get revealOutputOnError(): boolean {
    return this.wrappedRevealOutputOnError;
  }

  public get highlightUntyped(): boolean {
    return this.wrappedHighlightUntyped;
  }

  public get enabled(): boolean {
    return this.wrappedEnabled;
  }

  public setEnabled(b: boolean): Thenable<void> {
    return this.sorbetWorkspaceContext
      .update("enabled", b)
      .then(this.refresh.bind(this));
  }

  public setHighlightUntyped(b: boolean): Thenable<void> {
    return this.sorbetWorkspaceContext
      .update("highlightUntyped", b)
      .then(this.refresh.bind(this));
  }

  dispose() {
    Disposable.from(...this.configFileWatchers).dispose();
  }
}
