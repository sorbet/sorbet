---
id: error-reference
title: Sorbet Error Reference
sidebar_label: Error Reference
---

<style>
#missing-doc-for-error-code-box.is-hidden {
  display: none;
}
</style>

<div class="is-hidden red" id="missing-doc-for-error-code-box">

<a class="anchor" aria-hidden="true" id="missing-doc-for-error-code-scroll"></a>

> **Heads up**: There aren't any docs yet for <span id="missing-error-code">this
> error code</span>. If you have suggestions for what would have helped you
> solve this problem, click the "Edit" button above to contribute! Otherwise,
> try using the search above to find an answer.

</div>

> **Note**: This list is not exhaustive! Some errors are very context dependent
> and other error codes are not common enough to know how to generally suggest
> help. Contributions to this list are welcome!

This is one of three docs aimed at helping answer common questions about Sorbet:

1.  [Troubleshooting](troubleshooting.md)
1.  [Frequently Asked Questions](faq.md)
1.  [Sorbet Error Reference](error-reference.md) (this doc)

This page contains tips and tricks for common errors from `srb`.

## 1001

Sorbet has crashed. Please [report an issue]!

## 1004

Sorbet couldn't find a file.

## 2001

There was a Ruby syntax error. Sorbet was unable to parse the source code. If
you encounter this error but your code is accepted by Ruby itself, this is a bug
in our parser; please [report an issue] to us so we can address it.

