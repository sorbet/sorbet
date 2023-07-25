# Version history
## 0.3.24
- Disable the `Copy Symbol to Clipboard` command when there is a text selection.

## 0.3.23
- Fix: Sorbet extension fails when opening a project containing Ruby code but no active configuration.

## 0.3.22
- Fix: 0.3.21 introduced an unintended behavior change: empty configurations Ids are treated the same as stale ones, causing Sorbet to stay in `Disabled` state until a new configuration is selected.

## 0.3.21
- Provide UI hints on state of untyped code highlighting.
- Internal code clean-up.

## 0.3.20
- `Sorbet` status bar item shows a quick-pick drop down instead of a notification dialog when clicked.
- `Sorbet: Set Log Level…` command allows to control what messages are logged to the `Sorbet` output pane. 
  - Defaults to `info` and does not persist between sessions.
  - The `VSCODE_SORBETEXT_LOG_LEVEL` environment variable, if defined, changes the initial value. Use one of these values: `trace`, `debug`, `info`, `warning`, `error` (case-insensitive).

## 0.3.7

- Add extension icon.

## 0.3.6

- First open source release!
