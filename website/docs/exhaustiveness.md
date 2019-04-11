---
id: exhaustiveness
title: Exhaustiveness Checking
---

> **Note**: Sorbet does not currently have first-class support for
> exhaustiveness checks. This feature is planned, but has not yet been
> implemented. Here we document a current workaround.

Using [Flow-Sensitive Typing](flow-sensitive.md), [Union Types](union-types.md),
and [Type Assertions](type-assertions.md) we can approximate exhaustiveness
checking. **Exhaustiveness checking** is a feature of a language where the type
checker guarantees that the programmer has covered all cases. It can be super
useful at catching pesky edge cases before they become bugs!

## tl;dr

If you already know what exhaustiveness checking is, you might just want to see
the end result:

```ruby
# typed: true
extend T::Sig

class A; end
class B; end
class C; end

AorBorC = T.type_alias(T.any(A, B, C))

def do_a(a); puts 'Got an A!'; end
def do_b(b); puts 'Got a B!'; end
def do_c(c); puts 'Got a C!'; end

# (1) We have a union type of A, B, or C
sig {params(x: AorBorC).void}
def foo(x)
  # (2) We use flow-sensitivity to cover each case separately
  case x
  when A
    # To re-iterate: within this branch, Sorbet knows x is an A
    T.reveal_type(x) # Revealed type: `A`

    do_a(x)

  when B
    do_b(x)

  else
    # (3) Use a type assertion to require that x MUST be C
    T.let(x, C)
    #        ^ If the type of x ever widens to include other types,
    #          sorbet will report an error here.

    do_c(x)
  end
end
```

Otherwise, let's walk through an example explaining not only **how** we can get
exhaustiveness checking, but also **why** it's useful:


## Example

Let's say we have this setup:

```ruby
# typed: true
extend T::Sig

class A; end
class B; end

sig {params(x: T.any(A, B)).void}
def foo(x)
  # ...
end
```

There are two classes (`A` and `B`), and our method `foo` takes either `A` or
`B`.

In the body of `foo`, we'd like to do something different when given an `A`
versus when given a `B`:

```ruby
# ...

def do_a(a); puts 'Got an A!'; end
def do_b(b); puts 'Got a B!'; end

sig {params(x: T.any(A, B)).void}
def foo(x)
  # Problematic! We'll improve this shortly...
  case x
  when A
    do_a(x)
  when B
    do_b(x)
  end
end
```

There's no bug here yet, but consider that some time in the future, someone
wants to update `foo` to work with class `C`:

```ruby
# ...

class C; end
def do_c(c); puts 'Got a C!'; end

# ...

sig {params(x: T.any(A, B, C)).void}
def foo(x)
  # Bug! We forgot to update the body to handle C...
  case x
  when A
    do_a(x)
  when B
    do_b(x)
  end
end
```

In this case, there's a silent bug in our program. We've updated the signature
of `foo` to accept instances of `C`, but we haven't updated the method body to
actually do something with it!

**Exhaustiveness checking** is a feature that turns this kind of bug into a type
error. It lets us catch the problem statically before causing all sorts of
problems down the line.

We can enable exhaustivness checking in Sorbet using a [type
assertion](type-assertions.md):

```ruby
sig {params(x: T.any(A, B, C)).void}
def foo(x)
  case x
  when A
    do_a(x)
  else
    T.let(x, B) # error: Argument does not have accepted type: `B`
                #        Got: `T.any(B, C)`
    do_b(x)
  end
end
```

In this case, Sorbet is telling us that by the time we got to the else case, we
were missing a case: both `B` and `C` needed to be handled, but we were only
handling `B`.

Putting everything together, this is what our final program looks like. It
handles all the cases, and prevents against other people from forgetting to
handle any new cases that get added.

```ruby
# typed: true
extend T::Sig

class A; end
class B; end
class C; end

def do_a(a); puts 'Got an A!'; end
def do_b(b); puts 'Got a B!'; end
def do_c(c); puts 'Got a C!'; end

# (1) We have a union type of A, B, or C
sig {params(x: T.any(A, B, C)).void}
def foo(x)
  # (2) We use flow-sensitivity to cover each case separately
  case x
  when A
    # To re-iterate: within this branch, Sorbet knows x is an A
    T.reveal_type(x) # Revealed type: `A`

    do_a(x)

  when B
    do_b(x)

  else
    # (3) Use a type assertion to require that x MUST be C
    T.let(x, C)
    #        ^ If the type of x ever widens to include other types,
    #          sorbet will report an error here.

    do_c(x)
  end
end
```

> **Tip**: We can use [Type Aliases](type-aliases.md) to give a name to
> `T.any(A, B, C)` and reuse it throughout our codebase. This means we can
> update the alias in one place, instead of at every individual method!
