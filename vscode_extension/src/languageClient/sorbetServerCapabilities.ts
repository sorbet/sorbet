import { ServerCapabilities } from "vscode-languageserver";

export type SorbetServerCapabilities = ServerCapabilities & {
  sorbetShowSymbolProvider: boolean;
};
