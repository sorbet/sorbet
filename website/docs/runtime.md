---
id: runtime
title: Enabling Runtime Checks
---

As we've mentioned before, Sorbet is a [gradual](gradual.md) system: it can be
turned on and off at will. This means the predictions `srb` makes statically can
be wrong.

That's why Sorbet also uses **runtime checks**: even if a static prediction was
wrong, it will get checked during runtime, making things fail loudly and
immediately, rather than silently and after the fact.

In this doc we'll answer:

- What's the runtime effect of adding a `sig` to a method?
- Why do we want to have a runtime effect?
- What are our options if we don't want `sig`s to affect the runtime?

## Runtime-checked `sig`s

Adding a method signature opts that method into runtime typechecks (in addition
to opting it into [more static checks](static.md)). In this sense,
`sorbet-runtime` is similar to libraries for adding runtime contracts.

Concretely, adding a `sig` wraps the method defined beneath it in a new method
that:

- validates the types of arguments passed in against the types in the `sig`
- calls the original method
- validates the return type of the original method against what was declared
- returns what the original method returned[^void]

<!-- prettier-ignore-start -->

[^void]: The case for `.void` in a `sig` is slightly different. See [the docs
for void](sigs.md#returns-void-annotating-return-types).

<!-- prettier-ignore-end -->

For example:

```ruby
require 'sorbet-runtime'

class Example
  extend T::Sig

  sig {params(x: Integer).returns(String)}
  def self.main(x)
    "Passed: #{x.to_s}"
  end
end

Example.main([]) # passing an Array!
```

```shell
❯ ruby example.rb
...
Parameter 'x': Expected type Integer, got type Array with unprintable value (TypeError)
Caller: example.rb:11
Definition: example.rb:6
...
```

In this small example, we have a `main` method defined to take an Integer, but
we're passing an Array at the call site. When we run `ruby` on our example file,
sorbet-runtime raises an exception because the signature was violated.

## Why have runtime checks?

Runtime checks have been invaluable when developing Sorbet and rolling it out in
large Ruby codebases like Stripe's. Type annotations in a codebase are near
useless if developers don't trust them (consider how often YARD annotations fall
out of sync with the code... 😰).

Adding a `sig` to a method is only as good as the predictions it lets `srb` make
about a codebase. Wrong sigs are actively harmful. Specifically, when `sig`s in
our codebase are wrong:

- we can't use them to find code to refactor. Sorbet will think some code paths
  can never be reached when they actually can.
- they're effectively as good as out-of-date documentation, with little added
  benefit over just comments.
- we could never use them to make Ruby code run faster. In the future, we hope
  to use Sorbet types to make Ruby faster, but `sig`s that lie will actually
  make code _slower_ than no types at all.

By leveraging runtime checks, we can gain lots of confidence and trust in our
type annotations:

- Automated test runs become tests of our type annotations!
- Our production observability and monitoring catch bad sigs **early**, before
  they propagate false assumptions throughout a codebase.

To drive these points home, let's look at a concrete example:

```ruby
# typed: true
require 'sorbet-runtime'

class Example
  extend T::Sig

  def self.some_untyped_method
    nil
  end

  sig {params(x: Integer).returns(Integer)}
  def self.add_one(x)
    x + 1
  end
end

Example.add_one(Example.some_untyped_method)
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0Arequire%20'sorbet-runtime'%0A%0Aclass%20Example%0A%20%20extend%20T%3A%3ASig%0A%0A%20%20def%20self.some_untyped_method%0A%20%20%20%20nil%0A%20%20end%0A%0A%20%20sig%20%7Bparams(x%3A%20Integer).returns(Integer)%7D%0A%20%20def%20self.add_one(x)%0A%20%20%20%20x%20%2B%201%0A%20%20end%0Aend%0A%0AExample.add_one(Example.some_untyped_method)">→
View on sorbet.run</a>

In this example, `srb tc` reports that there were **no errors statically**. But
if we were to run this code with `ruby`, `some_untyped_method` would return
`nil`, we'd try to add `1` to it, and Ruby would raise a NoMethodError for `+`.
By adding a `sig` to `add_one`, the Sorbet runtime will raise an exception
before even starting to execute the method. This makes typing errors from
untyped code manifest early and loudly and right at the source, rather than
silently, long after a sig was added, and far removed from this line of code.

Most people are _either_ familiar with a completely typed language (Java, Go,
etc.) or a completely untyped language like Ruby; a
[gradual type system](gradual.md) can be very foreign at first. Including these
runtime checks by default protects typed code from untyped code, making it
easier to drive adoption of types in the long run.

## Changing the runtime behavior

While having runtime checks is the default, it's possible to change the behavior
of the runtime system via configuration settings. These configuration settings
live in under the `T::Configuration` module within the `sorbet-runtime` gem.

There are two main ways to change Sorbet's runtime:

- **When** the runtime checks fail, what to do in response.

  To change this, we use `.on_failure` in a method signature.

- **Whether** the runtime checks run in the first place.

  To change this, we use `.checked` in a method signature.

In the next sections, we'll give some examples of how to use both.

## `.on_failure`: Changing what happens on runtime errors

By adding `.on_failure(...)` to a sig and registering a `T::Configuration`
callback, we can change what happens when a sig check fails. For example:

```ruby
require 'sorbet-runtime'

