---
id: generics
title: Generic Classes and Methods
sidebar_label: Generics
---

Sorbet has syntax for creating generic methods, classes, and interfaces.

## How to use generics well

Despite many improvements made to Sorbet's support for generics over the years,
it is unfortunately easy to both:

1.  Use generics incorrectly, and not be told as much by Sorbet
1.  Use generics "correctly," only to realize that the abstractions you've built
    are not easy to use.

It is therefore important to thoroughly test abstractions making use of Sorbet
generics.

The tests you'll need to write look materially different from other Ruby tests
you may be accustomed to writing, because the tests need to deal with what code
should or should not typecheck, rather than what code should or should not run
correctly.

- Sometimes, something that shouldn't type check **does type check anyways**.

  This is bad because the generic abstraction being built will not necessarily
  provide the guarantees it should. This can give users of the generic
  abstraction false confidence in the type system.

  To mitigate this, write example code that should not type check and double
  check that it doesn't. Get creative with these tests. Consider writing tests
  that make uncommon use of subtyping, inheritance, mutation, etc.

  (There is nothing built into Sorbet for writing such tests. The easiest
  approach is to manually build small examples using the new API that don't type
  check, but don't check the resulting files in (or check them in, but mark them
  `# typed: ignore`, and bump the sigil up temporarily while making changes).
  The diligent may way to automate this by running Sorbet a second time on a
  codebase that includes extra files meant to not type check, asserting that
  Sorbet indeed reports errors.)

  It can also be helpful to use `T.reveal_type` and/or `T.assert_type!` to
  inspect the types of **intermediate values** to see if `T.untyped` has
  silently snuck in somewhere. If `T.untyped` has snuck into the implementation
  somewhere, things will type check, but it won't mean much.

- Sometimes, something **doesn't type check when it should**.

  Most of these kinds of bugs in Sorbet were fixed as of July 2022, but some
  remain.

  These kinds of problems are bad because they cause confusion and frustration
  for people attempting to **use** the generic abstraction, not for the person
  who implemented the abstraction. This is especially painful for those who are
  new to Sorbet (or even Ruby), as well as those those who are not intimately
  familiar with the limitations of Sorbet's generics.

  To mitigate this, "test drive" the abstraction being built. Don't assume that
  if the implementation type checks that it will work for downstream users. Get
  creative and write code as a user would.

  Encountering these errors is not only frustrating for you, but also
  frustrating for others, and incurs a real risk of making people's first
  experience with Sorbet unduly negative.

By avoiding both of these kinds of outcomes, you will be able to build generic
abstractions that work better overall.

As with all bugs in Sorbet, when you encounter them [please report
them][issues]. See the list of known bugs here:

→ [Generics milestone]

## Basic syntax

The basic syntax for class generics in Sorbet looks like this:

```ruby
# typed: strict

class Box
  extend T::Sig
  extend T::Generic # Provides `type_member` helper

  Elem = type_member # Makes the `Box` class generic

  # References the class-level generic `Elem`
  sig {params(val: Elem).void}
  def initialize(val:); @val = val; end
  sig {returns(Elem)}
  def val; @val; end
  sig {params(val: Elem).returns(Elem)}
  def val=(val); @val = val; end
end

int_box = Box[Integer].new(val: 0)
T.reveal_type(int_box) # `Box[Integer]`

T.reveal_type(int_box.val) # `Integer`

int_box.val += 1
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20strict%0A%0Aclass%20Box%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AGeneric%20%23%20Provides%20%60type_member%60%20helper%0A%0A%20%20Elem%20%3D%20type_member%20%23%20Makes%20the%20%60Box%60%20class%20generic%0A%0A%20%20%23%20References%20the%20class-level%20generic%20%60Elem%60%0A%20%20sig%20%7Bparams%28val%3A%20Elem%29.void%7D%0A%20%20def%20initialize%28val%3A%29%3B%20%40val%20%3D%20val%3B%20end%0A%20%20sig%20%7Breturns%28Elem%29%7D%0A%20%20def%20val%3B%20%40val%3B%20end%0A%20%20sig%20%7Bparams%28val%3A%20Elem%29.returns%28Elem%29%7D%0A%20%20def%20val%3D%28val%29%3B%20%40val%20%3D%20val%3B%20end%0Aend%0A%0Aint_box%20%3D%20Box%5BInteger%5D.new%28val%3A%200%29%0AT.reveal_type%28int_box%29%20%23%20%60Box%5BInteger%5D%60%0A%0AT.reveal_type%28int_box.val%29%20%23%20%60Integer%60%0A%0Aint_box.val%20%2B%3D%201)

The basic syntax for function generics in Sorbet looks like this:

```ruby
# typed: true
extend T::Sig

sig do
  # `extend T::Generic` is not required just to use `type_parameters`
  type_parameters(:U)
    .params(
      # The block can return any value, and the type of
      # that value defines type_parameter(:U)
      blk: T.proc.returns(T.type_parameter(:U))
    )
    # The method returns whatever the block returns
    .returns(T.type_parameter(:U))
end
def with_timer(&blk)
  start = Time.now
  res = yield
  duration = Time.now - start
  puts "Running block took #{duration.round(1)}s"
  res
end

res = with_timer do
  sleep 2
  puts 'hello, world!'
  # Block returns an Integer
  123
end
# ... therefore the method returns an Integer
T.reveal_type(res) # `Integer`
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20do%0A%20%20type_parameters%28%3AU%29%0A%20%20%20%20.params%28%0A%20%20%20%20%20%20blk%3A%20T.proc.returns%28T.type_parameter%28%3AU%29%29%0A%20%20%20%20%29%0A%20%20%20%20.returns%28T.type_parameter%28%3AU%29%29%0Aend%0Adef%20with_timer%28%26blk%29%0A%20%20start%20%3D%20Time.now%0A%20%20res%20%3D%20yield%0A%20%20duration%20%3D%20Time.now%20-%20start%0A%20%20puts%20%22Running%20block%20took%20%23%7Bduration.round%281%29%7Ds%22%0A%20%20res%0Aend%0A%0Ares%20%3D%20with_timer%20do%0A%20%20sleep%202%0A%20%20puts%20'hello%2C%20world!'%0A%20%20123%0Aend%0AT.reveal_type%28res%29)

## Generics and runtime checks

