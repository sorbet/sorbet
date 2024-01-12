import {
  ConfigurationChangeEvent,
  Disposable,
  Event,
  EventEmitter,
  ExtensionContext,
  FileSystemWatcher,
  Memento,
  workspace,
} from "vscode";
import * as fs from "fs";
import { Log } from "./log";
import { SorbetLspConfig, SorbetLspConfigData } from "./sorbetLspConfig";
import { deepEqual } from "./utils";

export type TrackUntyped = "nowhere" | "everywhere-but-tests" | "everywhere";

export const ALL_TRACK_UNTYPED: TrackUntyped[] = [
  "nowhere",
  "everywhere-but-tests",
  "everywhere",
];

function coerceTrackUntypedSetting(value: boolean | string): TrackUntyped {
  switch (value) {
    case true:
      return "everywhere";
    case false:
      return "nowhere";
    case "nowhere":
    case "everywhere-but-tests":
    case "everywhere":
      return value;
    default:
      return "nowhere";
  }
}

export function labelForTrackUntypedSetting(value: TrackUntyped): string {
  switch (value) {
    case "nowhere":
      return "Nowhere";
    case "everywhere-but-tests":
      return "Everywhere but tests";
    case "everywhere":
      return "Everywhere";
    default:
      const unexpected: never = value;
      throw new Error(`Unexpected value: ${unexpected}`);
  }
}

export function backwardsCompatibleTrackUntyped(
  log: Log,
  trackWhere: TrackUntyped,
): boolean | TrackUntyped {
  switch (trackWhere) {
    case "nowhere":
      return false;
    case "everywhere":
      return true;
    case "everywhere-but-tests":
      return trackWhere;
    default:
      const exhaustiveCheck: never = trackWhere;
      log.warning(`Got unexpected state: ${exhaustiveCheck}`);
      return false;
  }
}

export interface SorbetLspConfigChangeEvent {
  readonly oldLspConfig?: SorbetLspConfig;
  readonly newLspConfig?: SorbetLspConfig;
}

/**
 * Combines references to `extensionContext.workspaceState()`,
 * `workspace.getConfiguration("sorbet")`, and
 * `workspace.onDidChangeConfiguration` for the "sorbet" section
 * to make it easier to stub out behavior in tests.
 */
export interface ISorbetWorkspaceContext extends Disposable {
  /**
   * Get value from {@link ExtensionContext.workspaceState}, if not defined
   * fallback to {@link workspace.getConfiguration}, otherwise `defaultValue`.
   * @param name Setting name (do not include 'sorbet' prefix).
   * @param defaultValue Value to use if no datastore defines the value.
   */
  get<T>(name: string, defaultValue: T): T;

  /**
   * Set value. Value is saved to {@link extensionContext.workspaceState} unless it
   * matches state in {@link workspace.getConfiguration} in which case it is removed
   * (effectively setting `value` due to {@link get}'s fallback logic). One caveat is
   * that using `undefined` only resets {@link extensionContext.workspaceState}.
   * @param name Setting name (do not include 'sorbet' prefix).
   * @param value Setting value.
   */
  update(name: string, value: any): Promise<void>;

  /**
   * An event emitted when configuration has changed.
   */
  onDidChangeConfiguration: Event<ConfigurationChangeEvent>;

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
    this.onDidChangeConfigurationEmitter = new EventEmitter();
    this.workspaceState = extensionContext.workspaceState;

