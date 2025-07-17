---
id: vscode
title: Extension for Visual Studio Code
sidebar_label: Visual Studio Code
---

The [Sorbet extension for Visual Studio Code](https://marketplace.visualstudio.com/items?itemName=sorbet.sorbet-vscode-extension) integrates with the [Sorbet language server](lsp.md) to provide IDE-like features for typed Ruby files.

This doc covers certain VS Code-specific points, like how to install and configure the VS Code extension. Most of Sorbet's editor features are not VS Code-specific, and are documented in their own pages (see the "Editor Features" section of the page listing).

## Installing and enabling the extension

Install [the Sorbet extension](https://marketplace.visualstudio.com/items?itemName=sorbet.sorbet-vscode-extension) from the VS Code extension marketplace.

As long as the current project has a [`sorbet/config`](cli.md#config-file) file, Sorbet should start automatically whenever a Ruby file is open. To check that Sorbet has started working, look for "**Sorbet: ...**" in the VS Code status bar.

As long as it does not say "Sorbet: Disabled," then Sorbet is working. See [Server Status](server-status.md) for more information on what the statuses like "Idle" mean.

![](/img/sorbet-idle.png)

### Troubleshooting startup

#### I'm not seeing "Sorbet: ..." in the status bar

The extension is not active. Here are some steps to try:

- Is a Ruby file open? The Sorbet extension does not activate until at least one Ruby file is open.

- Make sure the VS Code window is wide enough to display the entire contents of the status bar.

- Do not use VS Code's [multi-root workspaces](https://code.visualstudio.com/docs/editor/multi-root-workspaces). The Sorbet extension does not support this feature.

#### I see "Sorbet: Disabled" in the status bar

Sorbet is installed and active, but has chosen not to start the Sorbet process automatically. To manually enable Sorbet, either:

- Use the [`>Sorbet: Enable`] command. The server will start with [the default server command](#sorbetlspconfigs).

- Use the [`>Sorbet: Configure`] command. A prompt will appear to choose which server configuration to start the Sorbet process with. See [the configuration docs](#sorbetlspconfigs) to change the configurations.

- Edit the `.vscode/settings.json` for the current workspace to include this:

  ```json
  {
    "sorbet.enabled": true
  }
  ```

  This may require restarting VS Code.

#### Sorbet keeps restarting

The Sorbet extension restarts the server process when it stops. If the process crashes on startup, this can cause Sorbet to restart in a loop. Fixing requires fixing the underlying reason for the crash.

Click on "Sorbet: ..." in the status bar and then choose "View Output" (or use the [`>Sorbet: Show Output`] command) to see the Sorbet extension logs.

![](/img/view-output.png)

There is usually an error message which gives a hint why Sorbet crashed. For example:

```
[Error - 9:36:32 AM] Connection to server got closed. Server will not be restarted.
Running Sorbet LSP with:
    bundle exec srb typecheck --lsp
Could not locate Gemfile or .bundle/ directory
```

See below for help with specific error messages.

Note that the "Server will not be restarted" message is logged by a library that the Sorbet VS Code extension uses—the actual restart logic lives at a higher level, so this error message is misleading. Sorbet **will** be restarted whenever the server process crashes.

#### "Could not locate Gemfile or .bundle/ directory"

The Sorbet extension uses this command to start the server process by default:

```
bundle exec srb typecheck --lsp
```

This assumes the project uses [Bundler](https://bundler.io/) to manage Ruby dependencies. If that's not the case, [configure how to launch](#sorbetlspconfigs) the Sorbet process.

#### "bundler: command not found: srb"

Either:

- The `Gemfile` does not include the `sorbet` gem

  Edit the `Gemfile` as per [the instructions here](adopting.md), and then run `bundle install`.

- The `sorbet` gem is included in the Gemfile, but not yet installed.

  Run `bundle install`, then try again.

#### "Sorbet's language server requires at least one input directory"

Sorbet in LSP mode requires a single input directory.

See [Sorbet Language Server > Prerequisites > A single input directory](lsp.md#a-single-input-directory) for more.

#### "Sorbet's language server requires a single input directory"

Sorbet in LSP mode requires a single input directory.

See [Sorbet Language Server > Prerequisites > A single input directory](lsp.md#a-single-input-directory) for more.

#### "Watchman is required for Sorbet to detect changes to files"

For the best experience, Sorbet requires [Watchman](https://facebook.github.io/watchman/), which listens for changes to the files on disk in addition to edits that happen to files open in the editor.

See [Sorbet Language Server > Prerequisites > Watchman](lsp.md#watchman) for more.

#### Something else

If you had to troubleshoot an error message you think is common when installing Sorbet, please help by [editing this page](https://github.com/sorbet/sorbet/edit/master/website/docs/sorbet-uris.md)!

## Disabling the extension

There are multiple ways to disable the Sorbet extension, which will stop the Sorbet server process:

- Use the [`>Sorbet: Disable`] command.

- Click on "Sorbet: ..." in the status bar and then choose "Disable Sorbet."

- Edit the `.vscode/settings.json` for the current workspace to include this:

  ```json
  {
    "sorbet.enabled": false
  }
  ```

  This is particularly useful to disable Sorbet for all contributors to a given project, if there's some reason why Sorbet should not be used in that project.

## Command Reference

To run these commands, use VS Code's [Command Palette](https://code.visualstudio.com/docs/getstarted/userinterface#_command-palette). To bring up the command palette, use ⇧⌘P on macOS or Ctrl + Shift + P on Linux.

The `>` character in the command names below does not need to be typed, unless it's been deleted from the text box, or if the palette was opened with the "Quick Open" menu (⌘P on macOS, Ctrl + P on Linux.)

### `>Sorbet: Enable`

[`>Sorbet: Enable`]: #sorbet-enable

Enables Sorbet, causing the Sorbet server process to start. It will use the most recently selected [server launch configuration](#sorbetlspconfigs), or else the default.

### `>Sorbet: Disable`

[`>Sorbet: Disable`]: #sorbet-disable

Disables Sorbet, causing the Sorbet server process to stop.

### `>Sorbet: Restart`

[`>Sorbet: Restart`]: #sorbet-restart

Disables Sorbet, causing the Sorbet server process to stop, then re-enables Sorbet, causing it to launch again, using the current server configuration.

### `>Sorbet: Configure`

[`>Sorbet: Configure`]: #sorbet-configure

Chooses which configuration to run Sorbet with, or choose to disable Sorbet. After choosing a configuration, any running Sorbet process will stop, and a new one will be launched with the chosen configuration.

By default, [there are three configurations](#sorbetlspconfigs), which allow opting into experimental features. A given workspace may choose to define its own configurations.

![](/img/configure.png)

### `>Sorbet: Show Output`

[`>Sorbet: Show Output`]: #sorbet-show-output

Opens the Sorbet extension's logs in the output pane. Mostly useful for [Troubleshooting startup](#troubleshooting-startup). For debugging the Sorbet extension itself, consider also using [`>Sorbet: Set Log Level...`] to make the logs more verbose.

Note that the Sorbet output **only** includes logs from the Sorbet extension. It does not include debug output from the Sorbet server process. To control log output from the Sorbet server process, see the [`--debug-log-file`](cli-ref.md#debugging-options) command line option. This flag is not passed by default. To pass it, change the [server configurations](#sorbetlspconfigs).

### `>Sorbet: Set Log Level...`

[`>Sorbet: Set Log Level...`]: #sorbet-set-log-level

The Sorbet extension includes various logs designed to help developers troubleshoot problems in the Sorbet extension itself. Use this command to control how verbose Sorbet's log should be. It may also be helpful to enable [logging at VS Code's language client level](https://github.com/sorbet/sorbet/blob/master/docs/lsp-dev-guide.md#by-trying-it-in-a-client).

### `>Sorbet: Copy Symbol to Clipboard`

[`>Sorbet: Copy Symbol to Clipboard`]: #sorbet-copy-symbol-to-clipboard

Copies the fully-qualified name of the symbol under the cursor to the clipboard.

This feature is powered by a custom extension to the LSP specification: [`sorbet/showSymbol` request](lsp.md#sorbetshowsymbol-request).

The same feature is also available as a right-click context menu item:

<video autoplay loop muted playsinline style="max-width: 597px;">
  <source src="/img/copy-symbol.mp4" type="video/mp4">
</video>

### `>Sorbet: Configure untyped code highligting`

[`>Sorbet: Configure untyped code highligting`]: #sorbet-configure-untyped-code-highlighting

See [Highlighting untyped code](highlight-untyped.md)

### `>Sorbet: Toggle highlighting untyped code`

[`>Sorbet: Toggle highlighting untyped code`]: #sorbet-toggle-highlighting-untyped-code

See [Highlighting untyped code](highlight-untyped.md)

### `` >Sorbet: Toggle the autocomplete nudge in `typed: false` files ``

[``>Sorbet: Toggle the autocomplete nudge in `typed: false` files``]: #-sorbet-toggle-the-autocomplete-nudge-in-typed-false-files-

By default, Sorbet shows a completion item in `# typed: false` files when there are no completion results due to the file being `# typed: false`, like this:

<img src="/img/typed-false-nudge.png" style="max-width: 463px; margin-left: 0;" />

The goal of the nudge is to encourage upgrading the current file [from `# typed: false` to `# typed: true`](static.md). Use this command to disable the nudge.

As a reminder, most features, including autocompletion, [have degraded support](lsp-typed-level.md) in `# typed: false` files.

## Configuration Reference

This section documents configuration options which live in the [User or Workspace Settings](https://code.visualstudio.com/docs/getstarted/settings) of VS Code.

For a complete reference of the Sorbet extension's configuration options, their types, and their defaults, see the `"configuration"` section of the [package.json](https://github.com/sorbet/sorbet/blob/master/vscode_extension/package.json) file.

### Sticky configuration

Sorbet extension settings are _sticky_: even if the `settings.json` file for the workspace or the user has a given setting, Sorbet will remember when the user previously changed that the setting from the UI. These choices persist across reboots of VS Code.

For example, consider the [`sorbet.enabled`](#sorbet-enabled) setting and the [`>Sorbet: Enable`] command in this scenario:

1.  Initially, the `.vscode/settings.json` file for a given workspace has

    ```json
    {
      "sorbet.enabled": false
    }
    ```

    When VS Code starts up, it sees that Sorbet is disabled and does not launch Sorbet.

1.  The user then runs `>Sorbet: Enable` to start Sorbet, does some work, and quits VS Code.

1.  The next time VS Code is opened in this workspace, Sorbet will still be enabled and start automatically.

It's not that the `>Sorbet: Enable` command edited the contents of the `.vscode/settings.json` file to flip it to `true`, but rather that the Sorbet extension remembers whether a setting has been changed via a command after starting up.

The only settings that are not sticky like this are the settings which do not have a [command](#command-reference) to change the corresponding [configuration setting](#configuration-reference).

### `sorbet.enabled`

Whether Sorbet should be enabled or disabled in this workspace.

### `sorbet.lspConfigs`

This setting controls the command used to launch the Sorbet server process.

The Sorbet extension supports switching between multiple configurations to make it easy to try out experimental features or register custom command line options. By default, it ships with three configurations: `Sorbet`, `Sorbet (Beta)`, and `Sorbet (Experimental)`.

Users can switch between these configurations using the [`>Sorbet: Configure`] command, or by clicking on "Sorbet: ..." in the status bar and selecting "Configure Sorbet".

As mentioned elsewhere:

- Changing the config will cause Sorbet to restart in the chosen configuration.
- Using the UI to change this setting is sticky: Sorbet will use the most recently chosen configuration when next launched.

An example configuration would look like this. Consisder a project that uses Sorbet but that does not use Bundler:

```json
{
  "sorbet.lspConfigs": [
    {
      "id": "without-bundler",
      "name": "srb",
      "description": "Launch Sorbet using `srb` from the PATH",
      "cwd": "${workspaceFolder}",
      "command": ["srb", "tc", "--lsp"]
    }
  ]
}
```

This project would not use `bundle exec [...]` to launch the Sorbet server process.

Some other ideas for how to use this configuration feature:

- Pass custom, [LSP-specific options](cli-ref.md#lsp-options) that don't belong in the [`sorbet/config`](cli.md#config-file).

- Wrap the call to `srb tc` in a custom script for the current project, which might do things like make sure the environment is set up correctly before starting Sorbet.

- Start the server process using a custom, local build of the Sorbet binary.

When setting this setting, it's recommended to refer to the more complete documentation in the Sorbet extension's [package.json](https://github.com/sorbet/sorbet/blob/master/vscode_extension/package.json) file.

### `sorbet.userLspConfigs`

This setting is largely the same as `sorbet.lspConfigs`, but has higher precedence. It's designed to allow users to keep their own Sorbet configurations in VS Code's User settings.

Normally, Workspace Settings [take precedence over](https://code.visualstudio.com/docs/getstarted/settings#_settings-precedence) User Settings. Having this option gives users the chance to invert that.

Because of this, it's usually not recommended to commit this setting to `.vscode/settings.json` in a workspace, because it prevents users from launching Sorbet according to their own preferences.

### `sorbet.selectedLspConfigId`

Which LSP configuration to pick first (see [`sorbet.lspConfigs`](#sorbetlspconfigs)).

If unset, picks the first one, giving precedence to `sorbet.userLspConfigs`.

Remember that [settings are sticky](#sticky-configuration), so if the user has used [`>Sorbet: Configure`] to change the configuration, that most recently selected configuration will take precedence over this setting.

### `sorbet.highlightUntyped`

Whether to highlight untyped, and where. Defaults to `"nowhere"`.

See [Highlighting untyped code](highlight-untyped.md)

### `sorbet.highlightUntypedDiagnosticSeverity`

What diagnostic severity to use when reporting usages of untyped.

See [Highlighting untyped code](highlight-untyped.md)

### `sorbet.typedFalseCompletionNudges`

Whether to enable `# typed: false` completion nudges. Defaults to `true`.

See [``>Sorbet: Toggle the autocomplete nudge in `typed: false` files``].

### `sorbet.configFilePatterns`

A list of workspace file patterns that contribute to Sorbet's configuration. Changes to any of those files should trigger a restart of any actively running Sorbet language server.

For example, changes to the `sorbet/config` file or the `Gemfile.lock` could mean that the Sorbet process needs to restart (to pick up new options, or a new version, respectively). The default `sorbet.configFilePattern` tracks these two files are tracked

Projects that have custom scripts that launch Sorbet likely want to set this setting.

Careful: setting this setting completely overwrites the default.

### `sorbet.revealOutputOnError`

Shows the extension output window on errors. Defaults to `false`.

---

## Extension API

### Reporting metrics

> Sorbet does not require metrics gathering for full functionality. If you are seeing a log line like "Sorbet metrics gathering disabled," Sorbet is working as intended.

It is possible to ask the Sorbet VS Code extension to collect and report usage metrics. This is predominantly useful if you maintain a large Ruby codebase that uses Sorbet, and want to gather metrics on things like daily users and editor responsiveness.

To start gathering metrics, implement a [custom VS Code command](https://code.visualstudio.com/api/extension-guides/command#creating-new-commands) using the name `sorbet.metrics.getExportedApi`.

The implementation of this command should simply return an object like this:

```js
{
  metricsEmitter: ...
}
```

The `metricsEmitter` value should conform to the [`MetricsEmitter` interface] declared in the Sorbet VS Code extension source code. Most likely, you will want to implement this interface by importing a StatsD client, connecting to an internal metrics reporting host, and forwarding requests from the `MetricEmitter` interface function calls to the StatsD client of your choice.

[`metricsemitter` interface]: https://github.com/sorbet/sorbet/blob/2b850340e9bccd689d6a976cddbbfecf533933ae/vscode_extension/src/veneur.ts#L11

### Tracking server status changes

Starting from version 0.3.41, the Sorbet extension exports an API to track server status changes. To ensure backward and forward compatibility, all properties are nullable.

- `status`: Represents Sorbet status, or `undefined` if the state is unknown.
- `onStatusChanged`: An event triggered whenever the status changes.

You can access this API using VS Code's `getExtension` API, e.g.

```javascript
const ext = vscode.extensions.getExtension('sorbet.sorbet-vscode-extension');

// Example: get current status
ext?.exports?.status;
```

The `status` field may take on the following string values:

- `"disabled"`: Indicates that the Sorbet Language Server has been disabled.
- `"error"`: Indicates that the Sorbet Language Server encountered an error. This status does **not** mean that there are type checking errors: it means there was an error with the language client itself.
- `"running"`: Indicates that the Sorbet Language Server is running.
- `"start"`: Indicates that the Sorbet Language Server is starting. This status may repeat in case of an error.

## Troubleshooting and FAQ

<!-- TODO(jez) Make separate pages for these features, and document them there.
Most of these are not VS Code-specific. -->

### Diagnostics / error squiggles

#### Sorbet is reporting diagnostics (error squiggles) that don't make sense.

If the errors are persistent, and you can reproduce your problem in the sandbox at https://sorbet.run/, then you've found an issue with Sorbet in general, not necessarily the VS Code Sorbet extension. Please file a bug tagged with "IDE" on the [issue tracker](https://github.com/sorbet/sorbet/issues).

If the errors are not persistent:

- First, is Sorbet still typechecking? If it is, wait for the status bar to display "Sorbet: Idle." before judging the diagnostics.
- If the diagnostics remain nonsensical when Sorbet is Idle, try restarting Sorbet (click on Sorbet in the status bar, and click on Restart Sorbet).
- If the diagnostics become fixed, then you might have discovered a bug in Sorbet.
- Another thing to check is whether the Problems tab in VS Code has the same errors as what Sorbet would report when run from the command line (assuming all files have been saved). If the list of errors is different, this usually represents a bug in Sorbet (so long as Sorbet is "Idle" in the IDE).

If you arrive at a set of edits that mess up the diagnostics, please file a bug on the [issue tracker](https://github.com/sorbet/sorbet/issues).