# (1) Register call_validation_error_handler callback.
# This runs every time a method with a sig fails to type check at runtime.
T::Configuration.call_validation_error_handler = lambda do |signature, opts|
  #                                                     ┌──┐
  if signature.on_failure && signature.on_failure[0] == :log
    puts opts[:pretty_message]
  else
    raise TypeError.new(opts[:pretty_message])
  end
end

class Main
  extend T::Sig

  # (2) Use .on_failure in the sig for a method
  #                                       ┌───────────────┐
  sig {params(argv: T::Array[String]).void.on_failure(:log)}
  def self.main(argv)
    puts argv
  end
end

# (3) When we call main incorrectly, it will print
# with puts instead of raise an exception.
Main.main(42)

# Output:
# ❯ ruby example.rb
# Parameter 'argv': Expected type T::Array[String], got type Integer with value 42
# Caller: example.rb:25
# Definition: example.rb:18
# 42
```

We defined our own meaning for `.on_failure` with `T::Configuration`. Without
doing this, `.on_failure` has no effect. Because of this, `.on_failure` can be
completely customized within any codebase to change what it means to fail. For
example at Stripe, we use `.on_failure` to attach team ownership information to
a failure.

The `T::Configuration` handler we wrote branches on whether `.on_failure` was
provided, which means the logging behavior is opt in. This is nice because it
means the default behavior is still to make problems with types fail loudly and
early, rather than silently as a log. If we wanted, we could have inverted this:

```ruby
T::Configuration.call_validation_error_handler = lambda do |signature, opts|
  #                                                     ┌────┐
  if signature.on_failure && signature.on_failure[0] == :raise
    raise TypeError.new(opts[:pretty_message])
  else
    puts opts[:pretty_message]
  end
end

# ...

#                                       ┌─────────────────┐
sig {params(argv: T::Array[String]).void.on_failure(:raise)}
```

With this `T::Configuration` handler, the default is to log, and we can use
`.on_failure` to opt specific sigs into raising on failure.

We haven't depicted it here, but the `T::Configuration` handler will get an
array of every argument that was provided to `.on_failure` for this
sig—specifically there's no restriction that `.on_failure` must be given only
one arg nor that the arg is a Symbol. For more on the various `T::Configuration`
handlers, see [Runtime Configuration](tconfiguration.md).

## `.checked`: Whether to check in the first place

> **Careful!** Opting out of runtime checks can significantly degrade the
> trustworthiness of type signatures. Only disable the runtime after
> understanding the tradeoffs. See [Gradual Type Checking](gradual.md) to learn
> more.

In our examples above with `.on_failure`, every method call had the runtime type
checks run to determine whether the `T::Configuration` handler should be called
in the first place. This comes with an overhead, and we carefully audit and
monitor the performance of `sorbet-runtime` as a result. The overhead of these
checks are usually very small.

But in some cases, especially when calling certain methods in tight loops or
other latency-sensitive paths, the overhead of even doing the checks (regardless
of what happens on failure) is prohibitively expensive. To handle these cases,
Sorbet offers `.checked(...)` which declares in what environments a sig should
be checked:

```ruby
# (1) Runtime checks always run.
sig {params(xs: T::Array[String]).void.checked(:always)}

# (2) Runtime checks only run in "tests" (see below).
# In non-tests, this sig specifically has no runtime overhead.
sig {params(xs: T::Array[String]).void.checked(:tests)}

