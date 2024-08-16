import { deepEqual, deepEqualEnv } from "./utils";

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
   * Human-readable long-form description suitable for hover text or help.
   */
  readonly description: string;
  /**
   * Working directory for {@link command}.
   */
  readonly cwd: string;
  /**
   * Environment variables to set when executing {@link command}.
   */
  readonly env: NodeJS.ProcessEnv;
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
   * Human-readable long-form description suitable for hover text or help.
   */
  public readonly description: string;
  /**
   * Working directory for {@link command}.
   */
  public readonly cwd: string;
  /**
   * Environment variables to set when executing {@link command}.
   */
  public readonly env: NodeJS.ProcessEnv;
  /**
   * Command and arguments to execute, e.g. `["bundle", "exec", "srb", "typecheck", "--lsp"]`.
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
    env: NodeJS.ProcessEnv,
  );

  constructor(
    id: string,
    name: string,
    description: string,
    cwd: string,
    env: NodeJS.ProcessEnv,
    command: ReadonlyArray<string>,
  );

  constructor(
    idOrData: string | SorbetLspConfigData,
    name: string = "",
    description: string = "",
    cwd: string = "",
    env: NodeJS.ProcessEnv = {},
    command: ReadonlyArray<string> = [],
  ) {
    if (typeof idOrData === "string") {
      this.id = idOrData;
      this.name = name;
      this.description = description;
      this.cwd = cwd;
      this.env = { ...env };
      this.command = command;
    } else {
      this.id = idOrData.id;
      this.name = idOrData.name;
      this.description = idOrData.description;
      this.cwd = idOrData.cwd;
      this.env = { ...idOrData.env };
      this.command = [...idOrData.command];
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
    if (this === other) return true;
    if (!(other instanceof SorbetLspConfig)) return false;

    return (
      this.id === other.id &&
      this.name === other.name &&
      this.description === other.description &&
      this.cwd === other.cwd &&
      deepEqualEnv(this.env, other.env) &&
      deepEqual(this.command, other.command)
    );
  }

  /**
   * Deep equality, suitable for use when left and/or right may be null or undefined.
   */
  public static areEqual(
    left: SorbetLspConfig | undefined | null,
    right: SorbetLspConfig | undefined | null,
  ) {
    return left ? left.isEqualTo(right) : left === right;
  }
}
