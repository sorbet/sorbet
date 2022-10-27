# Suggesting Sigs

It's possible to build Sorbet in a way that it can suggest the "most impactful"
methods to add a signature to, something like this:

```
❯ sorbet --suggest-sig foo.rb
No errors! Great job.
Typing `::A#bar` would impact 100.0% callsites(2 out of 2).
```

There are some fairly involved setup steps to get this working.

Something less involved is asking Sorbet to directly edit a project's source
code with sigs that might work. This involves less custom setup.

> Most impactful has a narrow definition: the most impactful method to add a sig
> to is the method which would increase the [call-site-level typedness][metrics]
> the most. Read [this doc][metrics] to learn what this means, and why the
> Sorbet team recommends tracking it. It's important to know that this
> definition of "most impactful" doesn't always match up with your team's or
> organization's definition of highest impact at any given time.
>
> [metrics]: https://sorbet.org/docs/metrics#which-metrics-to-track

## Suggesting the most impactful methods to sig

This approach involves some special setup.

### Building a debug build of Sorbet

The Sorbet team does not publish versions of Sorbet that can emit a list of most
impactful methods to sig out of the box. The logic which collects it to output
this information slows down every type check run, not just the runs which output
this information.

To gain access to this feature, you'll need to build Sorbet locally in **debug
mode**. You can check whether your version of Sorbet is in debug mode using the
`--version` flag:

```
❯ srb tc --version
Sorbet typechecker ... debug_mode=true
```

If you see `debug_mode=true`, then Sorbet was built in debug mode.

To build Sorbet in debug mode, follow the setup instructions in the [README] and
then run:

```
./bazel build //main:sorbet --config=release-debug-linux
```

(There are many other ways to build debug mode versions of Sorbet; see the
[README] if you're curious.)

[README]: https://github.com/sorbet/sorbet#readme

### Type checking a project

After having built Sorbet from source, there will be a file
`bazel-bin/main/sorbet` in the Sorbet project folder. To run this version of
Sorbet on a Ruby project:

1.  Change to the project's directory.

2.  Run `./path/to/bazel-bin/main/sorbet` with whatever arguments
    you usually give to `srb tc`, but add `--suggest-sig`:

    ```bash
    ./path/to/bazel-bin/main/sorbet --suggest-sig <...>
    ```

    If your project doesn't run `srb tc` directly but wraps it with a custom
    script, you can set `SRB_SORBET_EXE` before running your custom script, like

    ```bash
    export SRB_SORBET_EXE=./path/to/bazel-bin/main/sorbet
    my-custom-script.sh <...>
    ```

After Sorbet finishes checking the project, it will output a bunch of lines that
look like:

```
Typing `::A#bar` would impact 100.0% callsites(2 out of 2).
```

### Interpreting the output

The units of what's being counted here is kind of abstract (fully understanding
it involves fully understanding the intermediate representation that Sorbet uses
when typechecking a method), but at a high level it's counting the
number of sub-expressions in that method whose types reference anything untyped.
It tracks back the source of the untypedness to the definition that introduced
the `T.untyped`.

Feel free to try to read the source code if that's confusing. The point I'm
trying to make is that this metric is **not 1-to-1 with the call-site-level
typedness metric** (i.e., the percentage is not a measure of how much the
call-site-level typedness will improve by adding a sig to that thing).

Rather, treat it as an ordered list; the ones higher on the list will have more
of an impact on the metric than the ones lower.

## Directly adding sigs to a project

Sorbet can sometimes make an educated guess about what sig to put on a method,
and directly edit the project to include this sig. This is useful as a starting
point for a more manual effort.

Note that while much of this is automated, it is not a silver bullet. Sometimes
the sigs Sorbet guesses are bad or have lots of `T.untyped` in them. Sometimes
the sigs that Sorbet inserts reveal type errors in other parts of a codebase. In
these cases. Play around with this to see how well it works for you.

### Revealing sig autocorrects

In `typed: strict` files, it's an error for a method to not have a sig. When
reporting these errors, Sorbet attempts to guess a signature that might work for
that method, and then adds it to the error message.

```
editor.rb:5: The method `bar` does not have a sig https://srb.help/7017
     5 |  def bar; end
          ^^^^^^^
  Autocorrect: Use `-a` to autocorrect
    editor.rb:5: Insert sig {returns(NilClass)}

     5 |  def bar; end
          ^
Errors: 1
```

These errors always have the error code `7017`, are only reported if the file is
`typed: strict` or higher. They can be applied with the `--autocorrect` option.

To opt all files into this strictness level and insert as many sigs as Sorbet
can guess, run this:

```
srb tc --typed=strict --isolate-error-code=7017 --autocorrect
```

This

- opts all files into `typed: strict`, regardless of what level they were
  previously at
- silences reporting errors for any error other than `7017` (so that the only
  autocorrects will be for inserting sigs, not fixing any other type errors)
- edits the files in place whenever Sorbet could suggest a sig.

We used these options heavily when rolling out signatures in Stripe's codebase.
Some tips:

- Run this command until Sorbet stops editing files. Frequently adding a sig in
  one place will give Sorbet enough information to add a sig in another place.

- If a particular sig looks like it introduces a lot of type errors, remove it.
  You can fix it manually later, rather than trying to get Sorbet to suggest a
  different or better sig.

- Run your test suite. This is one area where having `sig`s checked at runtime
  helps: if Sorbet's guess was wrong, it's likely that the tests will catch it.