# (3) Never runs the runtime checks. Careful!
sig {params(xs: T::Array[String]).void.checked(:never)}
```

If `.checked(...)` is omitted on a sig, the default is `.checked(:always)`. The
default checked level can also be configured. For example:

```ruby
T::Configuration.default_checked_level = :tests
```

This can also be set via the `SORBET_RUNTIME_DEFAULT_CHECKED_LEVEL` environment
variable, see [Environment Variables](tconfiguration.md#environment-variables)
for more.

Writing this will make it so that any sig which does not have a `.checked(...)`
call in it will behave as if the user had written `.checked(:tests)`. To prevent
accidental misuse, `sorbet-runtime` will require that this setting is changed
before any `sig` is evaluated at runtime.

**Note**: For `.checked(:tests)` to work correctly, checking in tests must be
enabled in every entry point into the tests. To declare that a certain entry
point is a test, run:

```ruby
T::Configuration.enable_checking_for_sigs_marked_checked_tests
```

This can also be set via the `SORBET_RUNTIME_ENABLE_CHECKING_IN_TESTS`
environment variable, see
[Environment Variables](tconfiguration.md#environment-variables) for more.

For example, this should probably be placed as the first line of any `rake test`
target, as well as any other entry point to a project's tests. If this line is
absent, `.checked(:tests)` sigs behave as if they had been `.checked(:never)`.

## `T::Sig::WithoutRuntime.sig`

Even with `.checked(:never)`, there is some slight runtime overhead. The block
for a `sig {...}` above a method is not evaluated until the first time that
method is called. But Sorbet can only know whether a sig is `.checked(:never)`
or not until the block is evaluated. So even if a sig is marked
`.checked(:never)`, Sorbet will still wrap the method. The first time the method
is called, Sorbet will discover the `.checked(:never)` and put back the original
method.

Sometimes even this tiny amount of runtime metaprogramming is unacceptable at
runtime. To completely eliminate all runtime side effects when defining a
signature, replace `sig` with `T::Sig::WithoutRuntime.sig` when annotating
methods:

```ruby
# typed: true
require "sorbet-runtime"

class Foo
  extend T::Sig
  # This signature will raise both statically and at runtime
  # (because `NotFound` doesn't exist, and causes a NameError)
  sig { params(x: NotFound).void.checked(:never) }
  def foo(x); end

  # This signature will only raise statically
  # (the sig block is ignored completely at runtime)
  T::Sig::WithoutRuntime.sig { params(x: NotFound).void }
  def bar(x); end
end
```

**Note**: For consistency in large codebases, it's best to avoid
`T::Sig::WithoutRuntime` unless it can't be avoided, because the performance win
versus simply using `.checked(:never)` is marginal. Some examples:

- Circumstances make it impossible to put `extend T::Sig` in the class, but
  `sorbet-runtime` is still loaded.
- The method is `method_missing` or `respond_to_missing?`, as `sorbet-runtime`
  does not support wrapping these methods.
- Some code not under your control is trying to reflect on the result of
  `.parameters` or `.arity` of the sig'ed method. (Note that this precludes code
  under _your_ control: Sorbet provides [`T::Utils.signature_for_method`], which
  exposes the arity of the underlying method.)
- After careful measurements, even the first call to a method is performance
  sensitive. This is rare, but can happen when trying to optimize speed of code
  loading specifically.

If none of these reasons apply, it's best to avoid `T::Sig::WithoutRuntime`,
because it will stand out versus most other usages of `sig` in a codebase.

[`T::Utils.signature_for_method`]:
  https://github.com/sorbet/sorbet/blob/0dbaf2c465314556b5acecec6ec63937f2b9e836/gems/sorbet-runtime/lib/types/utils.rb#L63-L68

## T.let, T.cast, T.must, T.bind

Type assertions like `T.let`, `T.cast`, `T.must`, and `T.bind` are normally
checked at runtime, just like `sig` annotations on methods, unless runtime
checks have been disabled.

Unlike method signatures, type assertions _always_ have a performance cost,
whether or not they are checked at runtime. See
[Type Assertions](type-assertions.md) for tips on patterns that reduce or avoid
this cost.

## What's next?

- [Signatures](sigs.md)

  Method signatures are the primary way that we add static and dynamic type
  checking in our code. Learn the available syntax advanced features of
  signatures.

- [Runtime Configuration](tconfiguration.md)

  Learn how to change or disable Sorbet's runtime type checks via settings and
  callbacks.
