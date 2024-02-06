# Debugging Crashes in Sorbet

From time to time, Sorbet crashes. Every crash in Sorbet is a bug, no matter
what. We'd be more than happy to fix the cause of the crash. Most of the time,
fixing a crash in Sorbet is quite easy, while the hard part is diagnosing the
cause of the crash itself.

This doc contains some tips for debugging Sorbet crashes.

It's going to be specific to the Sorbet type checker (not sorbet-runtime).

## Find a reproducer

This doc is not going to cover tips for how to reproduce the crash. Hopefully,
most crashes will be fairly straightforward to reproduce. If you can reproduce
the crash in a codebase that you're comfortable sharing with the Sorbet team,
feel free to share that reproducer directly with the team, either in a [new
issue], or if absolutely necessary, in a DM with a member of the Sorbet team.

[new issue]: https://github.com/sorbet/sorbet/issues/new/choose

Once we have a reproducer for a crash, we are happy to take over from there.

## Get it running in a C++ debugger

If for whatever reason, you can't share a reproducer, the other option is to
share more information about the crash itself, so that a member of the Sorbet
team can try to use a combination of looking at the code and trial-and-error to
make a suitable reproducer.

A backtrace is the most useful piece of information to share, and it's easiest
to share this by running Sorbet under a C/C++ debugger like `gdb` or `lldb`.

To do this:

1.  Find the path to the Sorbet binary that you're running. If you're using
    Bundler, you can print it by running this inside your project.

    ```bash
    echo $(bundle info sorbet-static --path)/libexec/sorbet
    ```

    This will print the full path to the `sorbet` binary. Note that the `srb`
    executable is a shell wrapper script around the `sorbet` binary executable.
    Attempting to debug the `srb` executable will not work.

    If you are using the `SRB_SORBET_EXE` environment variable, that will
    already be the path to the `sorbet` binary.

1.  Run Sorbet on your project, under GDB or LLDB.

    ```bash
    # gdb:
    gdb --args $(bundle info sorbet-static --path)/libexec/sorbet

    # lldb:
    lldb -- $(bundle info sorbet-static --path)/libexec/sorbet
    ```

    Then press `r` ENTER to launch the process.

    > Note:
    > that when invoking Sorbet via the `srb tc` command, Sorbet will also
    > add an extra argument like `@/path/to/.cache/sorbet/...` which Sorbet uses
    > to discover RBI files provided via gems' `rbi/` folders. You may or may
    > not need to list this extra `@...` argument when running the debugger.
    >
    > An easy way to list these extra arguments is to run:
    >
    > ```bash
    > SRB_SORBET_EXE=echo srb tc
    > ```
    >
    > (This works by asking `srb` to pretend that the `sorbet` binary is
    > actually the `echo` command, which has the effect of simply printing the
    > arguments then exiting without type checking.)

1.  When the crash happens, the debugger will pause execution. You can type

    ```
    bt
    ```

    Ask it to print a pretty backtrace. Please include this information in your
    [new issue] or share it with a member of the Sorbet team.

Depending on the contents of the backtrace, a member of the Sorbet team may ask
you to try running other commands in the debugger to attempt to diagnose the
problem.

If you want to try to debug it on your own but are unfamiliar with GDB/LLDB, we
recommend this quick reference:

→ <https://lldb.llvm.org/use/map.html>

It shows a number of commonly-used debugger commands in both GDB and LLDB
command syntax.