Recall that Sorbet is not only a static type checker, but also a system for
[validating types at runtime](runtime.md).

However, Sorbet completely erases generic types at runtime, both for classes and
methods. When Sorbet sees a signature like `Box[Integer]`, at runtime it will
**only** check whether an argument has class `Box` (or a subtype of `Box`), but
nothing about the types that argument has been applied to. Generic types are
only checked statically. Similarly, if Sorbet sees a signature like

```ruby
sig do
  type_parameters(:U)
    .params(
      x: T.type_parameter(:U),
      y: T.type_parameter(:U),
    )
    .void
end
def foo(x, y); end
```

Sorbet will not check that `x` and `y` are the same class at runtime.

Since generics are only checked statically, this removes using tests as a way to
guard against misuses of [`T.untyped`](untyped.md). For example, Sorbet will
neither report a static error nor a runtime error on this example:

```ruby
sig {params(xs: Box[Integer]).void}
def foo(xs); end

untyped_box = Box[T.untyped].new(val: 'not an int')
foo(untyped_box)
#   ^^^^^^^^^^^ no static error, AND no runtime error!
```

Another consequence of having erased generics is that things like this will not
work:

```ruby
if box.is_a?(Box[Integer]) # error!
  # do something when `box` contains an Integer
elsif box.is_a?(Box[String]) # error!
  # do something when `box` contains a String
end
```

Sorbet will attempt to detect cases where it looks like this is happening and
report a static error, but it cannot do so in all cases.

The workaround is to check only the class type of the generic class, and check
any element type before it's used:

```ruby
if box.is_a?(Box)
  val = box.val
  if val.is_a?(Integer)
    # ...
  elsif val.is_a?(String)
    # ...
  end
end
```

## `type_member` & `type_template`

The `type_member` and `type_template` annotations declare class-level generic
type variables.

```ruby
class A
  X = type_member
  Y = type_template
end
```

Type variables, like normal Ruby variables, have a scope:

- The scope of a `type_member` is all instance methods on the given class. They
  are most commonly used for generic container classes, because each instance of
  the class may have a separate type substituted for the type variable.

- The scope of a `type_template` is all singleton class methods on the given
  class. Since a class only has one singleton class, `type_template` variables
  are usually used as a way for an abstract parent class to require a concrete
  child class to pick a specific type that all instances agree on.

One way to think about it is that `type_template` is merely a shorter name for
something which could have also been named `singleton_class_type_member`. In
Sorbet's implementation, `type_member` and `type_template` are treated almost
exactly the same.

