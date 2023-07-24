import {
  ConfigurationChangeEvent,
  Disposable,
  Event,
  EventEmitter,
  ExtensionContext,
  FileSystemWatcher,
  Memento,
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

  public toString(): string {
    return `${this.name}: ${this.description} [cmd: "${this.command.join(
      " ",
    )}"]`;
  }

  /** Deep equality. */
  public isEqualTo(other: any): boolean {
    if (
      this !== other &&
      (!(other instanceof SorbetLspConfig) ||
        this.id !== other.id ||
        this.name !== other.name ||
        this.description !== other.description ||
        this.cwd !== other.cwd ||
        !deepEqual(this.command, other.command))
    ) {
      return false;
    }

    return true;
  }

  /** Deep equality, suitable for use when left and/or right may be null or undefined. */
  public static areEqual(
    left: SorbetLspConfig | undefined | null,
    right: SorbetLspConfig | undefined | null,
  ) {
    return left ? left.isEqualTo(right) : left === right;
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
export interface ISorbetWorkspaceContext extends Disposable {
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
  private cachedSorbetConfiguration;
  private readonly disposables: Disposable[];
  private readonly workspaceState: Memento;
  private readonly onDidChangeConfigurationEmitter: EventEmitter<
    ConfigurationChangeEvent
  >;

  constructor(extensionContext: ExtensionContext) {
    this.cachedSorbetConfiguration = workspace.getConfiguration("sorbet");
    this.onDidChangeConfigurationEmitter = new EventEmitter<
      ConfigurationChangeEvent
    >();
    this.workspaceState = extensionContext.workspaceState;

    this.disposables = [
      workspace.onDidChangeConfiguration((e) => {
        if (e.affectsConfiguration("sorbet")) {
          // update the cached configuration before firing
          this.cachedSorbetConfiguration = workspace.getConfiguration("sorbet");
          this.onDidChangeConfigurationEmitter.fire(e);
        }
      }),
    ];
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    Disposable.from(...this.disposables).dispose();
  }

  public get<T>(section: string, defaultValue: T): T {
    const workspaceStateValue = this.workspaceState.get<T>(`sorbet.${section}`);
    if (workspaceStateValue !== undefined) {
      return workspaceStateValue;
    }
    return this.cachedSorbetConfiguration.get(section, defaultValue);
  }

  public async update(section: string, value: any): Promise<void> {
    const key = `sorbet.${section}`;
    await this.workspaceState.update(key, value);
    this.onDidChangeConfigurationEmitter.fire({
      affectsConfiguration: () => true,
    });
  }

  public get onDidChangeConfiguration(): Event<ConfigurationChangeEvent> {
    return this.onDidChangeConfigurationEmitter.event;
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
  public async initializeEnabled(enabled: boolean): Promise<void> {
    const stateEnabled = this.workspaceState.get<boolean>("sorbet.enabled");

    if (stateEnabled === undefined) {
      const cachedConfig = this.cachedSorbetConfiguration.inspect<boolean>(
        "enabled",
      );

      if (
        cachedConfig === undefined ||
        (cachedConfig.globalValue === undefined &&
          cachedConfig.workspaceValue === undefined &&
          cachedConfig.workspaceFolderValue === undefined &&
          cachedConfig.globalLanguageValue === undefined &&
          cachedConfig.workspaceFolderLanguageValue === undefined &&
          cachedConfig.workspaceLanguageValue === undefined)
      ) {
        await this.update("enabled", enabled);
      }
    }
  }
}

export class SorbetExtensionConfig implements Disposable {
  private configFilePatterns: ReadonlyArray<string>;
  private configFileWatchers: ReadonlyArray<FileSystemWatcher>;
  private readonly disposables: Disposable[];
  private readonly onLspConfigChangeEmitter: EventEmitter<
    SorbetLspConfigChangeEvent
  >;

  private selectedLspConfigId?: string;
  private readonly sorbetWorkspaceContext: ISorbetWorkspaceContext;
  /** "Standard" LSP configs. */
  private standardLspConfigs: ReadonlyArray<SorbetLspConfig>;
  /** "Custom" LSP configs that override/supplement "standard" LSP configs. */
  private userLspConfigs: ReadonlyArray<SorbetLspConfig>;
  private wrappedEnabled: boolean;
  private wrappedHighlightUntyped: boolean;
  private wrappedRevealOutputOnError: boolean;

  constructor(sorbetWorkspaceContext: ISorbetWorkspaceContext) {
    this.configFilePatterns = [];
    this.configFileWatchers = [];
    this.onLspConfigChangeEmitter = new EventEmitter<
      SorbetLspConfigChangeEvent
    >();
    this.sorbetWorkspaceContext = sorbetWorkspaceContext;
    this.standardLspConfigs = [];
    this.userLspConfigs = [];
    this.wrappedHighlightUntyped = false;
    this.wrappedRevealOutputOnError = false;

    const workspaceFolders = this.sorbetWorkspaceContext.workspaceFolders();
    this.wrappedEnabled = workspaceFolders?.length
      ? fs.existsSync(`${workspaceFolders[0].uri.fsPath}/sorbet/config`)
      : false;

    this.disposables = [
      this.onLspConfigChangeEmitter,
      this.sorbetWorkspaceContext.onDidChangeConfiguration(() =>
        this.refresh(),
      ),
      {
        dispose: () => Disposable.from(...this.configFileWatchers).dispose(),
      },
    ];

    this.sorbetWorkspaceContext.initializeEnabled(this.wrappedEnabled);
    this.refresh();
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    Disposable.from(...this.disposables).dispose();
  }

  /**
   * Refreshes the configuration from {@link sorbetWorkspaceContext},
   * emitting change events as necessary.
   */
  private refresh(): void {
    const oldLspConfig = this.activeLspConfig;
    const oldConfigFilePatterns = this.configFilePatterns;

    this.configFilePatterns = this.sorbetWorkspaceContext.get(
      "configFilePatterns",
      this.configFilePatterns,
    );
    this.wrappedEnabled = this.sorbetWorkspaceContext.get(
      "enabled",
      this.enabled,
    );
    this.wrappedRevealOutputOnError = this.sorbetWorkspaceContext.get(
      "revealOutputOnError",
      this.revealOutputOnError,
    );
    this.wrappedHighlightUntyped = this.sorbetWorkspaceContext.get(
      "highlightUntyped",
      this.highlightUntyped,
    );

    Disposable.from(...this.configFileWatchers).dispose();
    this.configFileWatchers = this.configFilePatterns.map((pattern) => {
      const watcher = workspace.createFileSystemWatcher(pattern);
      const onConfigChange = () => {
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

    this.standardLspConfigs = this.sorbetWorkspaceContext
      .get<ISorbetLspConfig[]>("lspConfigs", [])
      .map((c) => new SorbetLspConfig(c));

    this.userLspConfigs = this.sorbetWorkspaceContext
      .get<ISorbetLspConfig[]>("userLspConfigs", [])
      .map((c) => new SorbetLspConfig(c));

    this.selectedLspConfigId = this.sorbetWorkspaceContext.get<
      string | undefined
    >("selectedLspConfigId", undefined);

    // Ensure `selectedLspConfigId` is a valid Id (not `undefined` or empty)
    if (!this.selectedLspConfigId) {
      this.selectedLspConfigId = this.lspConfigs[0]?.id;
    }

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
    [...this.userLspConfigs, ...this.standardLspConfigs].forEach((c) => {
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
  public async setSelectedLspConfigId(id: string): Promise<void> {
    await this.sorbetWorkspaceContext.update("selectedLspConfigId", id);
    this.refresh();
  }

  /**
   * Select the given `SorbetLspConfig` and enable the extension, if
   * the extension is disabled.
   *
   * This is equivalent to calling `selectedLspConfigId = id; enabled=true`.
   */
  public async setActiveLspConfigId(id: string): Promise<void> {
    await Promise.all([
      this.sorbetWorkspaceContext.update("selectedLspConfigId", id),
      this.sorbetWorkspaceContext.update("enabled", true),
    ]);
    this.refresh();
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

  public async setEnabled(b: boolean): Promise<void> {
    await this.sorbetWorkspaceContext.update("enabled", b);
    this.refresh();
  }

  public async setHighlightUntyped(b: boolean): Promise<void> {
    await this.sorbetWorkspaceContext.update("highlightUntyped", b);
    this.refresh();
  }
}
