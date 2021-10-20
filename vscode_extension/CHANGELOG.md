# Version history

## 0.3.6

## 0.3.5

- Restarting the Sorbet language server on changes to files in `sorbet.configFilePatterns`.
- Upgrades vscode-languageclient to prevent code actions from blocking save when client is configured to apply eslint fixups.

## 0.3.4

- Remove internal references

## 0.3.3

- Add client-side latency metrics for common Sorbet requests (hover, autocomplete, etc).

## 0.3.2

- Add setting for `sorbet.userLspConfigs`.

## 0.3.1

Fix plugin documentation, add this changelog

## 0.3.0

- Revamp Sorbet's configuration to support multiple configurations
- Change limited-retry of launching Sorbet language server (fail after 10 attempts) to perpetual retry so that when a language server becomes available, the Sorbet extension quickly auto-reconnects.
