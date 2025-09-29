# Version history

## 0.3.46
- Exclude Ruby files outside the target workspace from triggering IntelliSense events.

## 0.3.45
- `Copy Symbol to Clipboard` is enabled and visible only when a workspace is open.

## 0.3.44
- Allow configuring the diagnostic severity used to highlight untyped code. See [the docs](https://sorbet.org/docs/highlight-untyped) for more.

## 0.3.43
- Remove use of SIGINT when terminating Sorbet.

## 0.3.42
- Use VS Code's standard `$(sync)` icon on status bar.

## 0.3.41
- New extension API to track status changes on the Language Service.

## 0.3.40
- Upgrade VS Language Client to [protocol 3.17.3](https://github.com/microsoft/vscode-languageserver-node/blob/main/README.md#3173-protocol-810-json-rpc-810-client-and-810-server)
- Remove `cwd` from Sorbet configuration (it has never been used).

## 0.3.39
- Fix: `Copy Symbol to Clipboard` fails to be enabled in remote devboxes.

## 0.3.38
- Sorbet can be disabled while in `Restarting` / `Initializing` loop.
- `Copy Symbol to Clipboard` is enabled only for supported `file` and `sorbet` URI schemes.

## 0.3.26
- Add option to toggle the auto-complete nudge in `typed: false` files

## 0.3.25
- Toggling highlighting of untyped code does not require a restart now
  - Sends workspace/didChangeConfiguration notification to LSP server instead

## 0.3.24
- `Copy Symbol to Clipboard`
  - Disable command when there is a text selection.
  - Show a progress dialog when Sorbet is not ready to process commands.
- Auto-save `__package.rb` files when edited by a quickfix.

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
