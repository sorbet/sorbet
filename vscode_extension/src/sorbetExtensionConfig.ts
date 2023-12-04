import {
  ConfigurationChangeEvent,
  Disposable,
  Event,
  EventEmitter,
  workspace,
} from "vscode";
import { existsSync } from "fs";
import { SorbetLspConfig } from "./sorbetLspConfig";

export const SORBET_CONFIG_SECTION = "sorbet";

export type LspConfigChanged = {
  current?: SorbetLspConfig;
  previous?: SorbetLspConfig;
};

export class SorbetExtensionConfig implements Disposable {
  private readonly disposables: Disposable[];
  private readonly enabledDefault: boolean;

  private readonly onDidChangeConfigurationEmitter: EventEmitter<
    ConfigurationChangeEvent
  >;

  private readonly onDidChangeLspConfigEmitter: EventEmitter<LspConfigChanged>;

  constructor() {
    this.onDidChangeConfigurationEmitter = new EventEmitter();
    this.onDidChangeLspConfigEmitter = new EventEmitter();

    const { workspaceFolders } = workspace;
    this.enabledDefault =
      !!workspaceFolders?.length &&
      existsSync(`${workspaceFolders[0].uri.fsPath}/sorbet/config`);

    this.disposables = [
      this.onDidChangeConfigurationEmitter,
      this.onDidChangeLspConfigEmitter,
    ];

    workspace.onDidChangeConfiguration((e) => {
      if (e.affectsConfiguration(SORBET_CONFIG_SECTION)) {
        this.onDidChangeConfigurationEmitter.fire(e);
      }
    }, this.disposables);
  }

  /**
   * Dispose and free associated resources.
   */
  public dispose() {
    Disposable.from(...this.disposables).dispose();
  }

  /**
   * Whether the LSP should be enabled.
   */
  public get enabled(): boolean {
    return this.getConfigValue("enabled", this.enabledDefault);
  }

  /**
   * Returns the active {@link SorbetExtensionConfig configuration}.
   *
   * If not {@link enabled}, this returns `undefined`
   *
   * If the Sorbet extension is disabled, returns `null`, otherwise
   * returns a `SorbetLspConfig` or `undefined` as per `selectedLspConfig`.
   */
  public get activeLspConfig(): SorbetLspConfig | null | undefined {
    return this.enabled ? this.selectedLspConfig : null;
  }

  /**
   * An event that is emitted when the {@link SorbetExtensionConfig configuration} has changed.
   */
  public get onDidChangeConfiguration(): Event<ConfigurationChangeEvent> {
    return this.onDidChangeConfigurationEmitter.event;
  }

  /**
   * An event that is emitted when the {@link activeLspConfig} config has changed.
   */
  public get onDidChangeLspConfig(): Event<LspConfigChanged> {
    return this.onDidChangeLspConfigEmitter.event;
  }

  /**
   * Available {@link SorbetLspConfig configurations} for Sorbet LSP.
   *
   * This method combines {@link lspConfigs} and {@link userLspConfigs} such as
   * configurations on the latter overrides ones on the former based on Id.
   */
  public getLspConfigs(): ReadonlyArray<SorbetLspConfig> {
    const results = new Map<string, SorbetLspConfig>([
      ...this.lspConfigs.map<[string, SorbetLspConfig]>((c) => [c.id, c]),
      // Override default configs with user's
      ...this.userLspConfigs.map<[string, SorbetLspConfig]>((c) => [c.id, c]),
    ]);

    return [...results.values()];
  }

  /**
   * Whether `untyped` code should be highlighted on editors.
   */
  public get highlightUntyped(): boolean {
    return this.getConfigValue("highlightUntyped", false);
  }

  /**
   * Whether the `Sorbet` panel should be revealed on error.
   */
  public get revealOutputOnError(): boolean {
    return this.getConfigValue("revealOutputOnError", false);
  }

  /**
   * Selected {@link SorbetLspConfig LSP configuration}.
   *
   * If the configuration does not specify a `selectedLspConfigId`, or if
   * the `id` refers to a `SorbetLspConfig` that does not exist, return `undefined`.
   */
  public get selectedLspConfig(): SorbetLspConfig | undefined {
    const selectedId = this.selectedLspConfigId;
    return selectedId
      ? this.getLspConfigs().find((c) => c.id === this.selectedLspConfigId)
      : undefined;
  }

  /**
   * Selected LSP {@link SorbetLspConfig configuration} Id.
   */
  public get selectedLspConfigId(): string | undefined {
    return this.getConfigValue<string | undefined>(
      "selectedLspConfigId",
      undefined,
    );
  }

  /**
   * LSP {@link SorbetLspConfig configurations}.
   */
  public get lspConfigs(): ReadonlyArray<SorbetLspConfig> {
    return this.getLspConfigsConfigValue("lspConfigs");
  }

  /**
   * User-defined LSP {@link SorbetLspConfig configurations} that override or
   * supplement {@link lspConfigs}.
   *
   * Because VS Code overrides User settings with Workspace-defined ones, this
   * exists so users can do the opposite. This is a candidate for deprecation
   * since change is not intended to be checked-in regardless nad adds
   */
  public get userLspConfigs(): ReadonlyArray<SorbetLspConfig> {
    return this.getLspConfigsConfigValue("userLspConfigs");
  }

  public get typedFalseCompletionNudges(): boolean {
    return this.getConfigValue("typedFalseCompletionNudges", true);
  }

  /**
   * Selects the given `SorbetLspConfig` and enable the extension, if
   * the extension is disabled.
   *
   * This is equivalent to calling `selectedLspConfigId = id; enabled=true`.
   */
  public async setActiveLspConfigId(id: string): Promise<void> {
    const config = this.getLspConfigs().find((c) => c.id === id);
    const enabled = !!config;

    // not calling `setEnabled` to prevent duplicated events
    await Promise.all([
      this.updateConfigValue("selectedLspConfigId", config?.id),
      this.updateConfigValue("enabled", enabled),
    ]);
    // this.refresh();
  }

  public async setEnabled(enabled: boolean): Promise<void> {
    await this.updateConfigValue("enabled", enabled);
    // this.refresh();
  }

  public async setHighlightUntyped(highlight: boolean): Promise<void> {
    await this.updateConfigValue("highlightUntyped", highlight);
    // this.refresh();
  }

  public async setRevealOutputOnError(reveal: boolean): Promise<void> {
    await this.updateConfigValue("revealOutputOnError", reveal);
    // this.refresh();
  }

  public async setTypedFalseCompletionNudges(nudge: boolean): Promise<void> {
    await this.updateConfigValue("typedFalseCompletionNudges", nudge);
    // this.refresh();
  }

  private getConfigValue<T>(name: string, defaultValue: T): T {
    return workspace
      .getConfiguration(SORBET_CONFIG_SECTION)
      .get(name, defaultValue);
  }

  private getLspConfigsConfigValue(
    name: string,
  ): ReadonlyArray<SorbetLspConfig> {
    // BEWARE: JSON deserialization does not know how to instantiate `SorbetLspConfig`
    // objects, it can only hydrate `SorbetLspConfig`-shaped hashes.
    const pureDataConfigs = workspace
      .getConfiguration(SORBET_CONFIG_SECTION)
      .get<ReadonlyArray<SorbetLspConfig>>(name, []);

    return pureDataConfigs.map(
      (c) => new SorbetLspConfig(c.id, c.name, c.description, c.cwd, c.command),
    );
  }

  private async updateConfigValue<T>(name: string, value?: T): Promise<void> {
    await workspace.getConfiguration(SORBET_CONFIG_SECTION).update(name, value);
  }
}