    this.disposables = [
      this.onDidChangeConfigurationEmitter,
      workspace.onDidChangeConfiguration((e) => {
        if (e.affectsConfiguration("sorbet")) {
          // update the cached configuration before firing
          this.cachedSorbetConfiguration = workspace.getConfiguration("sorbet");
          this.onDidChangeConfigurationEmitter.fire(e);
        }
      }),
    ];
  }

  public dispose() {
    Disposable.from(...this.disposables).dispose();
  }

  public get<T>(section: string, defaultValue: T): T {
    const stateKey = `sorbet.${section}`;
    return (
      this.workspaceState.get<T>(stateKey) ??
      this.cachedSorbetConfiguration.get(section, defaultValue)
    );
  }

  public async update(section: string, value: any): Promise<void> {
    const stateKey = `sorbet.${section}`;

    const configValue = this.cachedSorbetConfiguration.get(section, value);
    if (configValue === value) {
      // Remove value from state since configuration's is enough.
      await this.workspaceState.update(stateKey, undefined);
    } else {
      // Save to state since it is being customized.
      await this.workspaceState.update(stateKey, value);
    }

    this.onDidChangeConfigurationEmitter.fire({
      affectsConfiguration: (section: string) =>
        /"^sorbet($|\.)"/.test(section),
    });
  }

  public get onDidChangeConfiguration(): Event<ConfigurationChangeEvent> {
    return this.onDidChangeConfigurationEmitter.event;
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
  private wrappedHighlightUntyped: TrackUntyped;
  private wrappedTypedFalseCompletionNudges: boolean;
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
    this.wrappedHighlightUntyped = "nowhere";
    this.wrappedTypedFalseCompletionNudges = true;
    this.wrappedRevealOutputOnError = false;

    // Any workspace with a `…/sorbet/config` file is considered Sorbet-enabled
    // by default. This implementation does not work in the general case with
    // multi-root workspaces.
    const { workspaceFolders } = workspace;
    this.wrappedEnabled =
      !!workspaceFolders?.length &&
      fs.existsSync(`${workspaceFolders[0].uri.fsPath}/sorbet/config`);

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
    const highlightUntyped = this.sorbetWorkspaceContext.get(
      "highlightUntyped",
      this.highlightUntyped,
    );
    // Always store the setting as a TrackUntyped enum value internally.
    // We'll convert it to legacy-style boolean options (potentially) at the call sites.
    this.wrappedHighlightUntyped = coerceTrackUntypedSetting(highlightUntyped);
    this.wrappedTypedFalseCompletionNudges = this.sorbetWorkspaceContext.get(
      "typedFalseCompletionNudges",
      this.typedFalseCompletionNudges,
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
      .get<SorbetLspConfigData[]>("lspConfigs", [])
      .map((c) => new SorbetLspConfig(c));

    this.userLspConfigs = this.sorbetWorkspaceContext
      .get<SorbetLspConfigData[]>("userLspConfigs", [])
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
   * Get the active {@link SorbetLspConfig LSP config}.
   *
   * A {@link selectedLspConfig selected} config is only active when {@link enabled}
   * is `true`.
   */
  public get activeLspConfig(): SorbetLspConfig | undefined {
    return this.enabled ? this.selectedLspConfig : undefined;
  }

  public get enabled(): boolean {
    return this.wrappedEnabled;
  }

  public get highlightUntyped(): TrackUntyped {
    return this.wrappedHighlightUntyped;
  }

  public oldHighlightUntyped: TrackUntyped | undefined = undefined;

  /**
   * Returns a copy of the current SorbetLspConfig objects.
   */
  public get lspConfigs(): ReadonlyArray<SorbetLspConfig> {
    const results: Array<SorbetLspConfig> = [];
    const resultIds = new Set<string>();
    [...this.userLspConfigs, ...this.standardLspConfigs].forEach((c) => {
      if (!resultIds.has(c.id)) {
        results.push(c);
        resultIds.add(c.id);
      }
    });
    return results;
  }

  public get revealOutputOnError(): boolean {
    return this.wrappedRevealOutputOnError;
  }

  /**
   * Get the currently selected {@link SorbetLspConfig LSP config}.
   *
   * Returns `undefined` if {@link selectedLspConfigId} has not been set or if
   * its value does not map to a config in {@link lspConfigs}.
   */
  public get selectedLspConfig(): SorbetLspConfig | undefined {
    return this.lspConfigs.find((c) => c.id === this.selectedLspConfigId);
  }

  public get typedFalseCompletionNudges(): boolean {
    return this.wrappedTypedFalseCompletionNudges;
  }

  /**
   * Set active {@link SorbetLspConfig LSP config}.
   *
   * If {@link enabled} is `false`, this will change it to `true`.
   */
  public async setActiveLspConfigId(id: string): Promise<void> {
    const updates: Array<Thenable<void>> = [];

    if (this.activeLspConfig?.id !== id) {
      updates.push(
        this.sorbetWorkspaceContext.update("selectedLspConfigId", id),
      );
    }
    if (!this.enabled) {
      updates.push(this.sorbetWorkspaceContext.update("enabled", true));
    }

    if (updates.length) {
      await Promise.all(updates);
      this.refresh();
    }
  }

  public async setEnabled(enabled: boolean): Promise<void> {
    await this.sorbetWorkspaceContext.update("enabled", enabled);
    this.refresh();
  }

  public async setHighlightUntyped(trackWhere: TrackUntyped): Promise<void> {
    await this.sorbetWorkspaceContext.update("highlightUntyped", trackWhere);
    this.refresh();
  }

  /**
   * Set selected {@link SorbetLspConfig LSP config}.
   *
   * This does not change {@link enabled} state.
   */
  public async setSelectedLspConfigId(id: string): Promise<void> {
    if (this.selectedLspConfigId !== id) {
      await this.sorbetWorkspaceContext.update("selectedLspConfigId", id);
      this.refresh();
    }
  }

  public async setTypedFalseCompletionNudges(enabled: boolean): Promise<void> {
    await this.sorbetWorkspaceContext.update(
      "typedFalseCompletionNudges",
      enabled,
    );
    this.refresh();
  }
}
