---
id: strong
title: Banning untyped from a file
sidebar_label: Banning untyped
---

> **Note**: This feature is in beta. Please give us feedback!

The `# typed: strong` sigil bans all usage of `T.untyped` in a file.

![](/img/strong.png)

Writing code at this level can be extremely exacting. We do not recommend using
this level too early in the life cycle of adopting Sorbet, as it can cause an
unreasonably high barrier to entry when changing the codebase in the future.
Frequently, the best approach to dealing with `# typed: strong`-level errors is
to simply ignore them, by downgrading the sigil to `# typed: strict`.

A better option is to configure Sorbet to highlight untyped code in the editor:

→ [Highlighting untyped code](highlight-untyped.md)

This will give most of the benefits of flagging which code is covered by Sorbet,
while not being a large burden.

For more information on our recommendations when adopting Sorbet, see
[Suggestions for driving adoption](metrics.md#suggestions-for-driving-adoption).

That being said, we welcome enterprising Sorbet users to try writing code at
this level. If you do have feedback for where it's difficult or impossible to
use this level, please let us know.

## What counts as a usage of untyped?

Some examples of things that count as a usage of untyped:

- Calling a method on an untyped value
- Passing an untyped value to a method
- Conditionally branching on an untyped value
- Returning an untyped value from a method
- etc.

The list of things that count as usages of untyped will grow over time (which is
part of the reason why this feature is considered "beta").

### What doesn't count?

It's maybe more interesting to see what **doesn't** count as a usage of untyped:

- `x = T.let(..., SomeType)` where `...` is an untyped expression.

  This is one of the primary ways to fix errors arising in `# typed: strong`
  files. The [`T.let`](type-assertions.md) type assertion ascribes a static type
  to `x`. Using `x` in place of `...` means that Sorbet will see a usage of
  something typed instead of something untyped.

- Any usage of a type like `T::Array[T.untyped]` or
  `T::Hash[String, T.untyped]`, which has `T.untyped` somewhere inside it.

  We may reconsider this decision in the future. Sorbet will, at least, flag
  that usages of **elements** of these values are usages of untyped:

  ```ruby
  sig { params(xs: T::Array[T.untyped]).void }
  def example(xs)
    x0 = xs.first # (nothing reported here)
    x0.even?
  # ^^ Call to method `even?` on `T.untyped`
  end
  ```

- Any place where code type checks only because a method parameter accepts
  `T.untyped`.

  This is a compromise. There are currently too many methods in the wild which
  are typed as taking `T.untyped` to mean "I accept all kinds of values." Sorbet
  currently lacks syntax to declare when this is intentional vs accidental
  because a more precise type has not been declared.

## How can I avoid using `T.untyped`?

It depends on the source of the `T.untyped`.

- If the untyped comes from a method which returns `T.untyped`, consider adding
  a signature to that method (or improving the existing signature).

- If it's not possible to write a method signature which applies to the method
  definition, use `T.let` at each call site of the method which returns
  `T.untyped` to provide a more specific type to the result.

- If the untyped comes from Sorbet's RBI files for the standard library, please
  [make a contribution](faq.md#it-looks-like-sorbets-types-for-the-stdlib-are-wrong)
  to improve them!

- If the untyped comes from other generated RBI files, figure out a way to
  include generated signatures in the RBI files.

- If the untyped comes from a use of `T.unsafe`, either try to remove the call
  to `T.unsafe` and fix any type errors, or replace it with [`T.cast`], which
  will be a more limited type system escape hatch than `T.unsafe`.

Ultimately, fixing the error is similar to fixing any other type error.

[`t.cast`]: type-assertions.md#tcast

## Why is this feature in beta?

This feature is currently under development, and we'd love your feedback!

In addition to the things documented in the
[What doesn't count](#what-doesnt-count) section above, here are some known
limitations:

- Sometimes, the region Sorbet displays in the error message does not precisely
  match the code which is untyped.

  For example, a previous version of this feature accidentally underlined an
  entire block (from `do` until `end`) when the value returned from the block
  was `T.untyped`. This was fixed by reporting exactly the block's return value.

  Please report cases like these!

  You can use [this Sorbet playground] as a starting point to report issues with
  this feature.

- Sometimes, the source of the untyped is Sorbet itself.

  Certain features of Ruby are hard to statically type and have historically
  been supported on a "best effort" basis by marking them `T.untyped`.

  Feel free to browse our [Untyped code milestone] to see if it looks like
  someone has already reported a bug. Otherwise, use [this Sorbet playground] to
  craft a test case and report it to us.

Again, to browse the most up-to-date known limitations with this feature, check
the [Untyped code milestone] for the Sorbet project.

[this sorbet playground]:
  https://sorbet.run/#%23%20typed%3A%20strong%0A%23%20To%20report%20an%20issue%2C%20click%20%22Examples%20%E2%98%B0%20%3E%20Create%20issue%20with%20example%22%0A%0AT.unsafe%28nil%29.foo
[this faq entry]: faq#it-looks-like-sorbets-types-for-the-stdlib-are-wrong
[untyped code milestone]: https://github.com/sorbet/sorbet/milestone/20