Note that this means that it's not possible to refer to a `type_template`
variable from an instance method. For a workaround, see the docs for error code
[5072](error-reference.md#5072).

## `:in`, `:out`, and variance

Understanding variance is important for understanding how `type_member`'s and
`type_template`'s behave. Variance is a type system concept that controls how
generics interact with subtyping. Specifically, [from Wikipedia][variance]:

_"Variance refers to how subtyping between more complex types relates to
subtyping between their components."_

Variance is a property of each `type_member` and `type_template` (not the
generic class itself, because generic classes may have more than one such type
variable). There are three kinds of variance relationships:

- **invariant** (subtyping relationships are ignored for this type variable)
- **covariant** (subtyping order is preserved for this type variable)
- **contravariant** (subtyping order is reversed for this type variable)

Here is the syntax Sorbet uses for these concepts:

```ruby
module Example
  # invariant type member
  X = type_member

  # covariant type member
  Y = type_member(:out)

  # contravariant type member
  Z = type_member(:in)
end
```

In this example, we would say:

- "`Example` is invariant in `X`",
- "`Example` is covariant in `Y`", and
- "`Example` is contravariant in `Z`", and

For those who have never encountered variance in a type system before, it may be
useful to skip down to
[Why does tracking variance matter?](#why-does-tracking-variance-matter), which
motivates why type systems (Sorbet included) place such emphasis on variance.

### Invariance

(_For convenience throughout these docs, we use the annotation `<:` to claim
that one type is a subtype of another type._)

By default, `type_member`'s and `type_template`'s are invariant. Here is an
example of what that means:

```ruby
class Box
  extend T::Generic
  # no variance annotation, so invariant by default
  Elem = type_member
end

int_box = Box[Integer].new

# Integer <: Numeric, because Integer inherits from Numeric, however:
T.let(int_box, Box[Numeric])
# ^ error: Argument does not have asserted type

# Elem is invariant, so the claim
#   Box[Integer] <: Box[Numeric]
# is not true
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20strict%0A%0Aclass%20Box%0A%20%20extend%20T%3A%3AGeneric%0A%20%20%23%20no%20variance%20annotation%2C%20so%20invariant%20by%20default%0A%20%20Elem%20%3D%20type_member%0Aend%0A%0Aint_box%20%3D%20Box%5BInteger%5D.new%0A%0A%23%20Integer%20%3C%3A%20Numeric%2C%20because%20Integer%20inherits%20from%0A%23%20Numeric%2C%20however%3A%0AT.let%28int_box%2C%20Box%5BNumeric%5D%29%0A%23%20%5E%20error%3A%20Argument%20does%20not%20have%20asserted%20type%0A%0A%23%20Elem%20is%20invariant%2C%20so%20the%20claim%0A%23%20%20%20Box%5BInteger%5D%20%3C%3A%20Box%5BNumeric%5D%0A%23%20is%20not%20true)

Since `Elem` is invariant (has no explicit variance annotation), Sorbet reports
an error on the `T.let` attempting to widen the type of `int_box` to
`Box[Numeric]`. Two objects of a given generic class with an invariant type
member (`Box` in this example) are only subtypes if the types bound to their
invariant `type_member`'s are **equivalent**.

Invariant `type_member`'s and `type_template`'s, unlike covariant and
contravariant ones, may be used in **both** input and output positions within
method signatures. This nuance is explained in more detail in the next sections
about covariance and contravariance.

> **Note**: all `type_member`'s and `type_template`'s in a Ruby `class` must be
> invariant. Only `type_member`'s in a Ruby `module` are allowed to be covariant
> or contravariant. See the docs for error code [5016](error-reference.md#5016)
> for more information.

### Covariance (`:out`)

Covariant type variables preserve the subtyping relationship. Specifically, if
the type `Child` is a subtype of the type `Parent`, then the type `M[Child]` is
a subtype of the type `M[Parent]` if the type member of `M` is covariant. In
symbols:

```
Child <: Parent  ==>  M[Child] <: M[Parent]
```

Note that only a Ruby `module` (not a `class`) may have a covariant
`type_member`. (See the docs for error code [5016](error-reference.md#5016) for
more information.) Note that since `type_template` creates a type variable
scoped to a singleton class, `type_template` can never be covariant (because all
singleton classes are classes, even singleton classes of modules).

Here's an example of a module that has a covariant type member:

```ruby
extend T::Sig

# covariant `Box` interface
module IBox
  extend T::Generic
  # `:out` declares this type member as covariant
  Elem = type_member(:out)
end

sig {params(int_box: IBox[Integer]).void}
def example(int_box)
  T.let(int_box, IBox[Numeric]) # OK
end
```

In this case, the `T.let` assertion reports no static errors because
`Integer <: Numeric`, and therefore `IBox[Integer] <: IBox[Numeric]`.

Covariant type members may only appear in **output** positions (thus the `:out`
annotation). For more information about what an output position is, see
[Input and output positions](#input-and-output-positions) below.

In practice, covariant type members are predominantly useful for creating
interfaces that produce values of the specified type. For example:

```ruby
module IBox
  extend T::Sig
  extend T::Generic
  abstract!

  # Covariant type member
  Elem = type_member(:out)

  # Elem can only be used in output position
  sig {abstract.returns(Elem)}
  def value; end
end

class Box
  extend T::Sig
  extend T::Generic

  # Implement the `IBox` interface
  include IBox

  # Redeclare the type member, to be compatible with `IBox`
  Elem = type_member

  # Within this class, `Elem` is invariant, so it can also be used in the input position
  sig {params(value: Elem).void}
  def initialize(value:); @value = value; end

  # Implement the `value` method from `IBox`
  sig {override.returns(Elem)}
  def value; @value; end

  # Add the ability to update the value
  # (allowed because `Elem` is invariant within this class)
  sig {params(value: Elem).returns(Elem)}
  def value=(value); @value = value; end
end
```

[→ View on sorbet.run][covariant-ibox]

Note how in the above example, `Box` includes `IBox`, meaning that `Box` is a
child of `IBox`. Children of generic classes or modules must always redeclare
any type members declared by the parent, in the same order. The child must
either copy the parent's specified variance or redeclare it as invariant. When
the child is a `class` (not a `module`), redeclaring it as invariant is the
**only** option.

### Contravariance (`:in`)

Contravariant type parameters **reverse** the subtyping relationship.
Specifically, if the type `Child` is a subtype of the type `Parent`, then type
`M[Parent]` is a subtype of the type `M[Child]` if the type member of `M` is
contravariant. In symbols:

```
Child <: Parent  ==>  M[Parent] <: M[Child]
```

Contravariance is **quite** unintuitive for most people. Luckily, contravariance
is not unique to Sorbet—all type systems that have both subtyping relationships
and generics must grapple with variance, contravariance included, so there is a
lot written about it elsewhere online. It maybe even be helpful to read about
contravariance in a language you already have extensive familiarity with, as
many of the concepts will transfer to Sorbet.

The way to understand contravariance is by understanding which function types
are subtypes of other function types. For example:

```ruby
sig do
  params(
    f: T.proc.params(x: Child).void
  )
  .void
end
def takes_func(f)
  f.call(Child.new)
  f.call(GrandChild.new)
end

wants_at_least_parent = T.let(
  ->(parent) {parent.on_parent},
  T.proc.params(parent: Parent).void
)
takes_func(wants_at_least_parent) # OK

wants_at_least_child = T.let(
  ->(child) {child.on_child},
  T.proc.params(child: Child).void
)
takes_func(wants_at_least_child) # OK

wants_at_least_grandchild = T.let(
  ->(grandchild) {grandchild.on_grandchild},
  T.proc.params(grandchild: GrandChild).void
)
takes_func(wants_at_least_grandchild) # error!
```

[→ View full example on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Aclass%20Parent%0A%20%20def%20on_parent%3B%20end%0Aend%0Aclass%20Child%20%3C%20Parent%0A%20%20def%20on_child%3B%20end%0Aend%0Aclass%20GrandChild%20%3C%20Child%0A%20%20def%20on_grandchild%3B%20end%0Aend%0A%0Asig%20do%0A%20%20params%28%0A%20%20%20%20f%3A%20T.proc.params%28x%3A%20Child%29.void%0A%20%20%29%0A%20%20.void%0Aend%0Adef%20takes_func%28f%29%0A%20%20f.call%28Child.new%29%0A%20%20f.call%28GrandChild.new%29%0Aend%0A%0Awants_at_least_grandchild%20%3D%20T.let%28%0A%20%20-%3E%28grandchild%29%20%7Bgrandchild.on_grandchild%7D%2C%0A%20%20T.proc.params%28grandchild%3A%20GrandChild%29.void%0A%29%0Awants_at_least_child%20%3D%20T.let%28%0A%20%20-%3E%28child%29%20%7Bchild.on_child%7D%2C%0A%20%20T.proc.params%28child%3A%20Child%29.void%0A%29%0Awants_at_least_parent%20%3D%20T.let%28%0A%20%20-%3E%28parent%29%20%7Bparent.on_parent%7D%2C%0A%20%20T.proc.params%28parent%3A%20Parent%29.void%0A%29%0A%0Atakes_func%28wants_at_least_child%29%0Atakes_func%28wants_at_least_parent%29%0A%0Atakes_func%28wants_at_least_grandchild%29%20%23%20error)

In this example, `takes_func` requests that it be given an argument `f` that,
when called, can be given `Child` instances. As we see in the method body of
`takes_func`, it's valid to call `f` on both `Child` and `GrandChild` instances
(`class GrandChild < Child`, so all `GrandChild` instances are also `Child`
instances).

At the call site, both `wants_at_least_child` and `wants_at_least_parent`
satisfy the contract that `takes_func` is asking for. In particular, the
`wants_at_least_parent` is fine being given **any** instance, as long as it's
okay to call `parent.on_parent` (because of inheritance, both `Child` and
`GrandChild` have this method). Since `takes_func` guarantees that it will
always provide a `Child` instance, the thing provided will always have an
`on_parent` method defined.

For that reason, Sorbet is okay treating `T.proc.params(parent: Parent).void` as
a subtype of `T.proc.params(child: Child).void`, even though `Child` is a
subtype of `Parent`.

Meanwhile, it's not okay to call `takes_func(wants_at_least_grandchild)`,
because sometimes `takes_func` will only provide a `Child` instance, which would
not have the `on_grandchild` method available to call (which is being called
inside the `wants_at_least_grandchild` function).

When it comes to user-defined generic classes using contravariant type members,
the cases where this is useful is usually building generic abstractions that are
"function like." For example, maybe a generic task-processing abstraction:

```ruby
module ITask
  extend T::Sig
  extend T::Generic
  abstract!

  ParamType = type_member(:in)

  sig {abstract.params(input: ParamType).returns(T::Boolean)}
  def do_task(input); end

  sig {params(input: T.all(ParamType, BasicObject)).returns(T::Boolean)}
  def do_task_with_logging(input)
    Kernel.puts(input)
    res = do_task(input)
    Kernel.puts(res)
    res
  end
end

class Task
  extend T::Sig
  extend T::Generic

  include ITask

  ParamType = type_member

  sig {params(fn: T.proc.params(param: ParamType).returns(T::Boolean)).void}
  def initialize(&fn)
    @fn = fn
  end

  sig {override.params(input: ParamType).returns(T::Boolean)}
  def do_task(input); @fn.call(input); end
end

sig {params(task: ITask[Integer]).void}
def example(task)
  i = 0
  while task.do_task_with_logging(i)
    i += 1
  end
end

takes_int_task = Task[Integer].new {|param| param < 10}

example(takes_int_task)
```

[→ View full example on sorbet.run](https://sorbet.run/#%23%20typed%3A%20strict%0Aextend%20T%3A%3ASig%0A%0Amodule%20ITask%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AGeneric%0A%20%20abstract!%0A%0A%20%20ParamType%20%3D%20type_member%28%3Ain%29%0A%0A%20%20sig%20%7Babstract.params%28input%3A%20ParamType%29.returns%28T%3A%3ABoolean%29%7D%0A%20%20def%20do_task%28input%29%3B%20end%0A%0A%20%20sig%20%7Bparams%28input%3A%20T.all%28ParamType%2C%20BasicObject%29%29.returns%28T%3A%3ABoolean%29%7D%0A%20%20def%20do_task_with_logging%28input%29%0A%20%20%20%20Kernel.puts%28input%29%0A%20%20%20%20res%20%3D%20do_task%28input%29%0A%20%20%20%20Kernel.puts%28res%29%0A%20%20%20%20res%0A%20%20end%0Aend%0A%0Aclass%20Task%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AGeneric%0A%0A%20%20include%20ITask%0A%0A%20%20ParamType%20%3D%20type_member%0A%0A%20%20sig%20%7Bparams%28fn%3A%20T.proc.params%28param%3A%20ParamType%29.returns%28T%3A%3ABoolean%29%29.void%7D%0A%20%20def%20initialize%28%26fn%29%0A%20%20%20%20%40fn%20%3D%20fn%0A%20%20end%0A%0A%20%20sig%20%7Boverride.params%28input%3A%20ParamType%29.returns%28T%3A%3ABoolean%29%7D%0A%20%20def%20do_task%28input%29%3B%20%40fn.call%28input%29%3B%20end%0Aend%0A%0Asig%20%7Bparams%28task%3A%20ITask%5BInteger%5D%29.void%7D%0Adef%20example%28task%29%0A%20%20i%20%3D%200%0A%20%20while%20task.do_task_with_logging%28i%29%0A%20%20%20%20i%20%2B%3D%201%0A%20%20end%0Aend%0A%0Atakes_int_task%20%3D%20Task%5BInteger%5D.new%20%7B%7Cparam%7C%20param%20%3C%2010%7D%0A%0Aexample%28takes_int_task%29)

### Input and output positions

Understanding where covariant and contravariant type members can appear requires
knowing which places in a method signature are **output** positions, and which
are **input** positions.

An obvious output position is a method signature's `returns` annotation, but
there are more than just that. As an intuition, all positions in a signature
where the value is produced by some computation in the method's body are output
positions. This includes values yielded to lambda functions and block arguments.

```ruby
module IBox
  extend T::Sig
  extend T::Generic
  abstract!

  Elem = type_member(:out)

  sig {abstract.returns(Elem)}
  #                     ^^^^ output position
  def value; end

  sig do
    type_parameters(:U)
      .params(
        blk: T.proc.params(val: Elem).returns(T.type_parameter(:U))
        #                       ^^^^ output position
      )
      .returns(T.type_parameter(:U))
  end
  def with_value(&blk)
    yield value
  end
end
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20strict%0Aextend%20T%3A%3ASig%0A%0Amodule%20IBox%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AGeneric%0A%20%20abstract!%0A%0A%20%20Elem%20%3D%20type_member%28%3Aout%29%0A%0A%20%20sig%20%7Babstract.returns%28Elem%29%7D%0A%20%20%23%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%5E%5E%5E%5E%20output%20position%0A%20%20def%20value%3B%20end%0A%0A%20%20sig%20do%0A%20%20%20%20type_parameters%28%3AU%29%0A%20%20%20%20%20%20.params%28%0A%20%20%20%20%20%20%20%20blk%3A%20T.proc.params%28val%3A%20Elem%29.returns%28T.type_parameter%28%3AU%29%29%0A%20%20%20%20%20%20%20%20%23%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%20%5E%5E%5E%5E%20output%20position%0A%20%20%20%20%20%20%29%0A%20%20%20%20%20%20.returns%28T.type_parameter%28%3AU%29%29%0A%20%20end%0A%20%20def%20with_value%28%26blk%29%0A%20%20%20%20yield%20value%0A%20%20end%0Aend)

In this example, both the result type of the `value` method and the `val`
parameter that will be yielded to the `blk` parameter of `with_value` are output
positions.

(The intuition for input positions is flipped: they're all positions that would
correspond to an input to the function, instead of all things that the function
produces. This includes the direct arguments of the method, as well as the
return values of any lambda functions or blocks passed into the method.)

If it helps, some type systems actually formalize the type of a function as a
generic something like this:

```ruby
module Fn
  extend T::Sig
  extend T::Generic
  interface!

  Input = type_member(:in)
  Output = type_member(:out)

  sig {abstract.params(input: Input).returns(Output)}
  def call(input); end
end

sig do
  params(
    fn: Fn[Integer, String],
    x: Integer,
  )
  .returns(String)
end
def example(fn, x)
  res = fn.call(x)
  res
end
```

In the above example, `Fn[Integer, String]` is the type of a function that in
Sorbet syntax would look like this:

```ruby
T.proc.params(arg0: Integer).returns(String)
```

In fact, Sorbet uses exactly this trick. The `T.proc` syntax that Sorbet uses to
model to model [procs and lambdas](procs.md) is just syntactic sugar for
something that looks like the `Fn` type above (there are some gotchas around
functions that take zero parameters or more than one parameter, but the concept
is the same).

Another intuition which may help knowing which positions are input and output
positions: treat function return types as `1` and function parameters as `-1`.
As you pick apart a function type, multiply these numbers together. A positive
result means the result is an output position, while a negative means it's an
input.

```ruby
-1   +1
 A -> B

┌── -1 ──┐    ┌── +1 ──┐
 -1   +1       -1   +1
( C -> D ) -> ( E -> F )
```

In the first example, a simple function from type `A` to type `B`, `B` is the
result of the function, so it's clearly in an output position. Similarly, `A` is
in an input position.

The second example is the type of a function that takes a function as a
parameter, having type `C -> D`, and produces another function as its output,
having type `E -> F`. In this example, `C` is in the input position of an input
position, making type `C` actually be in output position overall
(`-1 × -1 = +1`). `D` is in the output position of an input position, and `E` is
in the input position of an output position, so they're both in input positions
(`-1 × +1 = -1`). `F` is in the output position of an output position, so it's
also in output position (`+1 × +1 = +1`).

### Why does tracking variance matter?

To get a sense for why Sorbet places constraints on where covariant and
contravariant type members can appear within signatures, consider this example,
which continues the example from the [covariance section](#covariance) above:

```ruby
int_box = Box[Integer].new(value: 0)

# not allowed (attempts to widen type,
# but `Box::Elem` is invariant)
int_or_str_box = T.let(int_box, Box[T.any(Integer, String)])

# no error reported here
int_or_str_box.value = ''

T.reveal_type(int_box.value)
# Sorbet reveals: `Integer`
# Actual type at runtime: `String`
```

[→ View full example on sorbet.run][covariant-ibox]

The example starts with a `Box[Integer]`. Obviously, Sorbet should only allow
this `Box` to store `Integer` values, and when reading values out of this box we
should also be guaranteed to get an `Integer`.

If Sorbet allowed widening the type with the `T.let` in the example, then
`int_or_str_box` would have type `Box[T.any(Integer, String)]`. Sensibly, Sorbet
allows using `int_or_str_box` to write the value `''` into the `value` attribute
on the `Box`.

But that's a contradiction! `int_box` and `int_or_str_box` are the same value at
runtime. The variables have different names and different types, but they're the
same object in memory at runtime. On the last line when we read `int_box.value`,
instead of reading `0`, we'll read a value of `''`, which is bad—Sorbet
statically declares that `int_box.value` has type `Integer`, which is out of
sync with the runtime reality.

This is what variance checks buy in a type system: they prevent abstractions
from being misused in ways that would otherwise compromise the integrity of the
type checker's predictions.

## A `type_template` example

So far, the discussion in this guide has focused on `type_member`'s, which tend
to be most useful for building things like generic containers.

The use cases for `type_template`'s tend to look different: they tend to be used
when a class wants to have something like an "abstract" type that is filled in
by child classes. Here's an example of an abstract RPC (remote procedure call)
interface, which uses `type_template`:

```ruby
module AbstractRPCMethod
  extend T::Sig
  extend T::Generic

  abstract!

  # Note how these use `type_member` in this interface module
  # They become `type_template` because we `extend` this module
  # in the child class
  RPCInput = type_member
  RPCOutput = type_member

  sig {abstract.params(input: RPCInput).returns(RPCOutput)}
  def run(input); end
end

class TextDocumentHoverMethod
  extend T::Sig
  extend T::Generic

  # Use `extend` to start implementing the interface
  extend AbstractRPCMethod

  # The `type_member` become `type_template` because of the `extend`
  # We're using `fixed` to "fill in" the type_template. Read more below.
  RPCInput = type_template {{fixed: TextDocumentPositionParams}}
  RPCOutput = type_template {{fixed: HoverResponse}}

  sig {override.params(input: RPCInput).returns(RPCOutput)}
  def self.run(input)
    puts "Computing hover request at #{input.position}"
    # ...
  end
end
```

[→ View full example on sorbet.run][abstract_rpc_method]

The snippet above is heavily abbreviated to demonstrate some new concepts
(`type_template` and `fixed`). The full example on sorbet.run contains many more
details, and it's strongly recommended reading.

There are a couple interesting things happening in the example above:

- We have a generic interface `AbstractRPCMethod` which says that it's generic
  in `RPCInput` and `RPCOutput`. It then mentions these types in the abstract
  `run` method. The example uses `type_member` to declare these generic types.

- The interface is implemented by a class that uses `extend` to implement the
  interface using the singleton class of `TextDocumentHoverMethod`. As we know
  from [Abstract Classes and Interfaces](abstract.md), that means
  `TextDocumentHoverMethod` must implement `def self.run`, _not_ `def run`.

  In the same way, the `type_member` variables declared by the parent must be
  redeclared by the implementing class, where they then become `type_template`.
  Recall from the [`type_member` & `type_template`](#type_member--type_template)
  section that the scope of a `type_template` is all singleton class methods on
  the given class.

- In the implementation, `TextDocumentHoverMethod` chooses to provided a
  [`fixed` annotation](#bounds-on-type-member-s-and-type-template-s-fixed-upper-lower)
  on the `type_template` definitions. This effectively says that
  `TextDocumentHoverMethod` **always** conforms to the type
  `AbstractRPCMethod[TextDocumentPositionParams, HoverResponse]`. We'll discuss
  `fixed` further below.

  Then when implementing the `def self.run` method, it can assume that
  `RPCInput` is equivalent to `TextDocumentPositionParams`. This allows it to
  access `input.position` in the implementation, a method that only exists on
  `TextDocumentPositionParams` (but not necessarily on every input to an
  `AbstractRPCMethod`).

Again, for more information, be sure to view [the full
example][abstract_rpc_method].

## Bounds on `type_member`'s and `type_template`'s (`fixed`, `upper`, `lower`)

The `fixed` annotation in the [example above](#type_templates-and-bounds) places
**bounds** on a `type_template`. There are three annotations for providing
bounds to a `type_member` or `type_template`:

- `upper`: Places an upper bound on types that can be applied to a given type
  member. Only that are subtypes of that upper bound are valid.

- `lower`: The opposite—places a lower bound, thus requiring only supertypes of
  that bound.

- `fixed`: Syntactic sugar for specifying both `lower` and `upper` at the same
  time. Effectively requires that an equivalent type be applied to the type
  member. Sorbet then uses this fact to never require that an explicit type
  argument be provided to the class.

```ruby
class NumericBox
  extend T::Generic
  Elem = type_member {{upper: Numeric}}
end

class IntBox < NumericBox
  Elem = type_member {{fixed: Integer}}
end

NumericBox[Integer].new # OK, Integer <: Numeric
NumericBox[String].new
#          ^^^^^^ error: `String` is not a subtype of upper bound of `Elem`

IntBox.new
# ^ Does not need to be invoked like `IntBox[Integer]` because Sorbet can
#   trivially infer the type argument
```

Placing the bound on the type member makes it an error to ever instantiate a
class with that member outside the given bound.

## Generic methods

Methods can also be made generic in Sorbet:

```ruby
# typed: true
extend T::Sig

sig do
  type_parameters(:U)
    .params(
      blk: T.proc.returns(T.type_parameter(:U))
    )
    .returns(T.type_parameter(:U))
end
def with_timer(&blk)
  start = Time.now
  res = yield
  duration = Time.now - start
  puts "Running block took #{duration.round(1)}s"
  res
end

res = with_timer do
  sleep 2
  puts 'hello, world!'
  123
end
T.reveal_type(res) # `Integer`
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20do%0A%20%20type_parameters%28%3AU%29%0A%20%20%20%20.params%28%0A%20%20%20%20%20%20blk%3A%20T.proc.returns%28T.type_parameter%28%3AU%29%29%0A%20%20%20%20%29%0A%20%20%20%20.returns%28T.type_parameter%28%3AU%29%29%0Aend%0Adef%20with_timer%28%26blk%29%0A%20%20start%20%3D%20Time.now%0A%20%20res%20%3D%20yield%0A%20%20duration%20%3D%20Time.now%20-%20start%0A%20%20puts%20%22Running%20block%20took%20%23%7Bduration.round%281%29%7Ds%22%0A%20%20res%0Aend%0A%0Ares%20%3D%20with_timer%20do%0A%20%20sleep%202%0A%20%20puts%20'hello%2C%20world!'%0A%20%20123%0Aend%0AT.reveal_type%28res%29)

The `type_parameters` method at the top-level of the `sig` block introduces
generic type variables that can be referenced elsewhere in the signature using
`T.type_parameter`. Names are specified as Ruby `Symbol` literals. Multiple
symbol literals can be given to `type_parameters`, like this:

```ruby
sig do
  type_parameters(:K, :V)
    .params(hash: T::Hash[T.type_parameter(:K), T.type_parameter(:V)])
    .returns([T::Array[T.type_parameter(:K)], T::Array[T.type_parameter(:V)]])
end
def keys_and_values(hash)
  [hash.keys, hash.values]
end
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20do%0A%20%20type_parameters%28%3AK%2C%20%3AV%29%0A%20%20%20%20.params%28hash%3A%20T%3A%3AHash%5BT.type_parameter%28%3AK%29%2C%20T.type_parameter%28%3AV%29%5D%29%0A%20%20%20%20.returns%28%5BT%3A%3AArray%5BT.type_parameter%28%3AK%29%5D%2C%20T%3A%3AArray%5BT.type_parameter%28%3AV%29%5D%5D%29%0Aend%0Adef%20keys_and_values%28hash%29%0A%20%20%5Bhash.keys%2C%20hash.values%5D%0Aend)

Note Sorbet does not support return type deduction. This means that doing
something like this won't work:

```ruby
sig do
  type_parameters(:U)
    .returns(T.type_parameter(:U))
end
def returns_something
  nil
end

x = returns_something
puts(x) # error: This code is unreachable
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20do%0A%20%20type_parameters%28%3AU%29%0A%20%20%20%20.returns%28T.type_parameter%28%3AU%29%29%0Aend%0Adef%20returns_something%0A%20%20nil%0Aend%0A%0Ax%20%3D%20returns_something%0Aputs%28x%29%20%23%20error%3A%20This%20code%20is%20unreachable)

In the above example, the `puts(x)` is listed as "unreachable" for a somewhat
confusing reason:

- Sorbet sees that the `T.type_parameter(:U)` in the `returns` annotation is not
  constrained by of the arguments. It could therefore be anything.
- The only type that is a subtype of any type in Sorbet is
  [`T.noreturn`](noreturn.md). The only way to introduce a value of this type is
  to raise an exception.
- Therefore, Sorbet infers that the only valid way to implement
  `returns_something` is by raising, which would imply that the `puts(x)` code
  is never reached.

### Placing bounds on generic methods

Sorbet does not have a way to place a bound on a generic method, but it's
usually possible to approximate it with
[intersection types (`T.all`)](intersection-types.md):

```ruby
class A
  extend T::Sig
  sig {returns(Integer)}
  def foo; 0; end
end

sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .void
end
def bad_example(x)
  x.foo # error!
end

sig do
  type_parameters(:U)
    .params(x: T.all(T.type_parameter(:U), A))
    .returns(T.type_parameter(:U))
end
def example(x)
  x.foo
  if x.foo.even? # calls to `.foo` and `.even?` are OK
    return x # this return is OK
  else
    return A.new # this return is not OK
  end
end
```

[→ View on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Aclass%20A%0A%20%20extend%20T%3A%3ASig%0A%20%20sig%20%7Breturns%28Integer%29%7D%0A%20%20def%20foo%3B%200%3B%20end%0Aend%0A%0Asig%20do%0A%20%20type_parameters%28%3AU%29%0A%20%20%20%20.params%28x%3A%20T.type_parameter%28%3AU%29%29%0A%20%20%20%20.void%0Aend%0Adef%20bad_example%28x%29%0A%20%20x.foo%0Aend%0A%0Asig%20do%0A%20%20type_parameters%28%3AU%29%0A%20%20%20%20.params%28x%3A%20T.all%28T.type_parameter%28%3AU%29%2C%20A%29%29%0A%20%20%20%20.returns%28T.type_parameter%28%3AU%29%29%0Aend%0Adef%20example%28x%29%0A%20%20x.foo%0A%20%20if%20x.foo.even%3F%0A%20%20%20%20return%20x%0A%20%20else%0A%20%20%20%20return%20A.new%0A%20%20end%0Aend)

There are a couple of things worth pointing out here:

- The `bad_example` method attempts to call `x.foo` but fails with an error. The
  error mentions that there is a call to method `foo` on an unconstrained
  generic type parameter. `T.type_parameter(:U)` alone means "for all types",
  but not all types have a `foo` method.

- In the `example` method, the method's signature changes to ascribe the type
  `T.all(T.type_parameter(:U), A)` to `x`. This in essence allows Sorbet to
  assume that there is an upper bound of `A` on the type of
  `T.type_parameter(:U)`.

- In the method body, the `T.all` is sufficient to allow the call to
  `x.foo.even?` to type check (and to have the type of `T::Boolean` statically).

- The first `return` in the method works without error: `x` has type
  `T.all(T.type_parameter(:U), A)` which is a subtype of `T.type_parameter(:U)`,
  so the `return x` type checks.

- The second return in the method fails to type check: `A.new` has type `A` but
  it does not have type `T.type_parameter(:U)`. This is **not** a bug. To see
  why, consider how Sorbet will typecheck a call site to `example`:

```ruby
class ChildA < A; end
child = example(ChildA.new)
T.reveal_type(child) # => `ChildA`
```

In the snippet above, Sorbet knows that the method returns
`T.type_parameter(:U)`, which is the same as whatever the type of `x` is, which
in this case is `ChildA`.

If Sorbet had allowed `return A.new` in the method body above, there would have
been a contradiction: Sorbet would have claimed that `child` had type `ChildA`,
but in fact it would have had type `A`, which is not a subtype of `ChildA`.

**tl;dr**: The only valid way to return something of type `T.type_parameter(:U)`
is to return one of the method's arguments (or some piece of an argument), not
by inventing an entirely new value.

### Shortcomings of generic methods

Most commonly, when there is something wrong with Sorbet's support for generic
methods, the error message mentions something about `<top>`, or something about
unreachable code. Whenever you see `<top>` in an error message, one of two
things is happening:

- There is a valid error, because the method's input type was not properly
  constrained. Double check the previous section on
  [placing bounds on generic methods](#placing-bounds-on-generic-methods).

- There is a bug or missing feature in Sorbet. Double check the list of issues
  in Sorbet's support for generics:

  [→ Issues with generics in Sorbet][generics milestone]

  If nothing in the list looks relevant to the particular behavior at hand,
  please report a new issue. Note that we have limited resources, and may not be
  able to prioritize fixing such issues.

When encountering an error like this, there are a couple of choices:

- Continue using generics, but use `T.unsafe` to silence the errors.

  Note that this can be **quite** burdensome: new programmers programming
  against the given API will be confused as to whether errors are their fault or
  Sorbet's.

- Refactor the API to use `T.untyped`. This has the benefit of having Sorbet
  stay out of people's way, letting them write the code they'd like to be able
  to write. It obviously comes at the cost of Sorbet not being able to provide
  strong guarantees about correctness.

- Find another way to type the API, potentially avoiding generics entirely. This
  might entail restructuring an API in a different way, using some sort of code
  generation, or something that merely doesn't trip the given bug. If you're
  stuck, ask for help.

[generics milestone]:
  https://github.com/sorbet/sorbet/issues?q=is%3Aopen+is%3Aissue+milestone%3AGenerics
[issues]: https://github.com/sorbet/sorbet/issues/new/choose
[variance]:
  https://en.wikipedia.org/wiki/Covariance_and_contravariance_(computer_science)
[covariant-ibox]:
  https://sorbet.run/#%23%20typed%3A%20strict%0A%0Amodule%20IBox%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AGeneric%0A%20%20abstract!%0A%0A%20%20%23%20Covariant%20type%20member%0A%20%20Elem%20%3D%20type_member%28%3Aout%29%0A%0A%20%20%23%20Elem%20can%20only%20be%20used%20in%20output%20position%0A%20%20sig%20%7Babstract.returns%28Elem%29%7D%0A%20%20def%20value%3B%20end%0Aend%0A%0Aclass%20Box%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AGeneric%0A%0A%20%20%23%20Implement%20the%20%60IBox%60%20interface%0A%20%20include%20IBox%0A%0A%20%20%23%20Redeclare%20the%20type%20member%2C%20to%20be%20compatible%20with%20%60IBox%60%0A%20%20Elem%20%3D%20type_member%0A%0A%20%20%23%20Within%20this%20class%2C%20%60Elem%60%20is%20invariant%2C%20so%20it%20can%20also%20be%20used%0A%20%20sig%20%7Bparams%28value%3A%20Elem%29.void%7D%0A%20%20def%20initialize%28value%3A%29%3B%20%40value%20%3D%20value%3B%20end%0A%0A%20%20%23%20Implement%20the%20%60value%60%20method%20from%20%60IBox%60%0A%20%20sig%20%7Boverride.returns%28Elem%29%7D%0A%20%20def%20value%3B%20%40value%3B%20end%0A%0A%20%20%23%20Add%20the%20ability%20to%20update%20the%20value%20%28allowed%0A%20%20%23%20because%20%60Elem%60%20is%20invariant%20within%20this%20class%29%0A%20%20sig%20%7Bparams%28value%3A%20Elem%29.returns%28Elem%29%7D%0A%20%20def%20value%3D%28value%29%3B%20%40value%20%3D%20value%3B%20end%0Aend%0A%0Aint_box%20%3D%20Box%5BInteger%5D.new%28value%3A%200%29%0A%0A%23%20not%20allowed%20%28attempts%20to%20widen%20type%2C%0A%23%20but%20%60Box%3A%3AElem%60%20is%20invariant%29%0Aint_or_str_box%20%3D%20T.let%28int_box%2C%20Box%5BT.any%28Integer%2C%20String%29%5D%29%0A%0A%23%20no%20error%20reported%20here%0Aint_or_str_box.value%20%3D%20''%0A%0AT.reveal_type%28int_box.value%29%0A%23%20Sorbet%20reveals%3A%20%60Integer%60%0A%23%20Actual%20type%20at%20runtime%3A%20%60String%60
[abstract_rpc_method]:
  https://sorbet.run/#%23%20typed%3A%20strict%0Aextend%20T%3A%3ASig%0A%0A%23%20---%20plain%20old%20data%20structures%20use%20for%20input%20and%20output%20types%20---%0Aclass%20Position%20%3C%20T%3A%3AStruct%0A%20%20const%20%3Aline%2C%20Integer%0A%20%20const%20%3Acharacter%2C%20Integer%0Aend%0A%0Aclass%20TextDocumentPositionParams%20%3C%20T%3A%3AStruct%0A%20%20const%20%3Atext_document%2C%20String%0A%20%20const%20%3Aposition%2C%20Position%0Aend%0A%0Aclass%20MarkupKind%20%3C%20T%3A%3AEnum%0A%20%20enums%20do%0A%20%20%20%20PlainText%20%3D%20new%28'plaintext'%29%0A%20%20%20%20Markdown%20%3D%20new%28'markdown'%29%0A%20%20end%0Aend%0A%0Aclass%20MarkupContent%20%3C%20T%3A%3AStruct%0A%20%20const%20%3Akind%2C%20MarkupKind%0A%20%20const%20%3Avalue%2C%20String%0Aend%0A%0Aclass%20HoverResponse%20%3C%20T%3A%3AStruct%0A%20%20const%20%3Acontents%2C%20MarkupContent%0Aend%0A%23%20----------------------------------------------------------------%0A%0Amodule%20AbstractRPCMethod%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AGeneric%0A%0A%20%20abstract!%0A%0A%20%20RPCInput%20%3D%20type_member%0A%20%20RPCOutput%20%3D%20type_member%0A%0A%20%20sig%20%7Babstract.returns%28String%29%7D%0A%20%20def%20method_name%3B%20end%0A%0A%20%20sig%20%7Babstract.params%28raw_input%3A%20T%3A%3AHash%5BT.untyped%2C%20T.untyped%5D%29.returns%28T.nilable%28RPCInput%29%29%7D%0A%20%20private%20def%20deserialize_impl%28raw_input%29%3B%20end%0A%0A%20%20sig%20%7Babstract.params%28input%3A%20RPCInput%29.returns%28T.nilable%28RPCOutput%29%29%7D%0A%20%20private%20def%20run_impl%28input%29%3B%20end%0A%0A%20%20sig%20do%0A%20%20%20%20params%28%0A%20%20%20%20%20%20raw_input%3A%20T%3A%3AHash%5BT.untyped%2C%20T.untyped%5D%0A%20%20%20%20%29%0A%20%20%20%20.returns%28T.nilable%28RPCOutput%29%29%0A%20%20end%0A%20%20def%20run%28raw_input%29%0A%20%20%20%20input%20%3D%20self.deserialize_impl%28raw_input%29%0A%20%20%20%20%23%20Could%20extend%20this%20example%20to%20use%20something%20richer%20for%20conveying%20an%20error%0A%20%20%20%20%23%20%28currently%20just%20returns%20nil%29%0A%20%20%20%20return%20unless%20input%0A%20%20%20%20run_impl%28input%29%0A%20%20end%0Aend%0A%0Aclass%20TextDocumentHoverMethod%0A%20%20extend%20T%3A%3ASig%0A%20%20extend%20T%3A%3AGeneric%0A%20%20extend%20AbstractRPCMethod%0A%20%20final!%0A%0A%20%20RPCInput%20%3D%20type_template%20%7B%7Bfixed%3A%20TextDocumentPositionParams%7D%7D%0A%20%20RPCOutput%20%3D%20type_template%20%7B%7Bfixed%3A%20HoverResponse%7D%7D%0A%0A%20%20sig%28%3Afinal%29%20%7Boverride.returns%28String%29%7D%0A%20%20def%20self.method_name%3B%20'textDocument%2Fhover'%3B%20end%0A%0A%20%20sig%28%3Afinal%29%20%7Boverride.params%28raw_input%3A%20T%3A%3AHash%5BT.untyped%2C%20T.untyped%5D%29.returns%28T.nilable%28RPCInput%29%29%7D%0A%20%20private_class_method%20def%20self.deserialize_impl%28raw_input%29%0A%20%20%20%20begin%0A%20%20%20%20%20%20TextDocumentPositionParams.from_hash%28raw_input%29%0A%20%20%20%20rescue%20TypeError%0A%20%20%20%20%20%20nil%0A%20%20%20%20end%0A%20%20end%0A%0A%20%20sig%28%3Afinal%29%20%7Boverride.params%28input%3A%20RPCInput%29.returns%28T.nilable%28RPCOutput%29%29%7D%0A%20%20private_class_method%20def%20self.run_impl%28input%29%0A%20%20%20%20puts%20%22Computing%20hover%20request%20at%20%23%7Binput.position%7D%22%0A%20%20%20%20raise%20%22TODO%22%0A%20%20end%0Aend%0A%0Asig%20%7Bparams%28raw_request%3A%20String%29.returns%28String%29%7D%0Adef%20handle_rpc_request%28raw_request%29%0A%20%20parsed_request%20%3D%20JSON.parse%28raw_request%29%0A%20%20params%20%3D%20parsed_request%5B'params'%5D%0A%0A%20%20output%20%3D%20case%20%28method%20%3D%20parsed_request%5B'method'%5D%29%0A%20%20when%20TextDocumentHoverMethod.method_name%0A%20%20%20%20TextDocumentHoverMethod.run%28params%29%0A%20%20else%0A%20%20%20%20raise%20%22Unknown%20method%3A%20%23%7Bmethod%7D%22%0A%20%20end%0A%0A%20%20if%20output%0A%20%20%20%20output.serialize%0A%20%20else%0A%20%20%20%20raise%20%22Error%20when%20running%20method%22%0A%20%20end%0Aend