The only intentional break with Ruby compatibility is that method names that are
keywords have some limitations with multi-line code, as explained in [#1993],
and should not be reported.

[#1993]: https://github.com/sorbet/sorbet/pull/1993

## 2003

If you're seeing this error code, it means that Sorbet already emitted a parse
error for the file, but found some extra information that might help point out
the root cause of a syntax error. For example:

```ruby
class A
  def foo
    if x
  end
end
```

This Ruby snippet does not parse, but the reason why is confusing. Because Ruby
does not care about indentation, it will try to consune `end` keywords eagerly
if there is something available to match. In this example, the first `end`
matches with `if` and the second matches with `def` and so Sorbet will report
`unexpected token "end of file"` because the `class A` definition was not
matched with an `end` token.

But given the indentation structure present in the original program, it's more
likely that the `if x` statement is unclosed. Thus, in some cases, Sorbet will
provide extra "Hint:" diagnostics that point out things that might be the root
cause.

It's important to note that **these hints are imperfect**—fixing them might not
actually fix the real parse error. Instead, they're provided as a way to help
recover from the real parse error.

As with all Sorbet error messages, if one of these hint error messages is
confusing or misleading, please [report an issue] to let us know.

## 3001

Sorbet doesn’t support singleton definitions outside of the class itself:

```rb
class << MyClass # error: `class << EXPRESSION` is only supported for `class << self`
  def foo
    # ...
  end
end
```

The workaround is to move the definition inside the `MyClass` class itself:

```rb
class MyClass
  class << self
    def foo
      # ...
    end
  end
end
```

Sometimes, `EXPRESSION` is not a constant literal like in what follows:

```rb
class << some_variable
  include Foo
end
```

In this case, it is possible to directly call `include` on the the singleton
class of `EXPRESSION`, but it should be done with **utmost caution**, as Sorbet
will not consider the include and provide a less accurate analysis (see also
[#4002](#4002)):

```rb
some_variable.singleton_class.include(Foo)
```

## 3002

Sorbet is limited to C++ `INT_MAX`:

```rb
puts 11377327221391349843 + 1 # error: Unsupported integer literal: 11377327221391349843
```

A possible workaround is to use a string and `to_i`:

```rb
puts "11377327221391349843".to_i + 1
```

## 3011

There was a Hash literal with duplicated keys.

```ruby
{my_key: 1, my_key: 2} # error: `my_key` is duplicated
```

This error can also be caused when trying to write a `sig` for a method with
duplicated parameter names:

```ruby
sig {params(_: String, _: Integer).void} # error: `_` is duplicated
def foo(_, _); end
```

To write a `sig` for this method, rename the method's parameters to have unique
names:

```ruby
sig {params(_a: String, _b: Integer).void} # ok
def foo(_a, _b); end
```

## 4002

Sorbet requires that every `include` references a constant literal. For example,
this is an error, even in `# typed: false` files:

```ruby
module A; end
module B; end

def x
  rand.round == 0 ? A : B
end

class C
  include x  # error: `include` must be passed a constant literal
end
```

Non-constant literals make it hard to impossible to determine the complete
inheritance hierarchy in a codebase. Sorbet must know the complete inheritance
hierarchy of a codebase in order to check that a variable is a valid instance of
a type.

It is possible to silence this error with `T.unsafe`, but it should be done with
**utmost caution**, as Sorbet will not consider the include and provide a less
accurate analysis:

```ruby
module A; end
module B; end

def x
  rand.round == 0 ? A : B
end

class C
  T.unsafe(self).include x
end
```

Which might create unexpected errors:

```ruby
c = C.new

c.a # error: Method `a` does not exist on `C`
c.b # error: Method `b` does not exist on `C`

T.let(C, A) # error: Argument does not have asserted type `A`
T.let(C, B) # error: Argument does not have asserted type `B`
```

## 4010

There are multiple definitions for a method. Having multiple definitions for a
method is problematic, because it can make a codebase's behavior dependent on
the order in which files are loaded.

The only way to silence this error currently is to mark the offending file as
`# typed: false`.

## 4011

There are multiple definitions for the same type member within a given class or
module.

```ruby
class Main
  extend T::Generic

  Elem = type_member
  Elem = type_member # error: Duplicate type member
end
```

You can fix this by removing the second definition of the type member:

```ruby
class Main
  extend T::Generic

  Elem = type_member # ok
end
```

## 4012

A `class` was redefined as a `module` or _vice versa_ in two separate locations.

```ruby
# file_a.rb
class Foo
end

# file_b.rb
module Foo
end
```

<a href="https://sorbet.run/#%23%20typed%3A%20true%0A%0A%23%20file_a.rb%0Aclass%20Foo%0Aend%0A%0A%23%20file_b.rb%0Amodule%20Foo%0Aend">→
View on sorbet.run</a>

You can fix this error by ensuring that both definitions are declared as
`class`es, ensuring both definitions are declared as `module`s, or renaming
either definition so they no longer conflict.

## 4015

This error usually comes when a class or module is dynamically defined and
stored into a constant, like this:

```ruby
A = ...
A::B = 1
```

where `...` is some expression which computes a class or module. Sorbet can't
know statically what this `...` code does (and for example even if could assume
that it's defining a class, Sorbet can't know what methods or constants it has).
Therefore, Sorbet does not support this pattern.

## 5001

Sorbet cannot resolve references to dynamic constants. The common case occurs
when a constant is dynamically referenced through the singleton class of `self`:

```rb
class MyCachable < Cachable
  CACHE_KEY_PREFIX = "my_cachable_"

  def cache_key
    self.class::CACHE_KEY_PREFIX + identifier
  end
end
```

This code can by made statically analysable by using a singleton method to
reference the constant:

```rb
class MyCachable < Cachable
  def cache_key
    self.class.cache_key_prefix + identifier
  end

  class << self
    def cache_key_prefix
      "my_cachable_"
    end
  end
end
```

## 5002

This means that the typechecker has been unable to resolve a reference to a
constant (e.g., a Ruby class). Most commonly, this indicates that there's a
typo.

First, try confirming whether the code runs successfully. Does the code raise an
"uninitialized constant" error when run? If so, Sorbet caught a bug! Try finding
out why that constant is actually uninitialized.

If it isn't a typo, then there are a few other things to look at. If it looks
like the constant is related to a gem, maybe one of these helps:

- Is it coming from a gem? Sorbet does not look through the gem's source code.
  Instead, there must be an `*.rbi` file for this gem. Try finding the `*.rbi`
  corresponding to this gem, and searching through it for the constant.

  For more information, see [RBI files](rbi.md). If you are at Stripe, please
  instead see <http://go/types/rbi>.

- If the gem was recently updated, its `*.rbi` might need to be regenerated.
  Each RBI file has a line at the top which can be copy / pasted to re-generate
  the file when the underlying gem has changed.

- When deleting constants, sometimes they are still referenced from an
  autogenerated `*.rbi` file. If that's the case, consider deleting the constant
  or regenerating the file.

Another thing it could be: Sorbet explicitly does not support resolving
constants through ancestors (both mixins or superclasses).

Concretely, here's an example of code rejected and accepted by Sorbet:

```ruby
class Parent
  MY_CONST = 91
end

class Child < Parent; end

Child::MY_CONST    # error
Parent::MY_CONST   # ok
```

Alternatively, if it's much more preferable to access the constant on the child,
we can set up an explicit alias:

```ruby
class Parent
  MY_CONST = 91
end

class Child < Parent
  MY_CONST = Parent::MY_CONST
end

Child::MY_CONST    # ok
Parent::MY_CONST   # ok
```

## 5005

A class or instance variable is defined in the wrong context.

```ruby
# typed: true
class A
  extend T::Sig

  def foo
    @@class_var = T.let(10, Integer)
    @x = T.let(10, Integer)
  end

end
```

There are two such errors in the above. In the first, `@@class_var` is declared
outside of the class scope. In the second, `@x` is declared outside of the
`initialize` method.

For how to fix, see [Type Annotations](type-annotations.md).

## 5006

An instance variable has been redeclared with another type.

```ruby
# typed: true
class A
  extend T::Sig

  def initialize
    @x = T.let(10, Integer)
    @x = T.let("x", String)
  end
end
```

## 5008

A class was defined as the subclass of a `type_alias`. It also occurs if a
`type_alias` mixin is used in a class.

```ruby
# typed: true
A = T.type_alias {Integer}
class B < A; end # error: Superclasses and mixins may not be type aliases

module M; end

AliasModule = T.type_alias {M}
class C
  include AliasModule # error: Superclasses and mixins may not be type aliases
end
```

## 5011

A class inherits from itself either directly or through an inheritance chain.

```ruby
class A < A; end

class B < C; end
class C < B; end
```

## 5012

A class was changed to inherit from a different superclass.

```ruby
class A; end
class B; end

class C < A; end
class C < B; end
```

## 5013

A class or instance variable declaration used `T.cast` when it should use
`T.let`.

```ruby
class A
  @@x = T.cast(10, Integer)
end
```

For how to fix, see [Type Annotations](type-annotations.md).

To instead use `T.cast` as a runtime-only type check (that is, neither as a
statically-checked assertion nor as an instance variable declaration), assign
the cast result to an intermediate variable:

```ruby
class A
  x = T.cast(10, Integer)
  @@x = x
end
```

## 5014

Given code like this:

```ruby
# typed: true
class Parent
  extend T::Generic
  Foo = type_member
end

class Child < Parent
  extend T::Generic
end
```

We need to change our code to redeclare the type member in the child class too:

```ruby
# typed: true
class Parent
  extend T::Generic
  Foo = type_member
end

class Child < Parent
  extend T::Generic
  Foo = type_member
end
```

The same thing holds for type templates.

## 5023

Some modules require specific functionality in the receiving class to work. For
example `Enumerable` needs a `each` method in the target class.

Failing example in
[sorbet.run](https://sorbet.run/#class%20Example%0A%20%20include%20Enumerable%0Aend):

```
class Example
  include Enumerable
end
```

To fix this, implement the required abstract methods in your class to provide
the required functionality.

Passing example in
[sorbet.run](<https://sorbet.run/#class%20Example%0A%20%20include%20Enumerable%0A%0A%20%20def%20each(%26blk)%0A%0A%20%20end%0Aend>):

```
class Example
  include Enumerable

  def each(&blk)
  end
end
```

## 5028

In `# typed: strict` files, Sorbet requires that all constants are annotated
with a `T.let`.

For how to fix, see [Type Annotations](type-annotations.md).

See also: [6002](#6002), [7017](#7017).

## 5034

Sorbet does not support creating normal Ruby constant aliases to type aliases.
Once a type alias is created, all subsequent aliases must also be type aliases.

Concretely, this is not allowed:

```ruby
A = T.type_alias {Integer}
B = A # error: Reassigning a type alias is not allowed
```

while this is:

```ruby
A = T.type_alias {Integer}
B = T.type_alias {A}
```

(Why? This is due to design tradeoffs to enforce stronger internal invariants.
Basically, Sorbet can emit more reliable warnings when users declare their
intent to create a new type alias.)

## 5035

A method was marked `override`, but sorbet was unable to find a method in the
class's ancestors that would be overridden. Ensure that the method being
overridden exists in the ancestors of the class defining the `override` method,
or remove `override` from the signature that's raising the error. See
[Override Checking](override-checking) for more information about `override`.

If the parent method definitely exists at runtime, it might be hidden in a
[`# typed: ignore`](static#file-level-granularity-strictness-levels) file.
Sorbet will not see it and this error will be raised. In that case you will need
to either raise the `typed` sigil of that file above `ignore`, or generate an
[RBI file](rbi) that contains signatures for the classes and methods that file
defines.

## 5036

See [5014](#5014). 5036 is the same error as [5014](#5014) but slightly modified
to allow more common Ruby idioms to pass by in `# typed: true` (5036 is only
reported in `# typed: strict`).

## 5037

Sorbet must be able to statically resolve a method to create an alias to it.

Here, the method is created through a DSL called `data_accessor` which defines
methods at runtime through meta-programming:

```rb
class Base
  def self.data_accessor(key)
    define_method(key) do
      data[key]
    end
  end

  # ...
end

class Foo < Base
  data_accessor :foo

  alias_method :bar, :foo # error: Can't make method alias from `bar` to non existing method `foo`
end
```

One way to make those methods visible statically is to add a declaration for
them in an [RBI file](https://sorbet.org/docs/rbi). For example, we can write
our definitions as RBI under `sorbet/rbi/shims/foo.rbi`:

```rb
# sorbet/rbi/shims/foo.rbi
# typed: true

module Foo
  def foo; end
end
```

Sometimes, Sorbet will complain about an alias to a method coming from an
included modules. For example, here `bar` is coming from the inclusion of `Bar`
but Sorbet will complain about the method not existing anyway:

```rb
module Bar
  def bar; end
end

class Foo
  include Bar

  alias_method :foo, :bar # error: Can't make method alias from `foo` to non existing method `bar`
end
```

It's because Sorbet resolves method aliases before it resolves includes. You can
see an example of this behaviour
[here](https://sorbet.run/#%23%20typed%3A%20true%0A%0Amodule%20Bar%0A%20%20def%20bar%3B%20end%0Aend%0A%0Amodule%20Foo%0A%20%20include%20Bar%0A%0A%20%20alias_method%20%3Afoo%2C%20%3Abar%20%23%20aliases%20are%20resolved%20before%20includes%2C%20so%20%60bar%60%20is%20not%20found%20yet%0A%0A%20%20def%20baz%0A%20%20%20%20bar%20%23%20includes%20are%20resolved%20when%20we%20analyze%20this%20code%0A%20%20end%0Aend).
To workaround this limitation, we can replace the `alias_method` by a real
method definition:

```rb
class Foo
  include Bar

  def foo
    bar
  end
end
```

## 5041

Sorbet does not allow inheriting from a class which inherits from `T::Struct`.

```ruby
class S < T::Struct
  prop :foo, Integer
end

class Bad < S; end # error
```

This limitation exists because, in order to generate a static type for
`initialize` for a struct, we need to know all of the `prop`s that are declared
on this struct. By disallowing inheritance of structs, we can know that all of
the props declared on this struct were syntactically present in the class body.

One common situation where inheritance may be desired is when a parent struct
declares some common props, and children structs declare their own props.

```ruby
class Parent < T::Struct
  prop :foo, Integer
end

class ChildOne < Parent # error
  prop :bar, String
end

class ChildTwo < Parent # error
  prop :quz, Symbol
end
```

We can restructure the code to use composition instead of inheritance.

```ruby
class Common < T::Struct
  prop :foo, Integer
end

class ChildOne < T::Struct
  prop :common, Common
  prop :bar, String
end

class ChildTwo < T::Struct
  prop :common, Common
  prop :quz, Symbol
end
```

## 5046

Generic classes must be passed all their generic type arguments when being used
as types. For example:

```ruby
T.let([], Array)              # error
T.let([], T::Array[Integer])  # ok
```

Many classes in the standard library are generic classes
([see here](stdlib-generics.md)), and must be passed type arguments, including
`Array` and `Hash`. Any user-defined generic classes must similarly be provided
type arguments when used.

For legacy reasons relating to the intial rollout of Sorbet, this error is only
reported at `# typed: strict` for standard library generic classes and
`# typed: true` for all user-defined generic classes. (In an ideal world, it
would have always been reported at `# typed: true`, and we might change this in
the future.)

## 5047

A class or module tried to inherit, include, or extend a final class or module.

```ruby
class Final
  extend T::Helpers
  final!
end

class Bad < Final; end # error
```

## 5048

A class or module was declared as final, but a method in the class or module was
not explicitly declared as final with a final `sig`.

```ruby
class C
  extend T::Helpers
  final!

  def no_sig; end # error

  extend T::Sig

  sig {void}
  def non_final_sig; end # error

  sig(:final) {void}
  def final_sig; end # good
end
```

## 5054

Use of `implementation` has been replaced by `override`.

## 5056

The `generated` annotation in method signatures is deprecated.

For alternatives, see [Enabling Runtime Checks](runtime.md) which talks about
how to change the runtime behavior when method signatures encounter a problem.

## 5057

Static methods (like `self.foo`) can never be mixed into another class or
module. Both `include` and `extend` only mix that module's _instance_ methods
onto the target class or module. Classes can inherit static methods from their
superclass, but only classes (not modules) can be superclasses.

Thus, a static, abstract method on a module is impossible to implement, and thus
is a no-op.

```ruby
module MyMixin
  sig {abstract.void}
  def foo; end

  sig {abstract.void}
  def self.bar; end # error: Static methods in a module cannot be abstract
end
```

Some alternatives:

- Use `mixes_in_class_methods`, which declares to Sorbet that when a module is
  included, some other module should be extended into the target class.
  [Full documentation here](abstract.md). This is the preferred option.

- Separate the interface into two modules. Include one and extend the other in
  all the places where

- Change the abstract module to an abstract class, and update all downstream
  references to inherit from this class instead of including the original
  module.

## 5058

It's an error to use `T.attached_class` to describe the type of method
parameters. See the
[T.attached_class](attached-class.md#tattached_class-as-an-argument)
documentation for a more thorough description of why this is.

## 5064

Using the `requires_ancestor` method, module `Bar` has indicated to Sorbet that
it can only work properly if it is explicitly included along module `Foo`. In
this example, we see that while module `Bar` is included in `MyClass`, `MyClass`
does not include `Foo`.

```rb
module Foo
  def foo; end
end

module Bar
  extend T::Helpers

  requires_ancestor { Foo }

  def bar
    foo
  end
end

class MyClass # error: `MyClass` must include `Foo` (required by `Bar`)
  include Bar
end
```

The solution is to include `Foo` in `MyClass`:

```rb
class MyClass
  include Foo
  include Bar
end
```

Other potential (albeit less common) sources of this error code are classes that
are required to have some class as an ancestor:

```
class Foo
  def foo; end
end

module Bar
  extend T::Helpers

  requires_ancestor { Foo }

  def bar
    foo
  end
end

class MySuperClass
  extend T::Helpers
  include Bar

  abstract!
end

class MyClass < MySuperClass # error: `MyClass` must inherit `Foo` (required by `Bar`)
end
```

Ensuring `MyClass` inherits from `Foo` at some point will fix the error:

```rb
class MySuperClass < Foo
  extend T::Helpers
  include Bar

  abstract!
end

class MyClass < MySuperClass
end
```

## 5068

Sorbet requires that class or module definitions be namespaced unambiguously.
For example, in this code:

```
# typed: true

module B
end

module A
  class B::C
   # ...
  end
end
```

The definition B::C is ambiguous. In Ruby's runtime, it resolves to B::C (and
not A::B::C). However, things are different in the presence of a pre-declared
filler namespace like below:

```
# typed: true

module B
end

module A
  module B; end

  class B::C
   # ...
  end
end
```

In this case, the definition resolves to A::B::C in Ruby's runtime.

By default, Sorbet assumes the presence of filler namespaces while typechecking,
regardless of whether they are explicitly predeclared like in the second
example. This means that in Sorbet's view, the definition resolves to A::B::C in
either case.

In Stripe's codebase, this is generally not a problem at runtime, as we use
Sorbet's own autoloader generation to pre-declare filler namespaces, keeping the
Ruby runtime's behavior equivalent to Sorbet. However, the autoloader has some
edge cases, which can often cause deviations between Ruby's runtime and Sorbet.
This error helps guard against these issues.

## 6002

In `# typed: strict` files, Sorbet requires that all instance and class
variables are annotated with a `T.let`.

For how to fix, see [Type Annotations](type-annotations.md).

See also: [5028](#5028), [7017](#7017).

## 7001

Sorbet does not allow reassigning a variable to a different type within a loop
or block. (Note that we model blocks similarly to loops, as in general they may
execute 0, 1, or more times). Due to implementation constraints, Sorbet does not
permit this behavior.

A prototypical example of code that might trigger this is code that sets a
variable to `nil`, and then updates it if some value is found inside a loop:

```ruby
found = nil

list.each do |elem|
  found = elem if want?(elem)
end
```

In most cases, we can fix this error by declaring the type of the loop variable
outside the loop using `T.let`:

```ruby
# This is a type annotation that explicitly widens the type:
found = T.let(nil, T.nilable(String))

list.each do |elem|
  found = elem if want?(elem)
end
```

**But my variable does not change its type, it is always a `Boolean`!**

In Ruby, there is no `Boolean` type. Instead, there are `FalseClass` and
`TrueClass` types, the union of which defines
[`T::Boolean` type as a union type](class-types.md#booleans).

When Sorbet encounters a variable declaration like `x = true`, it infers the
type of `x` as `TrueClass`. An assignment to `x` later on in the same block such
as `x = false` would imply that the variable is reassigned to a different type
(namely, to `FalseClass` in this case).

For this reason, a loop such as the following triggers an error:

```ruby
# Declares `found_valid` with type `FalseClass`
found_valid = false

list.each do |elem|
  # Might change the type of `found_valid` to `TrueClass`
  found_valid = true if valid?(elem) # error: Changing the type of a variable in a loop
end
```

The fix, again, is to use `T.let` to widen the type to `T::Boolean`:

```ruby
# Declare `found_valid` with type `T::Boolean`
found_valid = T.let(false, T::Boolean)

list.each do |elem|
  # Does not change the type of `found_valid`
  found_valid = true if valid?(elem) # ok
end
```

## 7002

This is a standard type mismatch. A method's `sig` declares one type, but the
actual value didn't match. For example:

```ruby
'str' + :sym  # error: Expected `String` but found `Symbol(:"sym")` for argument `arg0`
```

Even still, sometimes these errors can be rather confusing. Consider using
[`T.reveal_type`](troubleshooting.md) to pin down the origin of why Sorbet
thinks the types are what it says.

**Why does Sorbet think this is `nil`? I just checked that it's not!**

That's a [great question](flow-sensitive#limitations-of-flow-sensitivity), and
probably the most common question people have when using Sorbet!

It's answered here:
[Limitations of flow-sensitivity](flow-sensitive#limitations-of-flow-sensitivity)

## 7003

This error indicates a call to a method we believe does not exist (a la Ruby's
`NoMethodError` exception). Some steps to debug:

1.  Double check that the code actually runs, either in the REPL, in CI, or with
    manual tests. If the method doesn't actually exist when run, Sorbet caught a
    bug!

1.  Even if the method exists when run, Sorbet still might report an error
    because the method won't **always** be there. For example, maybe the value
    is [nilable](nilable-types.md), or we have a [union](union-types.md) of a
    handful of different types.

1.  Many times, methods are defined dynamically in Ruby. Sorbet cannot see
    methods defined with `define_method`. Sorbet also can't see methods defined
    using Ruby's `included` + `other.extend(self)` pattern. For such dynamically
    defined methods, Sorbet requires `*.rbi` files which define the method
    statically.

    See the [RBI](rbi.md) docs for how to regenerate the `*.rbi` files.

1.  <a class="anchor" aria-hidden="true" id="include-kernel"></a>Sorbet will
    complain about this code:

    ```ruby
    module MyModule; end

    sig {params(x: MyModule).void}
    def foo(x)
      x.nil? # error: Method `nil?` does not exist on `MyModule`
    end
    ```

    The `nil?` method is defined on `Kernel` in Ruby. `Kernel` is included in
    `Object` (which classes default to inheriting from), but not on
    `BasicObject` (which classes can optionally inherit from).

    The solution is to `include Kernel` in our module:

    ```ruby
    module MyModule
      include Kernel
    end
    ```

1.  Sorbet will complain about this code:

    ```ruby
    module MyModule
      def foo; puts 'hello'; end
    end
    ```

    The issue is similar to the above: `puts` is defined on `Kernel`, which is
    not necessarily included in our module. For this situation, there are
    actually two fixes:

    ```ruby
    # Option 1: include Kernel
    module MyModule
      include Kernel

      def foo; puts 'hello'; end
    end
    ```

    ```ruby
    # Option 2: Kernel.puts
    module MyModule
      def foo; Kernel.puts 'hello'; end
    end
    ```

## 7004

This error indicates that a method has been called with incorrect parameters.
There are a few cases where this can occur:

- Too many parameters
- Not enough parameters
- Trying to pass parameters that don't exist
- Missing required parameters
- Positional parameters used when the method expects named parameters, and vice
  versa

```ruby
def foo(x: nil); end

foo(1) # error
foo(y: 1) # error
foo(x: 1) # ok
foo() # ok

def bar(x:); end

bar() # error
bar(1) # error
bar(x: 1) # ok
```

## 7005

Sorbet detected a mismatch between the declared return type for the method and
the type of the returned value:

```rb
sig { returns(Integer) }
def answer
  "42" # error: Expected `Integer` but found `String("42")` for method result type
end
```

Here we specified in the signature that `find` returns an instance of
`Configuration`, yet the returned value might be `nil`:

```rb
sig { params(name: String).returns(Configuration) }
def find(name)
  @lookup[name] # error: Expected `Configuration` but found `T.nilable(Configuration)` for method result type
end
```

A possible solution, if we are _certain_ that `name` is in the `@lookup` hash,
is to use `T.must` when returning the value:

```rb
sig { params(name: String).returns(Configuration) }
def find(name)
  T.must(@lookup[name])
end
```

In some cases, we're already being cautious and perform some checks before
returning yet Sorbet still complains about the return type:

```rb
sig { params(name: String).returns(Configuration) }
def find(name)
  raise ArgumentError, "Configuration #{name} not found" unless @lookup.key?(name)
  @lookup[name] # error: Expected `Configuration` but found `T.nilable(Configuration)` for method result type
end
```

While this code is correct, Sorbet cannot assume the state of `@lookup` didn’t
change between the `key?` check and the `[]` read. To fix this, we can take
advantage of flow-typing to make the whole method work without inline type
annotations:

```rb
sig { params(name: String).returns(Configuration) }
def find(name)
  config = @lookup[name]
  raise ArgumentError, "Configuration #{name} not found" unless config
  config
end
```

By using a local variable, we allow Sorbet to assert that `config` is never
nilable past the `raise` instruction.

## 7006

In Sorbet, it is an error to have provably unreachable code. Because Sorbet is
[sensitive to control flow](flow-sensitive.md) in a program, Sorbet can not only
track what types each variable has within all the branches of a conditional, but
also whether any given branch could be executed at all.

Erroring for dead or unreachable code is generally a way to prevent bugs. People
don't usually expect that some branch of code is never taken; usually dead code
errors come from simple typos or misunderstandings about how Ruby works. In
particular: the only two "falsy" values in Ruby are `nil` and `false`.

Note if you intend for code to be dead because you've exhausted all the cases
and are trying to raise in the default case, use `T.absurd` to assert that a
case analysis is exhaustive. See [Exhaustiveness Checking](exhaustiveness.md)
for more information.

Sometimes, dead code errors can be hard to track down. The best way to pinpoint
the cause of a dead code error is to wrap variables or expressions in
`T.reveal_type(...)` to validate the assumptions that a piece of code is making.
For more troubleshooting tips, see [Troubleshooting](troubleshooting.md).

If for whatever reason it's too hard to track down the cause of a dead code
error, it's possible to silence it by making a variable or expression
"unanalyzable," aka untyped. (When something is untyped, Sorbet will do very
limited flow-sensitivity analysis compared to if Sorbet knows the type. To make
something unanalyzable, we can wrap it in `T.unsafe(...)`:

```ruby
x = false

if x
  puts 'hello!' # error: This code is unreachable
end

if T.unsafe(x)
  puts 'hello!' # ok
end
```

In this (contrived) example, Sorbet knows statically that `x` is always `false`
and so our `puts` within the first `if` is never reachable. On the other hand,
Sorbet allows the second `if` because we've explicitly made `x` unanalyzable
with `T.unsafe(...)`. T.unsafe is one of a handful of
[escape hatches](troubleshooting.md#escape-hatches) built into Sorbet.

## 7007

Sorbet can statically assert that the value passed into a `T.let` does not match
the expected type:

```rb
x = T.let(1, String) # error: Argument does not have asserted type `String`
```

Because of the way default values are desugared by Sorbet, this error also
occurs when Sorbet finds a mistmatch between the type specified for a parameter
in the signature and the default value provided in the method.

In this case, the signature states that `category` type is a `Category`, yet we
try to use `nil` as default value:

```rb
sig { params(name: String, category: Category).void }
def publish_item(name, category = nil) # error: Argument does not have asserted type `Category`
  # ...
end
```

If `category` value is `nil` by default, maybe we should make it so its type is
nilable:

```rb
sig { params(name: String, category: T.nilable(Category)).void }
def publish_item(name, category = nil)
  # ...
end
```

## 7009

This error occurs when a value is used in place of a type. There are many
different situations where this can happen; one example is given below:

```
class Box
  extend T::Generic

  E = type_member
end

Box[true].new # error: Unsupported usage of bare type
```

To fix this error, replace the `true` value with the type `TrueClass`:

```rb
Box[TrueClass].new
```

## 7013

Sorbet detected that an instance variable was reassigned with different types:

```rb
class A
  extend T::Sig

  sig { void }
  def initialize
    @x = T.let(0, Integer)
  end

  sig { void }
  def foo
    @x = 'not an integer' # error: Expected `Integer` but found `String("not an integer")` for field
  end
end
```

If the instance variable can hold both an `Integer` and a `String`, maybe the
type specified with `T.let` should be enlarged:

```ruby
@x = T.let(0, Object)
```

Similarly, Sorbet will reject constants reassigned with different types:

```rb
FOO = 42 # error: Expected `String("Hello, world!")` but found `Integer(42)` for field
FOO = "Hello, world!"
```

## 7010

Sorbet found a reference to a generic type with the wrong number of type
arguments.

Here we defined `MyMap` as a generic class expecting two type parameters
`KeyType` and `ValueType` but we try to instantiate it with only one type
argument:

```rb
class MyMap
  extend T::Generic

  KeyType = type_member
  ValueType = type_member

  # ...
end

MyMap[String].new # error: Wrong number of type parameters for `MyMap`. Expected: `2`, got: `1`
```

Unless a type member was `fixed`, it is always required to pass the correct
amount of type arguments. `T.untyped` can also be used if the type is not
relevant at this point:

```rb
MyMap[String, Integer].new
MyMap[String, String].new
MyMap[String, T.untyped].new
```

## 7014

Sorbet has a special method called `T.reveal_type` which can be useful for
debugging. `T.reveal_type(expr)` will report an error in the output of `srb tc`
that shows what the static component of Sorbet thinks the result type of `expr`
is.

Making this an error is nice for two reasons:

- It makes our internal implementation easier 😅 We don't have some special-case
  messages and then error messages. The only thing Sorbet prints under normal
  circumstances are error messages.

- It serves as a reminder to remove `T.reveal_type` before committing a change.
  Since it's a proper error, Sorbet will exit with non-zero status until it's
  removed.

For more information, see [Troubleshooting](troubleshooting.md).

> Looking for how to assert that an expression has a certain type? Check out
> [Type Assertions](type-assertions.md).

## 7017

In typed: strict files, Sorbet requires that all methods are annotated with a
`sig`. In a `# typed: true` file Sorbet implicitly assumes that definitions
without types are `T.untyped`, but in a `# typed: strict` file, Sorbet will no
longer make this implicit assumption.

You can still add a `sig` which declares the arguments or return as `T.untyped`,
so `# typed: strict` does not outright ban `T.untyped`. The upside is that usage
of `T.untyped` is more explicit, which makes it easier to drive the number of
occurrences down. If you're seeing this warning, there's no time like the
present to add proper types to your public-facing API (i.e., your top-level
constant and method definitions)!

For how to fix, see [Method Signatures](sigs.md).

See also: [5028](#5028), [6002](#6002).

## 7018

At `typed: strong`, Sorbet no longer allows `T.untyped` as the intermediate
result of any method call. This effectively means that Sorbet knew the type
statically for 100% of calls within a file. This sigil is rarely used—usually
the only files that are `# typed: strong` are RBI files and files with empty
class definitions. Most Ruby files that actually do interesting things will have
errors in `# typed: strong`. Support for `typed: strong` files is minimal, as
Sorbet changes regularly and new features often bring new `T.untyped`
intermediate values.

## 7019

Sorbet does not have great support for splats right now.

In general, when considering taking a variable number of arguments, consider
instead taking a single argument that's an array instead of a "rest" arg:

```ruby
# ----- AVOID THIS ----------------------------
sig {params(xs: Integer).void}
def foo(*xs); end

xs = Array.new(3) {|i| i}
foo(*xs)
# ---------------------------------------------

# ----- Do this instead -----------------------

sig {params(ys: T::Array[Integer]).void}
def bar(ys); end

ys = Array.new(3) {|i| i}
bar(ys)
# ---------------------------------------------
```

If it is not possible to refactor the code, the current work around is to use
`T.unsafe`:

```ruby
# ----- WORST CASE ----------------------------
# Prefer the solution described above

sig {params(xs: Integer).void}
def foo(*xs); end

xs = Array.new(3) {|i| i}
T.unsafe(self).foo(xs)
# ---------------------------------------------
```

## 7021

The method called declares a block parameter that is not `T.nilable`, but a
block was not passed when it was called.

This can be fixed by either passing a block to the method, or changing the
method's signature for the block parameter from `T.proc...` to
`T.nilable(T.proc...)` (and then changing the method to deal with a nilable
block parameter).

## 7023

This error occurs when a method is passed in a `Proc` object that Sorbet does
not know the arity for statically.

One instance where this can happen is when using `method`, since the arity of
method corresponding to the symbol is unknown. This can be fixed by passing in a
block with the correct arity:

```ruby
# typed: strict

extend T::Sig

sig {params(blk: T.proc.params(arg0: String).void).void}
def foo(&blk)
end

# ----- AVOID THIS ----------------------------
foo(&method(:puts))
# ---------------------------------------------

# ----- Do this instead -----------------------
foo do |arg0|
    method(:puts).call(arg0)
end
# ---------------------------------------------
```

## 7024

This error occurs when a generic argument is passed in as a block parameter.

In `# typed: strict` files, using a parameter from a method that does not have a
signature will cause this issue to be reported. Adding a signature to the method
will fix the issue.

```ruby
# typed: strict

extend T::Sig

# ----- This will error -----------------------
def foo(&blk)
    proc(&blk)
end
# ---------------------------------------------

# ----- This will not error -------------------
sig {params(blk: T.untyped).returns(T.untyped)}
def bar(&blk)
    proc(&blk)
end
# ---------------------------------------------
```

## 7026

Sorbet detected that it was possible for `T.absurd` to be reached. This usually
means that something that was meant to cover all possible cases of a union type
did not cover all the cases.

See [Exhaustiveness Checking](exhaustiveness.md) for more information.

## 7030

This error is consistently used when the user is trying (implicitly or
explicitly) to call some method on a Sorbet type (e.g. `T::Array[Integer]`)
which would actually return a Sorbet-runtime representation of a type.

This error generally occurs when generic types are used in pattern matching:

```rb
def get_value(input)
  case input
  when Integer
    input
  when T::Array[Integer] # error: Call to method `===` on `T::Array[Integer]` mistakes a type for a value
    input.first
  end
end
```

Since generic types are erased at runtime, this construct would never work when
the program executed. Replace the generic type `T::Array[Integer]` by the erased
type `Array` so the runtime behavior is correct:

```
def get_value(input)
  case input
  when Integer
    input
  when Array
    input.first
  end
end
```

## 7034

Sorbet detected that the safe navigation operator (`&.`) was being used on a
receiver that can never be nil. Replace the offending occurrence of `&.` with a
normal method call (`.`).

```ruby
# typed: true

extend T::Sig

sig {params(x: Integer, y: T.nilable(Integer)).void}
def foo(x, y)
  puts x&.to_s  # error: x can never be nil
  puts x.to_s   # no error

  puts y&.to_s  # no error: y may be nil
end
```

## 7037

See
[this doc for more information](flow-sensitive.md#limitations-of-flow-sensitivity).

In general, Sorbet can't know that two calls to identical methods return
identical things, because in general methods are not pure.

Consider this example

```ruby
class A < T::Struct
  const :foo, T.nilable(Integer)
end

if a.foo && a.foo.even?
  #         ^^^^^^^^^^^ error
  puts a.foo
end
```

In this example, the call to `a.foo.even?` results in an error, even though we
checked that `a.foo` was not `nil` with the `&&`, because Sorbet does not assume
any methods are pure, not even methods defined with the `T::Struct` class's
`const` DSL. (There are a number of technical and philosophical reasons why
Sorbet behaves this way, and we do not foresee these reasons changing).

There is always a simple solution, which is to either factor out the method
call's result into a variable, or to use Ruby's conditional method call operator
(`&.`):

```ruby
# -- solution 1 (preferred) --
x = a.foo
if x && x.even?
  puts x
end

# -- solution 2 --
if a.foo&.even?
  puts a.foo
end
```

Of the two, the first solution is preferred because not only will the program
type check as written, but Sorbet will know that the `x` variable is not `nil`
throughout the body of the `if` statement.

## 7038

Consider this example:

```ruby
sig do
  type_parameters(:U)
    .params(x: T.type_parameter(:U))
    .void
end
def example(x)
  x.foo # error!
end
```

This snippet declares a method `example` with a signature that says it can be
passed any input, but then attempts to call a specific method `.foo`.

Since this method can be given any type of value, Sorbet rejects the call to
`x.foo`.

To allow code like this, use [interfaces](abstract.md) with [intersection
types](intersection-types.md):

```ruby
# (1) Declare an interface
module IFoo
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.void}
  def foo; end
end

sig do
  type_parameters(:U)
    # (2) Use the interface with an intersection type
    .params(x: T.all(IFoo, T.type_parameter(:U)))
    .void
end
def example(x)
  x.foo # error!
end
```

Remember that in Sorbet, [interfaces](abstract.md) must be explicitly
implemented in a given class.

<!-- -->

[report an issue]: https://github.com/sorbet/sorbet/issues

<script src="/js/error-reference.js"></script>
