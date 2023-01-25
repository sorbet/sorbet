---
id: tconfiguration
title: Runtime Configuration
---

Recall from [Enabling runtime checks](runtime.md) that `sorbet-runtime` raises
exceptions for type errors that happen at runtime. This is to ensure that even
code that is not covered by types statically can still benefit from type
information, and that authors don't have to defend against their typed methods
being called incorrectly.

The behavior of the `sorbet-runtime` package can be configured by registering
callbacks before a program is run. While it's possible to use these callbacks to
silence all runtime exceptions raised by `sorbet-runtime`, we **strongly**
recommend not doing so. Runtime exceptions have proven to be invaluable in
gaining confidence in the correctness of type signatures as we rolled out types
to Stripe's Ruby codebase.

> **Note**: Think carefully before disabling runtime checks!

These docs are somewhat low-level. For a higher-level description of how to
change Sorbet's runtime, see [Enabling Runtime Checks](runtime.md).

## Errors from inline type assertions

There are four kinds of [inline type assertions](type-assertions.md):

- `T.let(expr, Type)`
- `T.cast(expr, Type)`
- `T.must(expr)`
- `T.assert_type!(expr, Type)`

To customize the behavior when one of these assertions fails:

```ruby
T::Configuration.inline_type_error_handler = lambda do |error, opts|
  puts error.message
end
```

The default error handler is to raise (a `TypeError`).

Note that setting this callback will **not** handle type errors raised when a
method with a signature is called incorrectly. For those, see the next section.

## Errors from invalid method calls

To customize the behavior when a method with a sig is called and the argument
types or return types don't match the actual value present at runtime, use this:

```ruby
T::Configuration.call_validation_error_handler = lambda do |signature, opts|
  puts opts[:message]
end
```

The default error handler is to raise an error.

The example handler above does the same thing for every method. One thing which
can be useful is to pass custom metadata to the `call_validation` handler, which
can be done with `.on_failure`:

```ruby
T::Configuration.call_validation_error_handler = lambda do |signature, opts|
  if signature.on_failure
    puts "Metadata: #{signature.on_failure}"
  end
  raise TypeError.new(opts[:pretty_message])
end

sig {params(x: Integer).void.on_failure(:hello, :world)}
def foo(x); end
```

When the `call_validation_error_handler` is called this time, it will be passed
all the args that were given to `on_failure` for the specific sig that failed.

## Errors from invalid sig procs

We [write sigs](sigs.md) using valid Ruby syntax. The body of the proc passed to
a sig is executed (lazily, on first method call) to compute an in-memory data
structure representing that sig's types. The execution of this proc can be
invalid (for example, if `returns` or `void` is never called).

The default behavior when building a sig is invalid is to raise an
`ArgumentError`. To customize this behavior, use this:

```ruby
T::Configuration.sig_builder_error_handler = lambda do |error, location|
  puts error.message
end
```

## Errors from invalid sigs

Method signatures that build correctly can still be invalid. For example, a sig
marked `override` must actually override a method. Same for `abstract` methods.
When overriding a parent sig, the variance must match on the input and output
types. If a sig that built correctly is invalid in anyway, this error handler
will be called:

```ruby
T::Configuration.sig_validation_error_handler = lambda do |error, opts|
  puts error.message
end
```

## Environment variables

There are a number of environment variables that `sorbet-runtime` reads from to
change its behavior:

### `SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS`

Announces to Sorbet that we are currently in a test environment, so it should
treat any sigs which are marked `.checked(:tests)` as if they were just a normal
sig. This can be set to any truthy value to take effect.

This can also be done by calling
`T::Configuration.enable_checking_for_sigs_marked_checked_tests` but the
environment variable ensures this value gets set before any sigs are evaluated.

### `SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL`

Configure the default checked level for a sig with no explicit `.checked`
builder. When unset, the default checked level is `:always`. This must be set to
a valid checked level (e.g. `always` or `tests`).

This can also be done by calling `T::Configuration.default_checked_level = ...`
but the environment variable ensures this value gets set before any sigs are
evaluated.
