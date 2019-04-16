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

There are four configuration hooks that can be set in the runtime.

> **Note**: These docs are incomplete. For complete usage information, refer to
> the YARD documentation for the `sorbet-runtime` gem.

## Errors from inline type assertions

There are four kinds of [inline type assertions](type-assertions.md):

- `T.let(expr, Type)`
- `T.cast(expr, Type)`
- `T.must(expr)`
- `T.assert_type!(expr, Type)`

To customize the behavior when one of these assertions fails:

```ruby
T::Configuration.inline_type_error_handler = lambda do |error|
  puts error.message
end
```

The default error handler is to raise (a `TypeError`).

Note that setting this callback will **not** handle type errors raised when a
method with a signature is called incorrectly. For those, see the next section.


## Errors from invalid method calls

To customize the behvaior when a method with a sig is called and the argument
types or return types don't match the actual value present at runtime, use this:

```ruby
T::Configuration.call_validation_error_handler = lambda do |signature, opts|
  puts opts[:message]
end
```

The default error handler is to raise an error, but it respects the `.soft` and
`.checked` builder methods, discussed in [Escape
Hatches](troubleshooting.md#escape-hatches).

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

Method signatures that build correctly can still be invalid. For exmaple, a sig
marked `override` must actually override a method. Same for `abstract` methods.
When overriding a parent sig, the variance must match on the input and output
types. If a sig that built correctly is invalid in anyway, this error handler
will be called:

```ruby
T::Configuration.sig_validation_error_handler = lambda do |error, opts|
  puts error.message
end
```

