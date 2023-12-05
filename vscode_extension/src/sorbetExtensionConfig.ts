import {
  Disposable,
  Event,
  EventEmitter,
  ExtensionContext,
  Memento,
  workspace,
} from "vscode";
import { existsSync } from "fs";
import { SorbetLspConfig, SorbetLspConfigData } from "./sorbetLspConfig";

export const SORBET_CONFIG_SECTION = "sorbet";

export type LspConfigChanged = {
  current?: string;
  previous?: string;
};

export class SorbetExtensionConfig implements Disposable {
  private readonly disposables: Disposable[];
  private readonly enabledByDefault: boolean;
  private readonly workspaceState: Memento;

  private readonly onDidChangeActiveLspConfigEmitter: EventEmitter<
    LspConfigChanged
  >;

  constructor(extensionContext: ExtensionContext) {
    // `…/sorbet/config` presence means workspace should be Sorbet-enabled by default.
    // Note this check does not handle multi-root workspaces correctly in general case.
    this.enabledByDefault =
      !!workspace.workspaceFolders?.length &&
      existsSync(`${workspace.workspaceFolders[0].uri.fsPath}/sorbet/config`);

    this.onDidChangeActiveLspConfigEmitter = new EventEmitter();
    this.workspaceState = extensionContext.workspaceState;

    this.disposables = [
      this.onDidChangeActiveLspConfigEmitter,
      workspace.onDidChangeConfiguration((e) => {
        if (e.affectsConfiguration(SORBET_CONFIG_SECTION)) {
          this.refresh();
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

  /**
   * Whether the Sorbet LSP should be enabled.
   */
  public get enabled(): boolean {
    return this.getConfigValue("enabled", this.enabledByDefault);
  }

  /**
   * An event that is emitted when the {@link getActiveLspConfig} config has changed.
   */
  public get onDidChangeActiveLspConfig(): Event<LspConfigChanged> {
    return this.onDidChangeActiveLspConfigEmitter.event;
  }

  /**
   * Get the active {@link SorbetExtensionConfig configuration}.
   *
   * If not {@link enabled}, returns `undefined`, otherwise it is equivalent to
   * call {@link getSelectedLspConfig getSelectedLspConfig(true)}.
   */
  public getActiveLspConfig(): SorbetLspConfig | undefined {
    return this.enabled ? this.getSelectedLspConfig(true) : undefined;
  }

  /**
   * Available {@link SorbetLspConfig configurations} for Sorbet LSP.
   *
   * This method combines {@link lspConfigs} and {@link userLspConfigs} such as
   * configurations on the latter overrides ones on the former based on Id.
   */
  public getLspConfigs(): ReadonlyArray<SorbetLspConfig> {
    // For low config count, as expected, linear search is faster than hashing
    // so no point on creating maps. This also ensures user's are listed first.
    const results = [...this.userLspConfigs];
    this.lspConfigs.forEach((c) => {
      if (!results.find((r) => r.id === c.id)) {
        results.push(c);
      }
    });
    return results;
  }

  /**
   * Selected {@link SorbetLspConfig LSP configuration}.
   * @param defaultToFirst Default to first {@link SorbetLspConfig config} if no
   * explicit selection has been made.
   */
  public getSelectedLspConfig(
    defaultToFirst = true,
  ): SorbetLspConfig | undefined {
    let selectedConfig: SorbetLspConfig | undefined;

    const configs = this.getLspConfigs();
    const currentConfigId = this.selectedLspConfigId;

    if (currentConfigId) {
      selectedConfig = configs.find((c) => c.id === currentConfigId);
    }
    if (!selectedConfig && defaultToFirst) {
      [selectedConfig] = configs;
    }
    return selectedConfig;
  }

  /**
   * Whether `untyped` code should be highlighted on editors.
   */
  public get highlightUntyped(): boolean {
    return this.getConfigValue("highlightUntyped", false);
  }

  /**
   * LSP {@link SorbetLspConfig configurations}.
   */
  public get lspConfigs(): ReadonlyArray<SorbetLspConfig> {
    return this.getLspConfigsConfigValue("lspConfigs");
  }

  /**
   * Whether the `Sorbet` panel should be revealed on error.
   */
  public get revealOutputOnError(): boolean {
    return this.getConfigValue("revealOutputOnError", false);
  }

  /**
   * Selected {@link SorbetLspConfig.id configuration Id}.
   */
  public get selectedLspConfigId(): string | undefined {
    return this.getWorkspaceStateValue<string | undefined>(
      "selectedLspConfigId",
      undefined,
    );
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

  private refresh(previousActiveId?: string) {
    const activeId = this.getActiveLspConfig()?.id;
    if (activeId !== previousActiveId) {
      this.onDidChangeActiveLspConfigEmitter.fire({
        current: activeId,
        previous: previousActiveId,
      });
    }
  }

  /**
   * Selects the given `SorbetLspConfig` and enables the extension, if
   * the extension is disabled.
   *
   * This is equivalent to calling {@link setSelectedLspConfig} followed by
   * {@link setEnabled setEnabled(true)} but it will only lead to a single
   * change event being raised.
   */
  public async setActiveLspConfigId(
    configOrId: string | SorbetLspConfig | undefined,
  ): Promise<void> {
    const configs = this.getLspConfigs();
    const config =
      typeof configOrId === "string"
        ? configs.find((c) => c.id === configOrId)
        : configOrId;
    const enabled = !!config;

    // Updating directly to prevent multiple refresh events.
    await Promise.all([
      this.updateWorkspaceStateValue("selectedLspConfigId", config?.id),
      this.updateConfigValue("enabled", enabled),
    ]);
    // this.refresh();
  }

  public async setEnabled(enabled: boolean): Promise<void> {
    // Changing enabled actually affects the active config
    const previous = this.getActiveLspConfig()?.id;
    await this.updateConfigValue("enabled", enabled);
    this.refresh(previous);
  }

  public async setHighlightUntyped(highlight: boolean): Promise<void> {
    await this.updateConfigValue("highlightUntyped", highlight);
    // this.refresh();
  }

  public async setRevealOutputOnError(reveal: boolean): Promise<void> {
    await this.updateConfigValue("revealOutputOnError", reveal);
    // this.refresh();
  }

  /**
   * Selects the given `SorbetLspConfig`.
   */
  public async setSelectedLspConfig(
    configOrId: string | SorbetLspConfig | undefined,
  ): Promise<void> {
    const configs = this.getLspConfigs();
    const config = configs.find(
      typeof configOrId === "string"
        ? (c) => c.id === configOrId
        : (c) => c.isEqualTo(configOrId), // ensure it is known
    );

    const currentConfigId = this.selectedLspConfigId;
    if (currentConfigId !== config?.id) {
      const previousActiveId = this.getActiveLspConfig()?.id;
      await this.updateWorkspaceStateValue("selectedLspConfigId", config?.id);
      this.refresh(previousActiveId);
    }
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
    // Cannot deserialize `SorbetLspConfig` directly from settings, so use the
    // data interface for clarity and instantiate proper class later.
    const configs = workspace
      .getConfiguration(SORBET_CONFIG_SECTION)
      .get<ReadonlyArray<SorbetLspConfigData>>(name, []);

    return configs.map((config) => new SorbetLspConfig(config));
  }

  private getWorkspaceStateValue<T>(name: string, defaultValue: T): T {
    return this.workspaceState.get(name, defaultValue);
  }

  private async updateConfigValue<T>(name: string, value?: T): Promise<void> {
    await workspace.getConfiguration(SORBET_CONFIG_SECTION).update(name, value);
  }

  private async updateWorkspaceStateValue<T>(
    name: string,
    value?: T,
  ): Promise<void> {
    await this.workspaceState.update(name, value);
  }
}
