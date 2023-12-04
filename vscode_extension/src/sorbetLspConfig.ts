import { deepEqual } from "./utils";

export class SorbetLspConfig {
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

  constructor(
    id: string,
    name: string,
    description: string = "",
    cwd: string = "",
    command: ReadonlyArray<string> = [],
  ) {
    this.id = id;
    this.name = name;
    this.description = description;
    this.cwd = cwd;
    this.command = command;
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
