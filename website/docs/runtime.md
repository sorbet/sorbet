---
id: runtime
title: Enabling Runtime Checks
---

As we've mentioned before, Sorbet is a [gradual](gradual.md) system: it can be
turned on and off at will. This means the predications `srb` makes statically
can be wrong.

That's why Sorbet also uses **runtime checks**: even if a static prediction was
wrong, it will get checked during runtime, making things fail loudly and
immediately, rather than silently and asynchronously.

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

[^void]: The case for `.void` in a `sig` is slightly different. See [the docs
for void](sigs.md#returns-void-annotating-return-types).

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
about a codebase. Wrong sigs are actively harmful.

But, by leveraging runtime checks, we can gain lots of confidence in our type
annotations:

- Automated test runs become tests of our type annotations!
- Our production observability and monitoring catch bad sigs **early**, before
  they propagate false assumptions throughout a codebase.

Most people are *either* familiar with a completely typed language (Java, Go,
etc.) or a completely untyped language like Ruby; a [gradual type
system](gradual.md) can be very foreign at first, including these runtime
checks. But it's precisely these runtime checks that make it easier to drive
adoption of types in the long run.


## Disabling runtime checks

Sometimes runtime checks don't make sense. There are two main reasons why

1.  For particularly hot method calls, we might not have the performance budget to
    wrap a method in `sig` checks.

2.  For the purposes of convincing an people to adopt Sorbet widely, we might
    have to make the compromise that adopting Sorbet doesn't affect the runtime
    behavior of the code.

The solution for (1) is to use the [`.soft` or
`.checked`](troubleshooting.md#soft-checked) escape hatch when annotating the
method. See those docs for more information.

For (2), it's possible to change the runtime behavior by defining callbacks. See
[Runtime Configuration](tconfiguration.md) for more about what callbacks are
available and how to register them.

## What's next?

- [Signatures](sigs.md)

  Method signatures are the primary way that we add static and dynamic type
  checking in our code. Learn the available syntax advanced features of
  signatures.
