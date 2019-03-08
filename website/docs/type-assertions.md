---
id: type-assertions
title: Type Assertions
---

> TODO: This page is still a fragment. Contributions welcome!

There are four ways to do type assertions in Sorbet:

- `T.let(expr, Type)`
- `T.cast(expr, Type)`
- `T.must(expr)`
- `T.assert_type!(expr, Type)`

There is also `T.unsafe` which is not a "type assertion" so much as an [Escape
Hatch](troubleshooting.md#escape-hatches).

At a high level:

- A `T.let` assertion is checked statically **and** at runtime.

- A `T.cast` assertion is only checked at runtime. Statically, Sorbet assumes
  this assertion is always true. This can be used to change the result type of
  an expression from the perspective of the static system.

- `T.must` is specifically for [Nilable Types](nilable-types.md). Use it when
  you want to assert that something is not `nil`.

- `T.assert_type!` is similar to `T.let`: it is checked statically **and** at
  runtime. It has the additional restriction that it will **always** fail
  statically if given something that's [`T.untyped`](untyped.md).

Some other ways to think about it:

-   `T.let` vs `T.cast`

    ```ruby
    T.cast(expr, Type)
    ```

    is the same as

    ```ruby
    T.let(T.unsafe(expr), Type)
    ```

-   `T.unsafe` in terms of `T.let`

    ```ruby
    T.unsafe(expr)
    ```

    is the same as

    ```ruby
    T.let(expr, T.untyped)
    ```

-   `T.must` is like `T.cast`, but without having to know the result type:

    ```ruby
    T.cast(nil_or_string, String)
    ```

    is the same as

    ```ruby
    T.must(nil_or_string)
    ```
