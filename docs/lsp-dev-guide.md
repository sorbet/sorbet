# Developing Sorbet's Language Server

Developing on Sorbet's language server is slightly more involved (as compared
with working on the guts of Sorbet's type checker, or the sorbet-runtime gem).

There are two main things you might want to do:

1.  Work on the server component.

    The best way to develop on the server is to write a small test and use
    our test suite. But _is_ possible to test this by driving the editor.

2.  Work on the client component.

    While LSP is technically client agnostic, in practice, VS Code needs a lot
    of boilerplate to be able to support most LSP features. As a result, we
    maintain first-party support for a VS Code extension.

## Working on the server

### By way of tests

There are two main kinds of LSP tests you're likely to interact with:

- The LSP tests in `test/testdata/lsp/`.

  Each file in `test/testdata/lsp/` is both a test of Sorbet via LSP and
  Sorbet's standalone pipeline.

  These tests are documented in [Sorbet's README]. In particular, when running
  as LSP tests, there are a bunch more annotations you can use to test LSP
  features. For example `# ^ hover: ...` is an annotation that will send a hover
  request at that location.

  **Tips:**

  - [Configure fzf to drive Bazel](https://blog.jez.io/fzf-bazel/). This is the
    easiest way to go from "name of file on disk" to "Bazel test target I can
    run"

  - Run the test with the `--test_output=errors` flag or `--test_output=all`
    flag. If the test fails, you'll be see a line that looks like

    ```
    + exec test/lsp_test_runner --single_test=test/testdata/lsp/completion/alias_method.rb
    ```

    somewhere in the output. This is the actual binary that Bazel built and ran
    to run the test, which you can then debug. To debug this test, delete the `+
    exec` part and add `lldb -- bazel-bin/` in front:

    ```
    lldb -- bazel-bin/test/lsp_test_runner --single_test=test/testdata/lsp/completion/alias_method.rb
    ```

    This is the easiest way to debug LSP things, because you can set breakpoints
    inside the code that implements various LSP methods without having to click
    in the UI to initiate those requests.

- The protocol tests.

  These files live in `test/lsp/*protocol_test_corpus.cc` (there are only a
  handful of them).

  These tests are written in C++ and test low-level protocol that can't be
  tested with plain Ruby files. That means things like "what if you get two
  `initialize` requests" or "what if you get a very oddly specific sequence of
  fast and slow path edits."

  These test are more work to write but sometimes your only option.

  **Tips:**

  - These tests are slow (some of them _really_ slow). You're almost never going
    to want to run all of them. Run just the one you want with `--test_arg`:

    ```
    bazel test --config=dbg --test_output=all \
      //test/lsp:multithreaded_protocol_test_corpus \
      --test_arg=--test-case=CanPreemptSlowPathWithCodeAction
    ```

  - Only edit multithreaded_protocol_test_corpus in particular as a last resort.
    This is one of the only places in Sorbet's test suite that uses threads, and
    that means that unless you know what you're doing, it can be very flaky.
    (An example of when to use this: when it's specifically the concurrency /
    sequencing that's going wrong in LSP.)

### By trying it in a client

There are two quick-and-dirty sandbox configurations that you can use for
testing a custom build of the language server in a real client:

- `test/sandbox/vscode/`
- `test/sandbox/nvim/`

Go into either of those directories, read the respective `README.md` in the
folder, and it will tell you how to launch the editor. All the configurations
will use `../../../bazel-bin/main/sorbet` to launch the language server, so
whatever Sorbet you most recently built will be used.

**Tips**:

- By default, both of those are configured to drop a Sorbet debug log to the
  current folder. In the log you will find the complete trace of the JSON RPC
  methods sent between the client and server, as well as other debug logs.

- You can edit the language server launch settings to include `-vvv`, which will
  raise the log level to trace, which will show you literally every log (can be
  noisy, sometimes useful).

- You can attach a debugger to a running LSP server launched this way. Find its
  PID and then use `lldb -p <PID>`.

  I use [this zsh config][fzf-lldb] so that I can simply do `lldb -p **<TAB>`
  and then use fzf to find the relevant process ID, rather than having to find
  it in `ps` or `htop` and copy it over.

- If the thing you're trying to test happens at startup, you can pass the
  `--wait-for-dbg` flag to the LSP command line args. Sorbet will hang
  indefinitely until it detects that a debugger has attached (like with `lldb
  -p`). It will then break, where you can set breakpoints and then `continue` to
  resume normal execution.

[fzf-lldb]: https://github.com/jez/dotfiles/blob/master/util/lldb.zsh

## Working on the VS Code client

**Tips:**

- Both mechanisms below set the log level to trace, so you will be able to see
  all logs.

- You can work on both the VS Code extension and the Sorbet language server at
  the same time: simply close whatever workspace might be open in the VS Code
  window and then "Open Folder" to the `test/sandbox/vscode/` folder discussed
  above. (Be sure that the "Sorbet (path)" configuration is selected.)


### If you want to use VS Code to edit the extension

```
cd vscode_extension
code --new-window .
```

This will open a new workspace for the VS Code extension sources.

Edit the code as you see fit, then to test your changes:

- Find the "Run and Debug" window
- Ensure that "Launch Extension" is selected in the dropdown
- Click the green triangle, which will launch a new VS Code instance with the
  local copy of the extension loaded
- Navigate to the project you want to test Sorbet in

Or in one command: ⇧⌘P > `Debug: Select and Start Debugging` > `Launch Extension`

This will launch a separate VS Code window, with the newly-compiled extension
sources. The nice thing is that you can use the first VS Code window to set
breakpoints, and they will get hit when you interact with the second VS Code
window.

If you want to use print debugging, the log level will be set to `trace` so all
logs should show up.

### If you want to use another editor

The "Launch Extension" workflow described in the previous section can be
achieved with the command line as well:

```bash
cd vscode_extension

# Start watching TypeScript files for changes, recompiling as necessary.
yarn watch

# ... alternatively, to run once without watching:
yarn compile

# Launch a new VS Code window with the newly-compiled extension sources
yarn launch ../test/sandbox/vscode
```

