import { deepEqual } from "./utils";

/**
 * Sorbet LSP configuration (data-only).
 */
export interface SorbetLspConfigData {
  /**
   * Configuration Id.
   */
  readonly id: string;
  /**
   * Display name suitable for short-form fields like menu items or status fields.
   */
  readonly name: string;
  /**
   * Human-readable zlong-form description suitable for hover text or help.
   */
  readonly description: string;
  /**
   * Working directory for {@link command}.
   */
  readonly cwd: string;
  /**
   * Command and arguments to execute, e.g. `["srb", "typecheck", "--lsp"]`.
   */
  readonly command: ReadonlyArray<string>;
}

/**
 * Sorbet LSP configuration.
 */
export class SorbetLspConfig implements SorbetLspConfigData {
  /**
   * Configuration Id.
   */
  public readonly id: string;
  /**
   * Display name suitable for short-form fields like menu items or status fields.
   */
  public readonly name: string;
  /**
   * Human-readable zlong-form description suitable for hover text or help.
   */
  public readonly description: string;
  /**
   * Working directory for {@link command}.
   */
  public readonly cwd: string;
  /**
   * Command and arguments to execute, e.g. `["srb", "typecheck", "--lsp"]`.
   */
  public readonly command: ReadonlyArray<string>;

  constructor(data: SorbetLspConfigData);

  constructor(id: string, name: string);
  constructor(id: string, name: string, description: string);
  constructor(id: string, name: string, description: string, cwd: string);
  constructor(
    id: string,
    name: string,
    description: string,
    cwd: string,
    command: ReadonlyArray<string>,
  );

  constructor(
    idOrData: string | SorbetLspConfigData,
    name: string = "",
    description: string = "",
    cwd: string = "",
    command: ReadonlyArray<string> = [],
  ) {
    if (typeof idOrData === "string") {
      this.id = idOrData;
      this.name = name;
      this.description = description;
      this.cwd = cwd;
      this.command = command;
    } else {
      this.id = idOrData.id;
      this.name = idOrData.name;
      this.description = idOrData.description;
      this.cwd = idOrData.cwd;
      this.command = idOrData.command;
    }
  }

  public toString(): string {
    return `${this.name}: ${this.description} [cmd: "${this.command.join(
      " ",
    )}"]`;
  }

  /**
   * Deep equality.
   */
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
}
