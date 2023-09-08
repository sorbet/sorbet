# Type member defaulting rules by variance

There are various places where Sorbet will take a bare generic class (one that
hasn't been applied to any type arguments) and apply it to "sensible" type
arguments.

## Why are bare generics bad?

Consider code like this:

```ruby
# typed: true
extend T::Sig

class Box
  extend T::Generic
  Elem = type_member

  sig {returns(T.nilable(Elem))}
  attr_accessor :val
end

sig {params(box: Box).void}
def example(box)
  val = box.val
  T.reveal_type(val) # ... ?
end
```

The `val` method mentions the `Elem` type member on `Box`. When we call this
method on a type like `Box[Integer]`, Sorbet effectively
substitutes[^substitute] `Integer` for `Elem` in the signature for `Box#val`.

[^substitute]: Sorbet doesn't actually use the term substitute here, it uses `alignBaseTypeArgs`, because OOP and inheritance are messy. But if your intuition is substitution, that's close enough for now.

If we allowed calling `box.val` when `box` had type `ClassType { klass = ::Box
}`, how would that substitution work? It wouldn't, because there are no type
arguments to apply.

You would think it nonsensical to try to treat this program as well-formed:

```ruby
def requires_one_arg(x)
  puts(x)
end

requires_one_arg() # 💥
```

And essentially, generic classes are type-level functions that take required,
type-level arguments.

As such, Sorbet tries as hard as possible to require type arguments for generic
classes. But the real world is messy.


## Why even have defaults?

Having defaults for bare generic classes is _mostly_ a hack.

In almost all cases, we would probably prefer the user to give us arguments. We
don't want to see `Array`, we want `T::Array[SomeType]`. But there are some
complications we have to accomodate.

- Pre-Sorbet type annotations.

  The runtime type systems in use at Stripe that predated Sorbet allowed writing
  `Array`, `Hash`, etc. as valid type annotations. We didn't want to codemod all
  these when we rolled out Sorbet to minimize disruption.

  This persists to this day: generic classes defined in the standard library are
  allowed to be bare in `# typed: true`. Only in `# typed: strict` does writing
  `Array` in type syntax produce an error.

- Types for class literals (or more generally, any class with type templates).

  All class singleton classes are generic classes (generic in the
  `<AttachedClass>` type template that Sorbet implicitly declares). Without
  defaulting rules, the type of an expression like `MyClass` would be
  `T.class_of(MyClass)`. This is a bare generic, because type templates are just
  [type members scoped to a singleton class].

  No matter how hard we try, it would be impossible to ask users to replace
  something like `MyClass.foo` with something like
  `T.class_of(MyClass)[MyClass].foo` (even assuming this syntax were valid).
  Indeed, most of our choices around type member defaulting rules were invented
  to accommodate `<AttachedClass>`. But the rules were chosen from first
  principles, not implementing something ad hoc for `<AttachedClass>`
  specifically.

- Instantiating a generic class and forgetting to provide type arguments.

  This is _somewhat_ a repeat/special case of the previous, but worth mentioning
  anyways. Consider code like this:

  ```ruby
  Array.new
  Box.new
  ```

  Both of these are problematic. We'd prefer that users write something like
  `Array[Integer].new` or `Box[String].new`. But because of how Sorbet builds
  the CFG, both these look like

  ```ruby
  <tmp>$1 = Array
  <tmp>$1.new()

  <tmp>$2 = Box
  <tmp>$2.new()
  ```

  Sorbet has to make a choice for the type of `<tmp>$1` and `<tmp>$2` when
  they're assigned, which makes it hard for Sorbet to reject the call to `.new`
  saying something like "you have to apply this class to a type when
  instantiating it."

  Unlike the previous case, `Box[Integer].new` is at least syntax we support,
  because it's applying the arguments to the instance class. And even then, we
  only support this syntax for the `.new` method, not for other singleton class
  methods: `Box[Integer].make` isn't valid Sorbet syntax, because Sorbet
  special-cases constructor methods.[^explicit-application]

- Types for the `.class` method.

  Consider code like this

  ```ruby
  class MyModel
    extend T::Generic
    MutatorType = type_template { {upper: MyMutator} }

    def foo
      klass = self.class
    end
  end
  ```

  What should the type of `klass` be in the snippet above? The type can't simply
  be `T.class_of(MyModel)`, because that would be a bare generic. This is a very
  similar problem to the class literal problem, where it would be prohibitively
  annoying to ask users to rewrite code like `self.class`, and we don't even
  have a suitable replacement if we wanted to enforce something.

  Even more, there's a similar problem to the previous point:

  ```ruby
  my_box.class.new('')
  ```

  If `my_box` is `Box[Integer]`, there's no reason why `my_box.class.new` has to
  create another `Box[Integer]`--making a `Box[String]` is fine!

  But in Sorbet's CFG, that snippet looks like this:

  ```ruby
  <tmp>$1 = my_box.class
  <tmp>$2 = <tmp>$1.new('')
  ```

  and Sorbet has no way of knowing whether `<tmp>$1` was a constant literal or
  an expression. (Wildly, Sorbet allows `my_box.class[String].new('')` to make a
  well-typed `Box[String]` instance).

**TL;DR**: It's unavoidable for Sorbet to have to allow bare generics in places.

However, completely bare generics are bad, because then we have no types to
substitute for generic types when used in method signatures, etc.

So Sorbet has default rules to silently convert bare generic classes to applied
generic classes in places where it's unavoidable.

## The defaulting rules

So we've established that bare generic classes are bad, and that bare generic
classes are also unavoidable. Defaulting rules are how we make the best of a bad
situation.

The defaulting rules are implemented by `ClassOrModule::externalType()`, which
is computed once per Symbol during resolver and cached, so the actual
computation happens in [`unsafeComputeExternalType`]. (It's not actually unsafe
anymore because of a change jvilk made in [this commit][175ce0f1]. It used to do
lurky things with atomics and racy mutable access).

The rules here might be out of date w.r.t. the implementation, and if they are
please edit this doc. But at a high level:

- If a type_member is a "legacy" generic, which is basically `Array` and a few
  other classes (but importantly, **not** every stdlib generic class), it
  unsafely defaults to `T.untyped` no matter what. (This was just a hack to get
  things to type check when rolling out Sorbet.)

- If a type member is `fixed`, default it to whatever the type member is fixed
  to, regardless of variance.

- Otherwise, we're going to have to use the variance:

  - Invariant? -> `T.untyped`
  - Covariant? -> the `upper` bound (the default upper bound is `T.anything`)
  - Contravariant? -> the `lower` bound (the default lower bound is
    `T.noreturn`)

The most common question is then: why are invariant type members defaulted to
`T.untyped`? It's because we can't get away with using `upper` nor `lower`.

Consider this setup:

```ruby
class Parent; end
class Child < Parent; end
```

In this setup, `T.class_of(Child) <: T.class_of(Parent)`, which is what users
expect. Now consider if we made `Parent` and `Child` generic:

```ruby
class Parent
  extend T::Generic
  Elem = type_template { {upper: Numeric} }
end
class Child < Parent
  extend T::Generic
  Elem = type_template { {upper: Integer} }
end
```

Suddenly, `Parent` and `Child` need to have defaults (because of all the
situations we had before, like if they appear as class literals in a method
body, or as the result type of a call to `self.class`). We might want to pick
defaults such that we maintain the relationship that held previously, namely that
`T.class_of(Child) <: T.class_of(Parent)`. (Those types are meaningless now that
those are bare generic classes, but the intuition of wanting it to still hold
remains). The problem is that both `upper` and `lower` are hard to stomach as
defaults.

Consider if we pick `upper`. Then we'd have that:

- `Parent` has type `T.class_of(Parent)[Numeric]` and
- `Child` has type `T.class_of(Child)[Integer]`

`T.class_of(Child)[Integer]` is not a subtype of `T.class_of(Parent)[Numeric]`
because `Elem` is invariant! As unintuitive as this sounds, this subtyping
outcome is actually the desirable, sound outcome: if we both pick the upper
bound as the default **and** treat `Child` as a valid value of type
`T.class_of(Parent)`, it's possible to draw a contradiction:

```ruby
# typed: strong
class Module; include T::Sig; end

class Parent
  extend T::Generic
  Elem = type_template { {upper: Numeric} }

  sig {overridable.params(x: Elem).void}
  def self.example(x); end
end

class Child < Parent
  extend T::Generic
  Elem = type_template { {upper: Integer} }

  sig {params(x: Integer).void}
  def self.takes_integer(x); end

  sig {override.params(x: Elem).void}
  def self.example(x)
    takes_integer(x)
    # ^ our program passes a float to takes_integer without using T.unsafe,
    # and with no errors
  end
end

sig {params(parent_class: T.class_of(Parent)).void}
#           ^ defaults to T.class_of(Parent)[Numeric]
def main(parent_class)
  parent_class.example(0.0)
  #                    ^^^ a valid numeric, but not an Integer!
end

main(Child)
```

So maybe `upper` was the wrong thing to pick? But if we pick `lower`, two things
happen:

- `T.class_of(Parent)` defaults to `T.class_of(Parent)[T.noreturn]`,
  which means we can't even call `parent_class.example`
- We could make a similar contradiction by rewriting the example to use `Elem`
  in `returns` instead of `params`.

So maybe we pick `upper` when defaulting in `params`, and `lower` when
defaulting in `returns`? Implementation complexity aside (we'd have to thread
input/output position (polarity) through all calls to `externalType`, which would
mean we couldn't cache it as easily as now), we'd still have the problem that
most users expect `Child` to be a valid `T.class_of(Parent)`.

So Sorbet gives up and accepts the unsoundness by simply defaulting to
`T.untyped`.

## Be super careful with unfixed, invariant type members

The takeaway? Type members in well-typed code should always either:

- be fixed, and accept the limitations that come with that (basically: makes the
  class final in practice[^final]), or
- be either co- or contravariant
- be carefully reviewed to ensure that `T.untyped` isn't sneaking in
  ([highlighting untyped code](https://sorbet.org/docs/highlight-untyped) is a
  good idea, maybe even using `# typed: strong` if possible).

Code making heavy use of invariant, non-fixed generics will be the most prone to
having `T.untyped` sneak in, due to a combination of unavoidable and intentional
choices in the type system. Mitigating `T.untyped` means looking at the list of
cases where Sorbet defaults type arguments, and doing the opposite.

- Use `# typed: strict`, so Sorbet will treat bare stdlib generics as errors.

  For generic singleton classes, note that `T.class_of(...)[...]` is valid
  syntax for applying type arguments to a generic singleton class. (Sorbet does
  not yet require this syntax in `# typed: strict` files, because the syntax is
  so new.)

- In a class with invariant type templates, avoid calling singleton class
  methods on class literals. Instead, only call them inside variables on a
  generic class:

  ```ruby
  class AbstractClass
    MyTemplate = type_template

    sig {abstract.returns(MyTemplate)}
    def self.thing; end

    def self.example
      self.thing # OK
    end
  end

  sig do
    type_parameters(:U)
      .params(abstract_class: T.class_of(AbstractClass)[AbstractClass,
      T.type_parameter(:U)])
      .returns(T.type_parameter(:U))
  end
  def example(abstract_class)
    abstract_class.thing # OK
  end

  AbstractClass.thing # 💥 bad, untyped
  ```

- Always provide types when instantiating a generic class.

  ```ruby
  Box.new(0) # 💥 bad, untyped
  Box[Integer].new(0) # OK
  ```

  Ideally Sorbet could catch this in the future.

- Understand that using the `.class` method will introduce untyped the same way
  as mentioning a class literal.

  Ideally Sorbet could catch this in the future.





[type members scoped to a singleton class]: https://sorbet.org/docs/generics#type_member--type_template

[^explicit-application]: This is another weird case. Most other languages would
treat the constructor as a generic **method**, where the type is something like
[T](x: T) -> Box[T], and would then have syntax for applying a type argument to
generic _methods_, not the class, like `Box.new[Integer](0)` or
`Box.make[Integer](0)`.

[^final]: I say "final in practice" here because while you'll still be able to
subclass this class, those subclasses will be forced to repeat the exact same
`fixed` annotation as the parent. This means that if, for example, you're using
a type member to track an associated `Result` type of a class that implements an
AbstractRPCCommand (like the [example in our generics docs]), you'll find that
child classes must have the same `Result` type as the parent. In rare cases
that's fine, but usually when you're making a subclass, you want to use set the
type member to something else entirely, which is no longer possible after a
bound is `fixed`.

[`unsafeComputeExternalType`]: https://github.com/sorbet/sorbet/blob/f47b4a5c61130bbd97695d4efa4287b0cd84a1f6/core/Symbols.cc#L128

[example in our generics docs]: https://sorbet.org/docs/generics#a-type_template-example

[175ce0f1]: https://github.com/sorbet/sorbet/commits/175ce0f1b7c910cc035aeccf2aec5907866178bb
