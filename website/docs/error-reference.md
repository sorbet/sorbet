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

This is one of three docs aimed at helping answer common questions about Sorbet:

1.  [Troubleshooting](troubleshooting.md)
1.  [Frequently Asked Questions](faq.md)
1.  [Sorbet Error Reference](error-reference.md) (this doc)

This page contains tips and tricks for common errors from `srb`.

## 1001

Sorbet has crashed. Please [report an issue]!

## 1003

Sorbet has an internal limitation on how deep a chain of class aliases can be:

```ruby
A1 = Integer
A2 = A1
# ...
A42 = A41
A43 = A42 # error: Too many alias expansions
```

It's meant to guard against cases where Sorbet might get stuck in an infinite
loop attempting to dealias these class aliases.

If you encounter this bug in the wild (i.e., not just for a contrived example,
but a real-world use case), please share with the Sorbet team. We'd like to see
what mode of use triggered this behavior, and either add a test to Sorbet or
tweak how Sorbet works to support the use case.

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

## 2002

Starting in Ruby 3.0, Ruby uses the `_1`, `_2`, etc. syntax for block arguments:

```ruby
xs.map {_1.to_s}

# ^ this is equivalent to:

xs.map { |x| x.to_s }
```

Because of this, `_1` and similar variables are not allowed as normal variable
names. Pick another variable name, for example: `_x1` or `_arg1`.

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

## 2004

Starting in Ruby 3.0, Ruby uses the `_1`, `_2`, etc. syntax for block arguments:

```ruby
xs.map {_1.to_s}

# ^ this is equivalent to:

xs.map { |x| x.to_s }
```

Because of this, `_1` and similar variables are not allowed as normal variable
names. Pick another variable name, for example: `_x1` or `_arg1`.

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

## 3003

Sorbet does not support certain Ruby features, like the flip flop operator and
the `redo` keyword. If you feel strongly that Sorbet support these features,
please open an issue.

Unfortunately, there is no workaround for this issue other than asking Sorbet to
ignore the file (which forces the _entire_ file to be ignored, including any
methods and constants defined in it), or rewriting the file to not use the
unsupported Ruby feature.

If you do decide to ignore the file entirely, you will likely need to use an
[RBI file](rbi.md) to let Sorbet know about any classes and methods defined in
the ignored file.

## 3004

There was an error parsing a float literal in Sorbet. Specifically, we asked a
C++ library to parse a Ruby float literal string to a float, and it returned a
`NaN` value.

If you are seeing this error, it likely represents a bug in Sorbet. Please
report an issue at <https://github.com/sorbet/sorbet/issues>.

## 3005

Sorbet does not support using operators like `&&=` or `||=` when the target of
the operator is a constant literal.

Note that Ruby itself will report a warning for such usage:

```ruby
A = nil
A ||= 1
# => warning: already initialized constant A
```

If you absolutely must reassign a constant using the current value of the
constant, rewrite the code to use `const_defined?` and `const_set`:

```ruby
A = nil
unless Object.const_defined?(:A) && A
  Object.const_set(:A, 1)
end
```

## 3007

Using `yield` in a method body means that a method implicitly takes a block
argument. In `# typed: strict` files, such methods are required to explicitly
name the block argument to make the contract clear:

```ruby
def foo(&blk)
  yield
end
```

This is required even if the block is only ever used by way of `yield`, and
never with something like `blk.call`.

**Why?** The distinguishing factor of `# typed: strict` is that every method has
an explicit interface with a signature. It's equally important to be explicit
about the block argument, if present. For more, see the docs on
[strictness levels](static.md#file-level-granularity-strictness-levels).

## 3008

Sorbet does not support the `undef` keyword. Sorbet assumes that the set of all
classes, modules, methods, and constants is static throughout the lifetime of a
program. It does not attempt to model the way that a Ruby program might mutate
itself by deleting constants or methods at runtime.

For this reason, the `undef` node is not supported.

Currently, this error is only reported at `# typed: strict` or higher, though in
the future this might move to lower strictness levels. It does mean that
currently it's possible to silence this error by downgrading the file containing
the `undef` to `# typed: true` or lower.

Note that regardless of whether Sorbet reports an error for using `undef`, it
has no meaning on what Sorbet considers to be defined or not defined. Sorbet
will not report errors for calls to a method that doesn't exist because it has
been undefined.

## 3009

Sorbet does not support certain kinds of complicated block parameter
destructuring patterns. In most cases, it is possible to rewrite these to use
destructuring assignments inside the block itself:

```ruby
# ----- BAD -----

xs = [[0, 1, 2, 3], 42]
xs.then do |(x, *args), y|
  #             ^^^^^ error: Unsupported rest args in destructure
  p x    # => 0
  p args # => [1, 2, 3]
  p y    # => 42
end

# ----- Use this instead -----

xs.then do |arg0, y|
  x, *args = arg0

  p x     # => 0
  p args  # => [1, 2, 3]
  p y     # => 42
end
```

## 3010

Methods defined in [RBI files](rbi.md) are not allowed to have code in their
bodies.

The only exception is for instance variable assignments (like `@x = ...`), which
are allowed so that RBI files may declare the existence of instance variables
and their types.

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

## 3501

Sorbet has special support for understanding Ruby's `attr_reader`,
`attr_writer`, and `attr_accessor` methods. For this support to work, it must be
able to see the name of the attribute **syntactically**, which means that the
argument must be a `String` or `Symbol` literal.

If you are attempting to dynamically compute the name of an `attr_*` method, you
must either:

1.  Downgrade the file to `# typed: false`, or
2.  Hide `attr_*` method call from Sorbet by using something like
    `public_send(:attr_reader, name)` (instead of `attr_reader name`)

## 3502

`T::InterfaceWrapper` is deprecated and should not be used.

## 3503

Ruby has separate syntax for marking instance methods and singleton class
methods private:

```ruby
private def some_instance_method; end
private_class_method def self.some_singleton_class_method; end
```

Note that the `self.` keyword in the method declaration changes the method from
being an instance method to being a class method. In Ruby this `self.` prefix is
similar to the `static` keyword on method definitions in languages like C++ or
Java.

## 3504

The signature of an `attr_reader`, `attr_writer`, or `attr_accessor` method
cannot have any `T.type_parameter` in it.

These methods get and set instance variables on the underlying class, while the
`T.type_parameter` in the signature would only be in method scope.

## 3505

The `module_function` helper declares that an instance method on a module should
be duplicated onto the class's singleton class.

The `module_function` must be given an argument that is **syntactically**
either:

- a method def
- a `String` or `Symbol` literal with the name of a method

This argument must be provided syntactically because Sorbet has special handling
for `module_function`, and this special logic must be able to see the exact name
of the method that is being defined via `module_function`.

To silence this error, either:

- Refactor the code to use `module_function` with only calls to method
  definitions or literals, or
- Downgrade the file to `# typed: false` or lower, or
- Use `send(:module_function, ...)` to hide the call to `module_function` from
  Sorbet.

## 3506

Enums declared with `T::Enum` are special. A `T::Enum` must have all of its enum
values defined in the `enums do` block inside an enum, and it's not allowed to
have any extra constants defined on the class (i.e., constants that don't hold
values of the enum).

**Why?** Consistency, readability, and simplicity of implementation at runtime.

This includes type aliases. If you'd like to define a type alias consisting of a
set of enum values, the type alias must be declared outside of the `T::Enum`
subclass itself.

Note that enums can still have instance variables and methods defined on them.
For example:

```ruby
class Direction < T::Enum
  enums do
    Up = new
    Down = new
    Left = new
    Right = new
  end

  def self.vertical
    @vertical ||= [Up, Down].freeze
  end
end
```

## 3507

The syntax for `test_each` looks like this:

```ruby
test_each([true, false]) do |x|
  it 'test' do
  end
end
```

For more examples of valid syntax,
[see the tests](https://github.com/sorbet/sorbet/blob/master/test/testdata/rewriter/minitest_tables.rb).

There are some limitations on how `test_each` can be used:

The block given to `test_each` must accept at least one argument (except when
using `test_each_hash`, it must be able to take exactly two arguments).

The body of the `test_each`'s block must contain only `it` blocks, because of
limitations in Sorbet. Sorbet models `it` blocks by translating them to method
definitions under the hood, and method definitions do not have access to
variables outside of their scope.

Usually this comes up with variable destructuring:

```ruby
# -- BAD EXAMPLE --
test_each(values) do |value|
  x, y = compute_x_y(value)
  it 'example 1' do
  end
  it 'example 2' do
  end
end
```

This can be fixed by worked around by writing the assignment into each `it`
block directly, or by computing it ahead of time:

```ruby
test_each(values) do |value|
  it 'example 1' do
    x, y = compute_x_y(value)
  end
  it 'example 2' do
    x, y = compute_x_y(value)
  end
end
```

```ruby
new_values = values.map do |value|
  x, y = compute_x_y(value)
  [value, x, y]
end
test_each(new_values) do |value, x, y|
  it 'example 1' do
  end
  it 'example 2' do
  end
end
```

## 3508

The argument to the `foreign:` attribute on a `prop` declaration must be a
lambda function. This prevents the other model class from needing to be loaded
eagerly. Use the autocorrect to fix the error.

## 3509

The argument to the `computed_by` argument on a prop must be a `Symbol` literal
(i.e., syntactically, so that Sorbet can analyze it without performing any
inference).

## 3510

The return type of the `initialize` method in Ruby is never used. Under the
hood, a call to `new` on a class in Ruby looks something like this:

```ruby
def self.new(...)
  instance = alloc
  _discarded = instance.initialize(...)
  instance
end
```

If you'd like to make a custom factory method for your class, define a custom
singleton class method.

## 3511

Accessor methods defined with `Struct.new` must not end with `=`, because the
generated setter method will then be given an invalid name ending with `==`.

## 3512

`T.nilable(T.untyped)` is just `T.untyped`, because `nil` is a valid value of
type `T.untyped` (along with all other values).

## 3513

The `has_attached_class!` annotation is only allowed in a Ruby `module`, not a
Ruby `class`. For more, see the docs for
[`T.attached_class`](attached-class.md).

## 3514

The `has_attached_class!` annotation cannot be given a contravariant `:in`
annotation because `T.attached_class` is only allowed in output positions.

## 3702

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

Package definitions must be formatted like this:

```ruby
class Opus::Foo < PackageSpec
  # ...
end
```

In this example, `Opus::Foo` is the name of the package, and `PackageSpec`
explicitly declares that this class definition is a package spec.

## 3703

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

Each package must have only one definition. A package definition is the place
where there is a line like `class Opus::Foo < PackageSpec` in a `__package.rb`
file.

## 3704

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

Sorbet found an `import` in a `__package.rb` file, but the imported constant did
not exist.

## 3705

> **TODO** This error code is not yet documented.

## 3706

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

### Import/export statements

All `import` and `export` lines in a `__package.rb` file must have constant
literals as their argument. Doing arbitrary computation of imports and exports
is not allowed in `__package.rb` files.

Also note that all `import` declarations must be unique, with no duplicated
imports.

### autoloader_compatibility declarations

> See [go/pbal](http://go/pbal) for more details.

`autoloader_compatibility` declarations must take a single String argument. The
only allowed value is `legacy`, otherwise the declaration cannot be present.
These declarations annotate a package as incompatible for path-based autoloading
and are used by our Ruby code loading pipeline.

## 3707

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

`__package.rb` files must declare exactly one package.

## 3709

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

A package cannot import itself. Double check which files and/or packages you
intended to modify, as you've likely made a typo.

## 3710

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

Even though `__package.rb` files use Ruby syntax, they do not allow arbitrary
Ruby code. The fact that they use Ruby syntax is a convenience so that:

- package declarations get syntax highlighting in all Ruby editors
- tooling like Sorbet and RuboCop work on `__package.rb` files out of the box
- Sorbet can support things like jump-to-definition inside `__package.rb` files

But despite that, `__package.rb` files must be completely statically analyzable,
which means most forms of Ruby expressions are not allowed in these files.

## 3711

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

Package files must be `# typed: strict`. If you are in the process of migrating
a codebase to use the packager mode and want to sometimes ignore a
`__package.rb` file, use the `--ignore=__package.rb` command line flag which
will ignore all files whose name matches `__package.rb` exactly, in any folder.

## 3712

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

Package names must not contain underscores. Internally, the packager assumes it
can use the underscore character to join components of a package name together.
For example, internally the package uses names like `Opus_Foo` to represent the
package `Opus::Foo`. If underscores were allowed in package names,
`Opus_Foo_Bar` could represent a package called `Opus::Foo_Bar`, `Opus_Foo::Bar`
or `Opus::Foo::Bar`.

## 3713

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

All code inside a package must live within the namespace declared by it's
enclosing `__package.rb` file. Note that since packages are allowed to nest
inside each other, sometimes you might have attempted to add code in a folder
that you didn't realize was actually managed by a nested package.

If you're seeing this error and surprised, double check which folders have
`__package.rb` files in them, and the names of the packages declared by them.

## 3714

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

This error arises when it's unclear which import actually provides a constant,
which in turn usually happens with nested packages: given a constant `A::B::C`
and a package that imports both the packages `A` and `A::B`, it's difficult to
tell without deep examination which actually exports `A::B::C`.

In these cases, it's best to refactor the packages so they are clearly
delineated: instead of `A` and `A::B`, it might be best to figure out what
behavior lives in `A` but not `A::B` and move it to a new package entirely, like
`A::X` (and then update the import structure as needed).

This error can also be produced when trying to import a name which is a prefix
of the nested package: for example, importing `A` when your package name is
`A::B`.

Again, this probably implies you should try to move away from the nested package
structure, and move the contents of `A` that aren't in a nested package into
another less ambiguous package.

## 3715

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

The `export_for_test` directive is used to say, "Make this constant from the
non-test namespace available in the test namespace for the same package." That
means it only makes sense to `export_for_test` constants from the non-test
namespace.

Trying to apply this directive to a constant defined in the `Test::` namespace
will result in this error.

If you're trying to export a constant from the test namespace to be used in
other packages, then just use `export`. Otherwise, these lines can be safely
deleted.

## 3716

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

This error means that you're exporting a constant redundantly. In Stripe
Packages mode, exporting a constant `A::B` will export anything accessible
underneath `A::B`. That means that if you try to export `A::B` and `A::B::C`,
you'll get this error—the latter export is redundant, as it's implied by export
`A::B`.

To fix this error, simply remove the more specific export.

## 3717

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

Referencing a constant defined in another package that has not been explicitly
exported by that package is not allowed.

To fix this error, either change the upstream package to start exporting the
constant, or change the code in the current package to depend on only public
exports of the upstream package. When changing an upstream package to expose
things that were not previously exposed, double check whether it's intentional
that the constant has not been exported.

## 3718

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

To reference constants defined in other packages, the package must first be
imported into the current package.

Fix the error by explicitly `import`'ing the upstream package into the
`__package.rb` file for the package where this error was reported.

Note that sometimes it is not possible to import another package because doing
so would make the dependency graph of packages have a dependency cycle. In these
cases, a common solution is to factor the shared functionality to a new package,
and import that new package wherever it's needed. In some situations, there may
be simpler ways to restructure the code that don't involve making a new package.

## 3720

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

The `--stripe-packages` mode draws a distinction between test and non-test code.
For a package called `A::B`

- Some of the constants defined outside of `A::B` but imported into `A::B` are
  **test-only** imports (created by writing `test_import` instead of `import` in
  a `__package.rb` file).

  To fix this error, if the change is desired, replace `test_import` with
  `import` for the constant.

- Some of the constants defined within `A::B` live in the test namespace of
  `Test::A::B`, meaning that those constants can only be referenced inside
  `T::A::B`, not inside the non-test `A::B` namespace.

  To fix this error, avoid referencing test code from a non-test namespace. This
  means refactoring where code lives within `A::B` to satisfy the above rule.

## 3721

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

A package may only export a constant defined within itself.

To re-export a constant defined by another package, make a constant alias to the
other constant, and export that:

```ruby
class A::B
  SomeOtherConstant = A::X::SomeOtherConstant
end

# -- a/b/__package.rb --

class A::B < PackageSpec
  import A::X

  # BAD
  export A::X::SomeOtherConstant

  # CORRECT
  export A::B::SomeOtherConstant
end
```

Additional signatures of error 3721 include:

- A package exporting a constant only defined in .rbi files. RBI files are shims
  to enable typechecking in places where Ruby metaprogramming prevents Sorbet
  from statically interpreting the behavior of a class or module. Generally,
  these files declare additional methods on classes defined in Ruby source
  files, and should not define any new constants. However, there are some rare
  exceptions where these files can define net-new constants. For these cases, we
  enforce that these constants cannot be exported.
- A package exporting an enum value:

```ruby
module MyPackage
  class A < T::Enum
    enums do
      Val1 = new
      Val2 = new
    end
  end
end

# -- my_package/__package.rb --

class MyPackage < PackageSpec
  export A::Val1 # not allowed, instead the full enum should be exported with `export A`
end
```

## 3722

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

Sorbet's package mode does not currently exporting type aliases. The workaround
is to declare the type alias inside a module, and export the containing module
instead:

```ruby
class A::B
  BadTypeAlias = T.type_alias {T.any(Integer, String)}
end

class A::B::Types
  GoodTypeAlias = T.type_alias {T.any(Integer, String)}
end

class A::B < PackageSpec
  export A::B::BadTypeAlias # error

  export A::B::Types # OK
  # ^ allows referencing `A::B::Types::GoodTypeAlias` in downstream packages
end
```

## 3723

> This error is specific to Stripe's custom `--stripe-packages` mode. If you are
> at Stripe, please see [go/modularity](http://go/modularity) for more.

The `--stripe-packages` mode allows packages to explicitly enumerate which other
packages are allowed to import them by using the `visible_to` directive. If a
package uses one or more `visible_to` lines, and is imported by a package _not_
referenced by a `visible_to` line, then Sorbet will report an error pointing to
that import.

Often, if you're running across this error, it means that you're trying to rely
on an implementation detail that was deliberately made private. However, if
you're sure that it should be okay to import this package, then you can add an
additional `visible_to` directive in order to allow the import you're trying to
add.

## 4001

Sorbet parses the syntax of `include` and `extend` declarations, even in
`# typed: false` files. Recall from the
[strictness levels](static.md#file-level-granularity-strictness-levels) docs
that all constants in a Sorbet codebase must resolve, even at `# typed: false`.
Parsing `include` blocks is required for this, so incorrect usages of `include`
are reported when encountered.

## 4002

Sorbet requires seeing the complete inheritance hierarchy in a codebase. To do
this, it must be able to statically resolve a class's superclass and any mixins,
declared with `include` or `extend`.

To make this possible, Sorbet requires that every superclass, `include`, and
`extend` references a constant literal. It's not possible to use an arbitrary
expression (like a method call that produces a class or module) as an ancestor.
This restriction holds even in `# typed: false` files.

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

(For some intuition why this restriction is in place: Sorbet requires resolving
the inheritance hierarchy before it can run inference. Inference is when it
assigns types to every expression in the codebase. Therefore inheritance
resolution cannot depend on inference, as otherwise there would be a logical
cycle in the order Sorbet has to type check a codebase. Similar restrictions
appear throughout Sorbet: see [Why type annotations?](why-type-annotations.md)
for more examples.)

For module mixins, it is possible to silence this error with `T.unsafe`, but it
should be done with **utmost caution**, as Sorbet will not consider the include
and provide a less accurate analysis:

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

There is no such workaround for superclasses.

## 4003

Sorbet parses the syntax of `include` and `extend` declarations, even in
`# typed: false` files. Recall from the
[strictness levels](static.md#file-level-granularity-strictness-levels) docs
that all constants in a Sorbet codebase must resolve, even at `# typed: false`.
Parsing `include` blocks is required for this, so incorrect usages of `include`
are reported when encountered.

## 4006

The `super` keyword in Ruby will call the method with the same name on the
nearest ancestor (whether on a mixed-in module or the superclass).

This method only makes sense to call inside the body of a method, not inside a
class or file top-level.

## 4010

When using Sorbet, try to avoid redefining a method. A method is redefined when
a method of the same name is defined in the same class (note that Sorbet
completely supports overriding methods, where two methods have the same name but
one is in a parent class and one in a child class).

If redefining a method is unavoidable, the arity of the new method must match
the previous method's arity exactly. A method's arity includes how many
positional arguments a method has, which keyword arguments it takes, etc.

Determining the arity of the previous method can sometimes be tricky, especially
when the previous method was defined dynamically by a DSL. In these cases, the
easiest way to determine a method's arity is to find a place where that method
is **called**, hover over it using Sorbet's [editor integration](vscode.md), and
copy the displayed method definition.

It's worth noting that this error occurs particularly frequently in RBI files,
especially autogenerated RBI files. Consider a case like this:

```ruby
# -- some_gem.rbi --
class SomeGem
  def foo(x = nil, y = nil); end
end

# -- autogenerated/some_gem.rbi --
class SomeGem
  def foo(*args); end
end
```

In this case, two RBIs define conflicting definitions for `SomeGem#foo` because
the arity of `x, y` does not match the arity of `*args`. (This frequently
happens because, say, the gem wants to do custom parameter checking so that in
certain cases `x` or `y` is actually required.) The `autogenerated/` RBI file
was generated by using Ruby's reflection APIs to ask for the arity of the method
as seen by the Ruby VM, while the other RBI was hand-written by the gem
maintainer.

In cases like these, usually the solution is to remove the `foo` definition from
`autogenerated/some_gem.rbi`, which can usually be accomplished by regenerating
the RBI.

**Note**: The arity of a method does not include types, only names and kinds of
its parameters. For example:

```ruby
sig {returns(Integer)}
def foo; end
sig {returns(String)}
def foo; end
```

In this example, both `foo` methods have the same arity (they take no arguments,
or are "nullary"), so no error is raised about redefining a method.

But in this case, where their signatures specify different types, the behavior
is unspecified. Sorbet **usually** takes the types from the "last" signature,
but which signature is "last" is implementation defined when when the two method
definitions happen in two separate files. This includes the case when both a
Ruby source file (`*.rb`) and an RBI file (`*.rbi`) specify a signature for a
method.

That Sorbet supports method redefinitions, including providing multiple
signatures for a method definition across multiple files, and that this error is
**only** reported at `# typed: true` and above is an accident of history, and
part of the reason why Sorbet strongly discourages using method redefinitions.

One alternative is to mark the original method `private` and define a new method
with a new name, instead of redefining the old method. Another alternative is to
use Sorbet's [editor integration](vscode.md) to rename the old method, declare
that old method `private`, and define a new method with the original name.

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

For more information, see the docs for [Generics](generics.md).

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

## 4013

The `interface!` annotation is reserved for modules. To include abstract methods
in a class, mark the class `abstract!` instead.

## 4014

Sorbet does not support dynamic constant references. All constants must be plain
constant literals.

If you are adopting Sorbet in your codebase and get stuck dealing with how to
avoid using a particular dynamic constant reference, you might want to ask
someone in [the Sorbet community](/en/community) whether they have encountered
the problem before. Members of the Sorbet community frequently answer questions
either on Slack or Stack Overflow.

**Why is it this way?** Sorbet has a simple architecture which has been chosen
to optimize for performance in large codebases. Specifically, Sorbet only knows
which method is being called during its inference phase. The inference phase
requires that a complete symbol table (listing all classes, all the methods that
class owns, and all the methods' types) has been built already. Thus building
this symbol table cannot depend on knowing which methods are defined.

This is also a philosophical belief that Ruby codebases are easier for
programmers to understand when constant references are simple.

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

## 4016

`type_member` and `type_template` cannot be used at the top-level of a file.
Instead, they must be used inside a class or module definition.

## 4019

This error is only reported when running Sorbet with the `--stripe-mode` command
line flag.

A class defines behavior in multiple files when at least two files would need to
be run in order to completely load that class. A class definition that only
serves as a namespace for inner definitions is not considered to have behavior.
For example, in this example module `A` has behavior in two files:

```ruby
# -- file1.rb --
module A
  def foo; end
end

# -- file2.rb --
module A
  def bar; end
end
```

However, in this example, module `A` does not have any behavior:

```ruby
# -- file1.rb --
module A; end
module A::B
  def foo; end
end

# -- file2.rb --
module A; end
module A::B
  def bar; end
end
```

The limitations around what constitutes "defining behavior" is intertwined with
which files would have to be loaded for a class (like `A` above) to be fully
loaded. In `--stripe-mode`, there must be at most one file to require to fully
load a class.

## 4021

The syntax for specifying type bounds when using `type_member` and
`type_template` has changed. The old syntax looked used a method call with
keyword args:

```ruby
type_member(fixed: ...)
```

while the new syntax uses a block that returns a `Hash` literal:

```ruby
type_member {{fixed: ...}}
```

(Note that any variance annotation like `:in` or `:out` is still specified as
the first positional argument.)

This new syntax mimics the syntax for `T.type_alias`, and shares the same
motivation: generics in Sorbet are completely erased at runtime, so it's silly
to have to pay the runtime price of computing the runtime types passed to
`:fixed`, `:upper`, and `:lower`. In codebases that heavily autoload constants,
it's also easy for `type_member` definitions to cause constants to be loaded
earlier than they might have been otherwise (potentially introducing load-time
cyclic references).

The first version of Sorbet on RubyGems to support the new syntax is 0.5.9889.
It accepts both syntaxes side by side, so you can use it while incrementally
migrating your codebase to the new syntax.

This error includes an autocorrect you can run to automatically migrate to the
new syntax:

```
srb tc --isolate-error-code=4021 --autocorrect
```

## 4022

Sorbet does not model all Ruby constants using the same internal data structure.
Instead, models constants differently depending on how the constant is defined.
For example, all three of these constant definitions are treated differently in
Sorbet:

```ruby
class A; end      # a class or module constant
B = 1             # a "normal" constant, which Sorbet calls a "static field"
C = type_member   # a type member constant, used with generic classes
```

Sorbet requires that a constant with a given name must have a unique constant
"kind" throughout all its appearances in a codebase. For example, even though
code like this is valid Ruby code, it's not allowed in Sorbet:

```ruby
def method_to_compute_a_Class_object
  Class.new
end

MyClass = method_to_compute_a_Class_object()

class MyClass
  # ... re-open MyClass to add methods to it ...
  def foo; end
end
```

In the snippet above, Sorbet complains that it can see conflicting definitions
of `MyClass`: one as a static field, and one as a class.

Sorbet imposes this limitation for performance—Sorbet does not do method-level
type inference until it has a full view of all the constants defined in a
program.

At the point where the `MyClass = ...` assignment happens, Sorbet must choose
whether to treat `MyClass` as a class or module (which allows using it as a
[class or interface type](class-types.md), grants it a singleton class which
derives from `Module`, lets it own other constants and methods, and more) or
whether to treat it as a "normal" static field constant, which does not allow
any of those things.

To workaround issues like this, if you absolutely must have both a constant
assignment and a class definition for a given constant, you can either:

1.  Use `const_set` to hide the constant assignment from Sorbet:

    ```ruby
    def method_to_compute_a_Class_object
      Class.new
    end

    const_set(:MyClass, method_to_compute_a_Class_object())

    class MyClass
      def foo; end
    end
    ```

2.  Factor the code such that the constant assignment is in a file all by
    itself, and mark that file `# typed: ignore` (possibly also using an `RBI`
    file to declare anything that can't be factored out of the ignored file but
    should still be visible to Sorbet).

## 4023

The `has_attached_class!` annotation is only allowed in a Ruby `module`, not a
Ruby `class`. For more, see the docs: [`T.attached_class`](attached-class.md).

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

## 5003

Sorbet failed to parse a method signature. For more documentation on valid `sig`
syntax, see [Method Signatures](sigs.md).

## 5004

Sorbet failed to parse a Ruby expression as a valid Sorbet type annotation.
Sorbet supports many type annotations—use the sidebar to find relevant docs on
the kinds of valid Sorbet type annotations.

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
  X = type_member
end

class Child < Parent
end
```

We need to change our code to redeclare the type member in the child class too:

```ruby
# typed: true
class Parent
  extend T::Generic
  X = type_member
end

class Child < Parent
  X = type_member
end
```

The same thing holds for type templates.

Note that when the ancestor is a `module` that has added to a child class using
`extend`, the child class will need to use `type_template` to redeclare the
module's type members

```ruby
# typed: true
module IFoo
  extend T::Generic
  X = type_member
end

class A
  extend T::Generic
  extend IFoo
  X = type_template
end
```

The intuition here is similar to how using `extend` on an interface requires
implementing its abstract methods as singleton class methods (`def self.foo`)
instead of instance methods (`def foo`).

For more information, see the docs for [Generics](generics.md).

## 5015

[Variance](<https://en.wikipedia.org/wiki/Covariance_and_contravariance_(computer_science)>)
is a type system concept that controls how generics interact with subtyping.

If a `type_member` is declared as covariant (`:out`) in a parent class or
module, it must be declared as either covariant or invariant in any children of
that class or module. (A `type_member` with no variance annotation is
invariant.)

Similarly if a `type_member` is declared as contravariant (`:in`) in a parent
class or module, it must be declared as either contravariant or invariant in any
children.

To see why, consider this example:

```ruby
module Parent
  extend T::Generic
  X = type_member(:out)
end

module Child
  extend T::Generic
  include Parent

  X = type_member(:in) # error: variance mismatch
end
```

If Sorbet were to allow this, Sorbet would fail to catch type errors that it
would need to be able to catch. To see why, consider this chain of `T.let`
statements, all of which have no error:

```ruby
x1 = Child[T.any(String, Symbol)].new

# `Child::X` is contravariant, so "String <: T.any(String, Symbol)"
# implies "Child[T.any(String, Symbol)] <: Child[String]"
x2 = T.let(x1, Child[String])

# `String` is equivalent to `String`, so Child[String] <: Parent[String]
x3 = T.let(x2, Parent[String])

# `Parent::X` is covariant, so "String <: T.any(String, Integer)"
# implies "Parent[String] <: Parent[T.any(String, Integer)]"
x4 = T.let(x3, Parent[T.any(Integer, String)])
```

Given the above definitions, with `Parent::X` being covariant and `Child::X`
being contravariant, each subsequent `T.let` checks out, with the reasons being
specified.

But that's a contradiction! We were able to make a conclusion that we know is
false. If we had jumped straight from where we started to where we finished,
Sorbet would report an error:

```ruby
y1 = Child[T.any(String, Symbol)].new
T.let(y1, Parent[T.any(Integer, String)])
#     ^^ error: Argument does not have asserted type
```

This is because `T.any(Integer, String)` is neither a subtype nor a supertype of
`T.any(Symbol, String)`, the types are completely incompatible.

To avoid introducing contradictions like this into the type system, Sorbet
requires that the variance on parent and child classes matches.

[→ View full example on sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0A%0Amodule%20Parent%0A%20%20extend%20T%3A%3AGeneric%0A%20%20X%20%3D%20type_member%28%3Aout%29%0Aend%0A%0Amodule%20Child%0A%20%20extend%20T%3A%3AGeneric%0A%20%20include%20Parent%0A%0A%20%20X%20%3D%20type_member%28%3Ain%29%0Aend%0A%0Ax1%20%3D%20Child%5BT.any%28String%2C%20Symbol%29%5D.new%0A%0A%23%20%60Child%3A%3AX%60%20is%20contravariant%2C%20so%20%22String%20%3C%3A%20T.any%28String%2C%20Symbol%29%22%0A%23%20implies%20%22Child%5BT.any%28String%2C%20Symbol%29%5D%20%3C%3A%20Child%5BString%5D%22%0Ax2%20%3D%20T.let%28x1%2C%20Child%5BString%5D%29%0A%0A%23%20%60String%60%20is%20equivalent%20to%20%60String%60%2C%20so%20Child%5BString%5D%20%3C%3A%20Parent%5BString%5D%0Ax3%20%3D%20T.let%28x2%2C%20Parent%5BString%5D%29%0A%0A%23%20%60Parent%3A%3AX%60%20is%20covariant%2C%20so%20%22String%20%3C%3A%20T.any%28String%2C%20Integer%29%22%0A%23%20implies%20%22Parent%5BString%5D%20%3C%3A%20Parent%5BT.any%28String%2C%20Integer%29%5D%22%0Ax4%20%3D%20T.let%28x3%2C%20Parent%5BT.any%28Integer%2C%20String%29%5D%29%0A%0Ay1%20%3D%20Child%5BT.any%28String%2C%20Symbol%29%5D.new%0AT.let%28y1%2C%20Parent%5BT.any%28Integer%2C%20String%29%5D%29)

## 5016

> Note: more recent versions of Sorbet have eliminated this error--it is now
> possible to define generic classes with covariant and contravariant type
> members.

Sorbet does not allow classes to be covariant nor contravariant.

**Why?** The design of generic classes and interfaces in Sorbet was heavily
inspired by the design of C#. This exact question has been answered for C# by
Eric Lippert, who worked on the design and implementation of C# for many years.

→
[Why isn't there variance for generic classes?](https://stackoverflow.com/questions/2733346/why-isnt-there-generic-variance-for-classes-in-c-sharp-4-0/2734070#2734070)

The answer concludes that the main selling point of having covariant classes is
to make immutable generic classes, at the cost of a more complex implementation
of generics.

## 5017

The `type_member` and `type_template` declarations from a parent class must all
be repeated in the same order in a child class (and before any newly-added type
members belonging only to the child class).

One non-obvious way this error can manifest is via code generation tooling. If a
tool attempts to generate `type_member` declarations using runtime reflection,
but does them out of order, that will confuse Sorbet. Ask the owner of the code
generator for help resolving the problem.

## 5018

A child class defined a normal constant with a name that was already in use as a
`type_member` in the parent class. For example:

```ruby
class Parent
  extend T::Generic

  X = type_member
end

class Child < Parent
  X = 0 # error!
end
```

In this example, since `X` is declared as a `type_member` in the parent, it must
also be declared as a `type_member` in the child, and cannot be changed to some
other kind of constant in the child.

## 5019

Abstract methods must not have bodies. For more information, see
[Abstract Classes and Interfaces](abstract.md). Despite these methods not having
a body, Sorbet's [runtime support](runtime.md) via the `sorbet-runtime` gem will
convert these methods to raise. For example:

```ruby
module IService
  extend T::Sig
  extend T::Helpers
  interface!

  sig {abstract.returns(String)}
  def port; end
end

class MyService
  include IService
  # For sake of example, let's "forget" to implement `port`
  # (Sorbet would report a static error here)
end

MyService.new.port # raises an exception at runtime:

# example.rb:16:
#   The method `port` on `IService` is declared as `abstract`.
#   It does not have an implementation.
```

So you do not need to manually insert a `raise` inside the body of an abstract
method.

If you would like to define a method in an abstract class or interface with a
default implementation, use the `overridable` annotation on the signature:

```ruby
class AbstractService
  extend T::Sig

  # Default port of 8080, but can be overridden in the child class.
  sig {overridable.returns(String)}
  def port; '8080'; end
end
```

## 5020

There are a few constraints around how methods like `include`, `extend`,
`mixes_in_class_methods` [(docs)](abstract.mdd), and `requires_ancestor`
[(docs)](requires-ancestor.md) work.

- `include` and `extend` must be given a constant that Sorbet can _statically_
  see resolve to a `module`. If a codebase is using metaprogramming to define
  constants that are later mixed with these methods, the codebase must either:
  - Ensure that all relevant `include` and `extend` targets are statically
    defined in `*.rb` or `*.rbi` files that are not being ignored, or
  - Hide the `include` or `extend` invocations from Sorbet by using
    [`# typed: ignore` sigils](static.md) to ignore the entire file, or
  - Hide only the individual call from Sorbet statically using something like
    `send(:include, ...)`.
- `mixes_in_class_methods` and `requires_ancestor` must only be declared in a
  `module`, not a `class`. Classes are never mixed into other classes or
  modules, so these methods would have no meaning in a class.

## 5021

Sorbet requires that classes or modules which define `abstract` methods are also
marked as abstract using either `abstract!` or `interface!`.

For more information, see [Abstract Classes and Interfaces](abstract.md).

## 5022

Sorbet requires that modules marked `interface!` must only have abstract
methods. To have a module with some abstract methods and some implemented
methods, use `abstract!` in the module instead.

Apart from this error message, there is otherwise essentially no difference
between abstract modules and interface modules. `interface!` is provided mostly
as a way for the author of a piece of code to declare intent to the reader more
than anything else.

## 5023

Some modules require specific functionality in the receiving class to work. For
example `Enumerable` needs a `each` method in the target class.

Failing example in
[sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20Example%0A%20%20include%20Enumerable%0Aend):

```
class Example
  include Enumerable
end
```

To fix this, implement the required abstract methods in your class to provide
the required functionality.

Passing example in
[sorbet.run](https://sorbet.run/#%23%20typed%3A%20true%0A%0Aclass%20Example%0A%20%20include%20Enumerable%0A%0A%20%20def%20each%28%26blk%29%0A%0A%20%20end%0Aend):

```
class Example
  include Enumerable

  def each(&blk)
  end
end
```

## 5024

This error usually happens when attempting to define a cyclic type alias:

```ruby
X = T.type_alias {X} # error: Unable to resolve right hand side of type alias
```

Note that Sorbet does not support making recursive type aliases. To make a
recursively-defined data structure, see
[Approximating algebraic data types](sealed.md#approximating-algebraic-data-types).

## 5025

Sorbet does not currently support type aliases in generic classes.

This limitation was crafted early in the development of Sorbet to entirely
sidestep problems that can arise in the design of a type system leading to
unsoundness. (Sorbet users curious about type system design may wish to read
[What is Type Projection in Scala, and Why is it Unsound?](https://lptk.github.io/programming/2019/09/13/type-projection.html).)

We are likely to reconsider lifting this limitation in the future, but have no
immediate plans yet.

As a workaround, define type aliases somewhere else. Unfortunately this does
mean it is not currently possible to define type aliases that reference type
members defined by a generic class.

## 5026

Various classes in the standard library have been defined as generic classes
retroactively, not via Sorbet's built-in support for generics. Sorbet's usual
mechanism for defining custom generic classes is to define the `[]` method on
the generic class's singleton class, so that syntax like
`MyGenericClass[Integer]` is valid code at runtime.

However for various stdlib generic classes, this syntax is already taken. For
example:

```ruby
# BAD EXAMPLE

Array[Integer]
# => evaluates at runtime to `[Integer]`
# (i.e. a list with one element: the `Integer` class object)
```

To use these standard library classes in type annotations, prefix the generic
class's name with `T::`, like this:

```ruby
sig {returns(T::Array[Integer])}
def foo; [0]; end
```

For more information, see
[Arrays, Hashes, and Generics in the Standard Library](stdlib-generics.md).

## 5027

> This error is opt-in, behind the `--check-out-of-order-constant-references`
> flag.
>
> Sorbet does not check this by default because certain codebases make clever
> usage of Ruby's `autoload` mechanism to allow all constants to be referenced
> before their definitions.

This error fires when a constant is referenced before it is defined.

```ruby
puts X # error: `X` referenced before it is defined
X = 1
```

```ruby
module Foo
  A = X
    # ^ error: `Foo::X` referenced before it is defined
  class X; end
end
```

Generally, Sorbet is not opinionated about definition-reference ordering. It
assumes files are required in the correct order or at the correct times to
ensure that definitions are available before they're referenced.

However, if a constant is defined in a single file, Sorbet can detect when it's
been referenced in that file ahead of its definition (because in the single-file
case, it doesn't matter whether or in what order any require statements happen).
There are some limitations:

### Load-time scope must be established definitively

Sorbet has to prove definitively that a given constant is accessed out-of-order
at load time. It cannot track accesses across function calls or blocks, meaning
that the following code, while technically unloadable, will not throw a Sorbet
error.

```ruby
module Foo
  def bar(&blk)
    yield
  end

  bar do
    A = X # this will not report an error
  end

  class X; end
```

### Symbols have to be guaranteed to exist only in one file

In the above example, if `Foo::X` is also declared in another file, the error
will not fire. In such cases, the other file that defines `X` may get required
first, so Sorbet cannot prove that there will be a problem referencing `X` in
this file.

Ways to fix the error include:

- Re-ordering the constant access below the declaration.
- In the case of classes, adding an empty pre-declaration before the access.

## 5028

In `# typed: strict` files, Sorbet requires that all constants are annotated
with a `T.let`.

For how to fix, see [Type Annotations](type-annotations.md).

See also: [6002](#6002), [7017](#7017), [7027](#7027), [7043](#7043).

## 5030

A constant cannot store a reference to itself, like this:

```ruby
X = X
```

## 5031

Constants are not allowed to resolve through type aliases. For example, this is
an error:

```ruby
class A
  X = 0
end

AliasToA = T.type_alias {A}

AliasToA::X # error: not allowed
```

**Why**? A number of reasons:

- Type aliases do not always store references to plain classes. For example,
  sometimes type aliases store references to types like `T.nilable(A)`. Sorbet
  doesn't allow resolving constants through type aliases for the same reason
  that it doesn't allow writing `T.nilable(A)::X`.

- The runtime representation of type aliases do not behave like constants at
  runtime, so `AliasToA::X` would not actually resolve to `A::X` at runtime.

If the type alias is merely an alternate name for an existing constant, use a
class alias not a type alias:

```ruby
AliasToA = A
AliasToA::A # allowed
```

## 5032

See the docs for error code [5020](#5020).

## 5033

Final methods cannot be overridden by definition. See
[Final Methods, Classes, and Modules](final.md) for more information.

The error code 5033 is raised only statically, but it is worth noting that
attempting to the override of a final method statically but still have the
method overridden at runtime will not work—Sorbet's runtime support for `final`
methods prevents methods from being overridden at runtime as well. This is
intentional. Final methods cannot even be overridden or redefined by mocks or
stubs in a test suite.

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
[Override Checking](override-checking.md) for more information about `override`.

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

```ruby
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

```ruby
# sorbet/rbi/shims/foo.rbi
# typed: true

class Foo
  def foo; end
end
```

Sometimes, Sorbet will complain about an alias to a method coming from an
included modules. For example, here `bar` is coming from the inclusion of `Bar`
but Sorbet will complain about the method not existing anyway:

```ruby
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

```ruby
class Foo
  include Bar

  def foo
    bar
  end
end
```

## 5038

When Sorbet detects that a file is using `sig` syntax for methods, it requires
an explicit `# typed:` sigil. The default `# typed:` sigil in Sorbet is
`# typed: false`, so if you would like to add signatures in a method but change
nothing else about how Sorbet checks your codebase, add `# typed: false` to the
top of the file.

However, note that Sorbet will not check the method body's implementation
against the signature at `# typed: false`. When you see this error, it's
**strongly recommended** that you additionally upgrade the file to at least
`# typed: true`.

Regardless of whether the file where this error was reported is `# typed: false`
or `# typed: true`, all other `# typed: true` files in the codebase where Sorbet
detects a call to this method will still be checked against this method's
signature.

To summarize:

```ruby
# -- a.rb --
# typed: false

class A
  extend T::Sig
  sig {params(x: Integer).returns(String)}
  def int_to_string(x)
    x # no static error!
      # (this file is `# typed: false`)
  end
end

# -- b.rb --
# typed: true

a = A.new
res = a.int_to_string('not an int') # error: Expected `Integer` but got `String`

res + 1 # error: Expected `String` but found `Integer`
```

For more information on these typedness levels, see
[Enabling Static Checks](static.md).

## 5039

Sorbet relies on running inference in a method to be able to reveal the type of
an expression with `T.reveal_type`. For performance reasons, Sorbet skips
running inference in any file that is `# typed: false`, because none of the
other inference-related errors will be reported at that typedness level.

To use `T.reveal_type` in a file, upgrade the file to `# typed: true` or above.

For more information on typedness levels, see
[Enabling Static Checks](static.md).\
For more information on `T.reveal_type`, see [Troubleshooting](troubleshooting.md).

## 5040

Overloading a method (by providing multiple signatures for the method) is only
allowed for methods defined in the Ruby standard library. The definitions for
the Ruby standard library live inside Sorbet's textual and binary payload, and
cannot be updated by Sorbet end-users except by sending a pull request to Sorbet
(see
[this FAQ entry for more](faq.md#it-looks-like-sorbets-types-for-the-stdlib-are-wrong)).

Note that even within the type definitions for the Ruby standard library,
overloads in Sorbet come with substantial and somewhat fundamental limitations.

When attempting to define methods that would require overloading to properly
type them, users are strongly recommended to instead define multiple separate
methods. For example, instead of defining a method like this:

```ruby
# BAD example, will not typecheck

sig {params(idx: Integer, raise_if_not_found: TrueClass).returns(String)}
sig {params(idx: Integer).returns(T.nilable(String))}
def get_element(idx, raise_if_not_found)
  # ...
end
```

This attempts to define a `get_element` method that normally returns
`T.nilable(String)` when called like `get_element(0)`, but will instead always
return `String` or raise an exception if called like `get_element(0, true)`.

The idea is that these are conceptually two different methods, which is the
suggestion for how to refactor the code:

```ruby
sig {params(idx: Integer).returns(T.nilable(String))}
def get_element(idx)
  # ...
end

sig {params(idx).returns(String)}
def get_element_or_raise(idx)
  elem = get_element(idx)
  case elem
  when NilClass then raise "No element found at index #{idx}"
  else elem
  end
end
```

We recommend such a refactoring even when attempting to write sigs for methods
defined in gems outside the standard library.

If this workaround will not work in your case, the final alternative is to
either omit a signature for the method in question (or define an explicit
signature where all parameters that cannot be typed accurately use `T.untyped`).

**Why are overloads not well-supported in Sorbet?**

Consider how overloading works in typed, compiled languages like C++ or Java;
each overload is a separate method. They actually have separate implementations,
are type checked separately, compile (with link-time name mangling) to separate
symbols in the compiled object, and the compiler knows how to resolve each call
site to a specific overload ahead of time, either statically or dynamically via
virtual dispatch.

Meanwhile, Ruby itself doesn't have overloading—there's only ever one method
registered with a given name in the VM, regardless of what arguments it accepts.
That complicates things. It becomes unclear how Sorbet should typecheck the body
of the method (against all sigs? against one sig? against the component-wise
union of their arguments?). There's no clear answer, and anything we choose will
be bound to confuse or surprise someone.

Also because Sorbet doesn't control whether the method can be dispatched to,
even if it were going to make a static claim about whether the code type checks,
it doesn't get to control which (fake) overload will get dispatched to at the
call site (again: there's only one version of the method in the VM).

Finally this choice is somewhat philosophical: codebases that make heavy use of
overloading (even in typed languages where overloading is supported) tend to be
harder for readers to understand at a glance. The above workaround of defining
multiple methods with unique names solves this readability problem, because now
each overload has a descriptive name.

## 5041

See [T::Struct: Structs and inheritance](tstruct.md#structs-and-inheritance) for
more information.

## 5042

Sorbet does not allow defining a `private` method in an `interface!`. Note that
this limitation is lifted if the `module` is declared `abstract!` not
`interface`.

The justification for why this is an error has been lost to the sands of time.
It seems reasonable to implement support for this.

<https://github.com/sorbet/sorbet/issues/5687>

## 5043

The syntax for declaring type aliases has changed. What used to be written like
this:

```ruby
# BAD
X = T.type_alias(Integer)
```

must now be written like this, with a block argument instead of a positional
argument:

```ruby
# GOOD
X = T.type_alias {Integer}
```

The new syntax avoids incurring eagerly evaluating the right-hand side of the
type alias, potentially forcing one or more constants to be autoloaded before
they would otherwise be required. This improves load-time performance in
lazily-loaded environments (usually: development environments) and also prevents
certain Sorbet usage patterns from introducing load-time cyclic references.

## 5044

As background reading, you may first want to read more about
[variance](generics.md#in-out-and-variance).

When a type member is declared normally, without any variance annotation, it is
invariant. It can then appear either in the `params` list or the `returns` of a
method's signature, but then prevents using subtyping on that type member. For
example:

```ruby
module IBox
  extend T::Generic
  Elem = type_member
end

sig {params(x: IBox[Integer]).void}
def example(x)
  T.let(x, IBox[T.any(Integer, String)]) # error: Argument does not have asserted type
end
```

In this example, even though `Integer` is a subtype of `T.any(Integer, String)`,
`IBox[Integer]` is not a subtype of `IBox[T.any(Integer, String)]` because
`Elem` has not been declared as `:out` nor `:in`, and is thus **invariant**.

To allow code like this, we can declare `Elem` using `:out`, but this comes at
the restriction of only being able to use `Elem` in **out positions**. See
[Input and output positions](generics.md#input-and-output-positions) for more
information.

The ways to fix this error include:

- Make the type invariant by removing the `:in` or `:out` annotation on the
  type. (This comes with the normal restrictions on invariant type members.)
- Mark the method in question `private`. (This comes with the normal
  restrictions on `private` methods.)

If neither of these works, you'll have to reconsider whether it's possible to
statically type the code in question, and how best to rewrite the code so that
it can be typed statically.

> **Note** that `T.attached_class` is actually modeled as a covariant (`:out`)
> `type_template` defined automatically on all singleton classes, which means
> that `T.attached_class` can only be used in `:out` positions.

## 5045

This error regards misuse of user-defined generic classes. This error is
reported even at `# typed: true`. Otherwise, the error explanation is the same
as for error code [5046](#5046).

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

For legacy reasons relating to the initial rollout of Sorbet, misuse of generics
defined in the standard library error are only reported at `# typed: strict`,
despite being reported at `# typed: true` for all user-defined generic classes.
(In an ideal world, it would have always been reported at `# typed: true`, and
we might change this in the future.)

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

## 5049

Parameters given type annotations in a method signature must be provided in the
same order they appear in the method definition, even for keyword parameters for
which order would not usually be required. This is for readability and
consistency.

Also, all optional keyword parameters must come after all required keyword
parameters.

## 5050

Sealed classes (or modules) can only be inherited (or included/extended) in the
same file as the parent.

For more information, see [Sealed Classes](sealed.md)

## 5051

See [Override Checking](override-checking.md).

## 5052

When providing bounds for a type member with `lower` and `upper`, the lower type
bound must be a subtype of (or equivalent to) the `upper` type bound.

Otherwise, it would never be possible to instantiate the generic type because
the bounds would never be satisfiable.

## 5053

For classes that subclass generic classes, the child's lower and upper bound
must still be within the lower and upper bound range specified by the parent
class.

This problem can come up from time to time when the parent class uses `fixed`
when it meant to use `upper`. For example:

```ruby
class IntOrStringBox
  extend T::Generic
  Elem = type_member {{fixed: T.any(Integer, String)}}
end

class IntBox < IntOrStringBox
  Elem = type_member {{fixed: Integer}} # error, incompatible bound
end
```

In this case we'd like `IntBox` to be a subclass of `IntOrString` box, but since
we're using `fixed` Sorbet prevents us.

A fix is to use only `upper` instead:

```ruby
class IntOrStringBox
  extend T::Generic
  Elem = type_member {{upper: T.any(Integer, String)}}
end

class IntBox < IntOrStringBox
  Elem = type_member {{upper: Integer}}
end
```

But this does come at the slight cost that now all type annotations that used to
be using `IntOrStringBox` without any generic type arguments must now provide
one:

```ruby
T.let(IntBox[Integer].new, IntOrStringBox) # error: Generic class without type arguments
```

The fix is to change all occurrences of `IntOrStringBox` without type arguments
to specify `IntOrStringBox[T.any(Integer, String)]`.

## 5054

Use of `implementation` has been replaced by `override`.

## 5055

Sorbet does not consider a class "fully resolved" until all of its type members
have also been fully resolved. Sorbet cannot fully resolve a type member in a
class until all constants mentioned in its right-hand side have been fully
resolved. So if any of those right-hand-side constants are themselves generic
classes, Sorbet will detect a loop and report this error. Concretely:

```ruby
class A
  extend T::Sig
  extend T::Generic

  X = type_member {{fixed: B}} # error: A::X is involved in a cycle
end

class B
  extend T::Sig
  extend T::Generic

  X = type_member {{fixed: A}} # error: B::X is involved in a cycle
end
```

## 5056

The `T.experimental_attached_class` syntax has been stabilized and is now
`T.attached_class`. Use the provided autocorrect to migrate.

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

## 5059

The syntax for the second argument to `T::NonForcingConstants.non_forcing_is_a?`
is much more constrained than what Sorbet allows for when resolving arbitrary
constant literals.

For more information, see [T::NonForcingConstants](non-forcing-constants.md).

## 5060

When applying a type argument to a generic class, the type argument must be
between any `lower` and `upper` bound declared by the `type_member` (or
`type_template`) declaration for that type parameter.

## 5061

A constant was marked private with Ruby's `private_constant` method, which means
it's only valid to access that constant directly, with no constant literal
prefix scope:

```ruby
class A
  X = 0
  private_constant :X

  X # ok

  module Inner
    X # ok
  end
end

A::X # error
```

If you must access the private constant, you will have to use `.const_get` to
both hide the constant access from Sorbet and appease the Ruby VM:

```ruby
A.const_get(:X)
```

Note that this technique should be used very sparingly, and many codebases have
lint rules or other coding conventions discouraging or even preventing the use
of `const_get`.

## 5062

Invalid syntax for `requires_ancestor`. See
[Requiring Ancestors](requires-ancestor.md) for more information.

## 5063

Useless `requires_ancestor`, because the given class or module is already an
ancestor. See [Requiring Ancestors](requires-ancestor.md) for more information.

## 5064

Using the `requires_ancestor` method, module `Bar` has indicated to Sorbet that
it can only work properly if it is explicitly included along module `Foo`. In
this example, we see that while module `Bar` is included in `MyClass`, `MyClass`
does not include `Foo`.

```ruby
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

```ruby
class MyClass
  include Foo
  include Bar
end
```

Other potential (albeit less common) sources of this error code are classes that
are required to have some class as an ancestor:

```ruby
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

```ruby
class MySuperClass < Foo
  extend T::Helpers
  include Bar

  abstract!
end

class MyClass < MySuperClass
end
```

## 5065

Unsatisfiable `requires_ancestor`. See
[Requiring Ancestors](requires-ancestor.md) for more information.

## 5067

A class's superclass (the `Parent` in `class Child < Parent`) must be statically
resolvable to a class, not a module.

## 5068

Sorbet requires that class, module or constant definitions be namespaced
unambiguously. For example, in this code:

```ruby
# typed: true

module B
end

module A
  class B::C
   # ...
  end
end
```

The definition `B::C` is ambiguous. In Ruby's runtime, it resolves to `B::C`
(and not `A::B::C`). However, things are different in the presence of a
pre-declared filler namespace like below:

```ruby
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

In this case, the definition resolves to `A::B::C` in Ruby's runtime.

By default, Sorbet assumes the presence of filler namespaces while typechecking,
regardless of whether they are explicitly predeclared like in the second
example. This means that in Sorbet's view, the definition resolves to `A::B::C`
in either case.

In Stripe's codebase, this is generally not a problem at runtime, as we use
Sorbet's own autoloader generation to pre-declare filler namespaces, keeping the
Ruby runtime's behavior equivalent to Sorbet. However, the autoloader has some
edge cases, which can often cause deviations between Ruby's runtime and Sorbet.
This error helps guard against these issues.

## 5069

There must be exactly one statement in a method signature (possibly a single
chain of methods).

Sorbet used to allow syntax like

```ruby
sig do
  params(x: Integer)
  returns(Integer)
end
def foo(x); x; end
```

where the body of the `sig` block was allowed to have multiple methods not
connected by a method call chain.

Use the provided autocorrect to convert the old syntax to the new syntax.

## 5070

`T.nilable(T.untyped)` is just `T.untyped`, because `nil` is a valid value of
type `T.untyped` (along with all other values).

## 5071

Providing a `.bind` when using `T.proc` is only valid on a method's single block
argument (`&blk`), not on all arguments. The `.bind` is not actually a type
annotation, but instead an instruction to the type inference algorithm about how
to typecheck the body of the block. Type checking for the body of the block
provided to a method call happens **after** type checking all other arguments,
while type checking for lambda functions and procs passed as positional or
keyword arguments happens **before** the proc value is checked against the
`T.proc` type. For example:

```ruby
sig do
  params(
    f: T.proc.returns(Integer),
    blk: T.proc.returns(Integer),
  )
  .void
def example(f, &blk)
end

f = ->() {0} # lambda is type checked here, before running type inference on
             # the `example` call below.
example(f) do # call to `example` is typechecked next
  1 # block body is typechecked last
end
```

Since the definition of the lambda `f` is typechecked entirely before the call
to `example` is typechecked, no `.bind` annotation on any of `example`'s
positional or keyword arguments would actually affect how the lambda is
typechecked.

## 5072

See [`type_member` & `type_template`](generics.md#type_member--type_template) in
the generics docs for more.

A `type_member` is limited in scope to only appear in instance methods of the
given class. A `type_template` is limited in scope to only appear in singleton
class methods of the given class. For example:

```ruby
class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig {returns(Elem)} # error
  def self.example
    # ...
  end
end
```

This example doesn't make sense because `Elem` in this case is the element type
of our generic `Box` type. For example, instances of `Box[Integer]` and
`Box[String]` hold `Integer`'s and `String`'s respectively. Meanwhile, a call to
the method `Box.example` wouldn't have a meaning, because there is no instance
of `Box` in this call to `Box.example` whose element type should be used.

Sometimes this error comes up when attempting to do something like this:

```ruby
class AbstractSerializable
  extend T::Sig
  extend T::Generic
  abstract!
  SerializeType = type_template

  sig {abstract.returns(SerializeType)} # error
  def serialize; end
  sig do
    abstract
      .params(raw_input: SerializeType)
      .returns(T.attached_class)
  end
  def self.deserialize(raw_input); end
end

class MyClass < AbstractSerializable
  SerializeType = type_template {{fixed: String}}

  # ... implement abstract methods ...
end
```

In this case, the idea is that some subclasses may want to serialize to a
`String`, some may want to serialize to an `Integer`, some may want to serialize
to a `Symbol`, etc. and the child class should get to specify that, by using
`fixed` on the `type_template`.

Even in these cases, Sorbet does not let the `type_template` be referenced from
an instance method. The solution is to instead specify both `type_member` and a
`type_template`, and have the child class fix both of them:

```ruby
class AbstractSerializable
  # ...
  SerializeTypeMember = type_member
  SerializeTypeTemplate = type_template
  # ...
end

class MyClass < AbstractSerializable
  SerializeTypeMember = type_member {{fixed: String}}
  SerializeTypeTemplate = type_template {{fixed: String}}
  # ...
end
```

## 5073

Abstract classes cannot be instantiated by definition. See
[Abstract Classes and Interfaces](abstract.md) for more information.

```ruby
class Abstract
  extend T::Sig
  extend T::Helpers
  abstract!

  sig {abstract.void}
  def foo; end
end

Abstract.new # error: Attempt to instantiate abstract class `Abstract`
```

To fix this error, there are some options:

- If the class which is marked `abstract!` does not actually have any `abstract`
  methods, simply remove `abstract!` from the class definition to fix the error.
- If the class _does_ have `abstract` methods, find some concrete subclass to
  call `new` on instead. If the call to `new` is in a test file, you may wish to
  make a new, test-only subclass of the abstract class. (Depending on the
  specifics of the test, it may even be possible to simply define all the
  abstract methods to simply `raise`, so that other aspects of the parent class
  can be tested.)

## 5074

A module marked `has_attached_class!` can only be mixed into a class with
`extend`, or a module with `include`. When mixing a `has_attached_class!` module
into another module, both modules must declare `has_attached_class!`.

For more information, see the docs for [`T.attached_class`](attached-class.md).

## 6001

Certain Ruby keywords like `break`, `next`, and `retry` can only be used inside
a Ruby block.

## 6002

In `# typed: strict` files, Sorbet requires that all instance and class
variables are annotated with a `T.let`.

For how to fix, see [Type Annotations](type-annotations.md).

See also: [5028](#5028), [7017](#7017), [7028](#7028), [7043](#7043).

## 6004

In order to statically check [exhaustiveness](exhaustiveness), Sorbet provides
`T.absurd`, which lets people opt into exhaustiveness checks.

`T.absurd` must be given a variable. If not, like in this example, it reports an
error:

```ruby
# -- bad example --

sig {returns(T.any(Integer, String))}
def returns_int_or_string; 0; end

case returns_int_or_string
when Integer then puts 'got int'
when String then puts 'got string'
# error! `returns_int_or_string` is not a variable!
else T.absurd(returns_int_or_string)
end
```

While it looks like `returns_int_or_string` is the name of a variable, it's
actually a method call (Ruby allows method calls to omit parentheses). To fix
this error, store the result of calling `returns_int_or_string` in a variable,
and use that variable with the `case` and `T.absurd`:

```ruby
sig {returns(T.any(Integer, String))}
def returns_int_or_string; 0; end

# calls returns_int_or_string, stores result in x
x = returns_int_or_string

case x
when Integer then puts 'got int'
when String then puts 'got string'
else T.absurd(x)
end
```

## 6005

`T.bind` can only be called on `self`, syntactically.

To perform type assertions on non-`self` things, use `T.let` or `T.cast`.

See [Type Assertions](type-assertions.md) for more.

## 6006

The program attempted to use `T.type_parameter` in a method body, but there was
no such `T.type_parameter` in scope.

For more information, see the docs for
[Generic methods](generics.md#generic-methods).

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

```ruby
sig { params(name: String, category: Category).void }
def publish_item(name, category = nil) # error: Argument does not have asserted type `Category`
  # ...
end
```

If `category` value is `nil` by default, maybe we should make it so its type is
nilable:

```ruby
sig { params(name: String, category: T.nilable(Category)).void }
def publish_item(name, category = nil)
  # ...
end
```

## 7009

This error occurs when a value is used in place of a type. There are many
different situations where this can happen; one example is given below:

```ruby
class Box
  extend T::Generic

  E = type_member
end

Box[true].new # error: Unexpected bare `TrueClass` value found in type position
```

More generally, Sorbet draws a distinction between places in a program where
Ruby values are allowed, and places where Sorbet type syntax is allowed. For
example:

```ruby
# ----- Arbitrary type syntax allowed -----
T.let(0, T.any(Integer, String))
#        ^^^^^^^^^^^^^^^^^^^^^^
sig {returns(T.any(Integer, String))}
#            ^^^^^^^^^^^^^^^^^^^^^^
T::Array[T.any(Integer, String)].new
#        ^^^^^^^^^^^^^^^^^^^^^^


# ----- Arbitrary type syntax NOT allowed -----

case 0
when T.any(Integer, String)
#    ^^^^^^^^^^^^^^^^^^^^^^
end

x.is_a?(T.any(Integer, String))
#       ^^^^^^^^^^^^^^^^^^^^^^
```

Only valid Sorbet types are allowed in type positions. Arbitrary Sorbet types
are not allowed in most places where Ruby expects a normal value.

> **Note**: Historically this error message has been one of the more confusing
> Sorbet errors, and over time we have added special cases to detect common
> points of confusion. If you're reading this because you found the error
> message confusing, please consider sharing your example with the Sorbet team
> so we can further improve the error message.

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

## 7011

Historically, users seeing this error has represented finding a bug in Sorbet
(or at least finding a test case for which there could be a better error
message). Consider searching for similar bugs at
<https://github.com/sorbet/sorbet/issues> or reporting a new one.

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
@x = T.let(0, T.any(Integer, String))
```

Similarly, Sorbet will reject constants reassigned with different types:

```rb
FOO = 42 # error: Expected `String("Hello, world!")` but found `Integer(42)` for field
FOO = "Hello, world!"
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

## 7015

Certain forms of `T.let`, `T.cast`, and `T.must` calls are redundant. Follow the
suggestion in the error message to fix—these errors should have an autocorrect
you can apply to automatically fix the error.

## 7016

Sorbet has special handling for certain methods in the standard library, like
`Array#flatten`. This method takes an optional `depth` argument, and will only
flatten nested arrays that deep if passed:

```ruby
[[[[1]]]].flatten(2)

# ERROR
depth = T.let(2, Integer)
[[[[1]]]].flatten(depth)
```

Unfortunately, Sorbet can only perform this analysis when the depth is static.
If Sorbet cannot see the exact value of the depth and instead only sees a type
of `Integer` for the depth argument, it reports an error.

Either:

1.  Refactor the code so that Sorbet can see the exact depth value, or
2.  Use `T.unsafe` to hide the method call from Sorbet.

```ruby
depth = T.let(2, Integer)
# `T.unsafe` disables static type checking for this call site,
# including the flatten depth check
T.unsafe([[[[1]]]]).flatten(depth)
```

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

See also: [5028](#5028), [6002](#6002), [7028](#7028), [7043](#7043).

## 7018

At `# typed: strong`, Sorbet no longer allows using `T.untyped` values. To fix
errors of this class, add type annotations to the code until Sorbet has enough
context to know the static type of a value. Usually this means adding
[method signatures](sigs.md) or [type assertions](type-assertions.md) to declare
types to Sorbet that it couldn't infer.

**Note**: this strictness level should be considered a beta feature: the errors
at this level are still being developed. Most Ruby files that actually do
interesting things will have errors in `# typed: strong`. As such, an
alternative solution to fixing these errors is simply to downgrade the file to
`# typed: strict` or below, which will silence all these `T.untyped` errors.

For more information on `# typed: strong`, strategies for dealing with errors
that arise from using `T.untyped`, and current known limitations, see the docs
for [`# typed: strong`](strong.md).

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

## 7020

Note that method-level generics (i.e., those declared by using `type_parameters`
inside a `sig`) are a somewhat unstable feature. If you encounter this error and
believe it to represent a bug in Sorbet, first check the
[Sorbet bug tracker](https://github.com/sorbet/sorbet/issues) to see if a
similar-looking error has already been reported. If no such bug exists, feel
free to open an issue.

## 7021

The called method declares a block parameter that is not `T.nilable` (making the
block argument required), but a block was not passed when it was called.

This can be fixed by either passing a block to the method, or changing the
method's signature for the block parameter from `T.proc...` to
`T.nilable(T.proc...)` (and then changing the method to deal with a nilable
block parameter).

## 7022

When run with the `--suggest-typed` command line argument, Sorbet will suggest
upgrading or downgrading `# typed:` sigils in all files in the project to the
highest level possible that would still have no errors. If the project starts
off with no errors initially, this should have the effect of only upgrading
sigils.

Accept the provided autocorrect suggestion to commit the upgrades (using the
`--autocorrect` flag).

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

## 7027

In `# typed: strict` files, Sorbet requires that all constants are annotated
with a `T.let`.

For how to fix, see [Type Annotations](type-annotations.md), or accept the
autocorrect suggestion associated with this error.

See also: [5028](#5028), [6002](#6002), [7017](#7017), [7043](#7043).

## 7030

This error is consistently used when the user is trying (implicitly or
explicitly) to call some method on a Sorbet type (e.g. `T::Array[Integer]`)
which would actually return a Sorbet-runtime representation of a type.

This error generally occurs when generic types are used in pattern matching:

```ruby
def get_value(input)
  case input
  when Integer
    input
  when T::Array[Integer] # error: Call to method `===` on `T::Array[Integer]` mistakes a type for a value
    input.first
  end
end
```

Since [generic types are erased](generics.md#generics-and-runtime-checks) at
runtime, this construct would never work when the program executed. Replace the
generic type `T::Array[Integer]` by the erased type `Array` so the runtime
behavior is correct:

```ruby
def get_value(input)
  case input
  when Integer
    input
  when Array
    input.first
  end
end
```

## 7031

Private methods in Ruby behave somewhat differently from private methods in
other statically typed languages. You may want to read more about
[method visibility in Ruby](https://www.rubyguides.com/2018/10/method-visibility/)
first. The tl;dr is that private methods can only be called when the receiver is
**syntactically** `self` (or omitted):

```ruby
class Parent
  private def foo; end

  def in_parent
    foo      # OK
    self.foo # OK
  end
end

class Child < Parent
  def in_child
    foo      # OK
    self.foo # OK
  end
end

Parent.new.foo # error!
Child.new.foo  # error!
```

This makes private methods in Ruby behave more like protected methods in other
object-oriented languages.

To fix this error, either:

1.  Avoid calling the private method entirely, or
2.  Update the method definition to not be private, or
3.  Use `.send(:foo, ...)` to ask Ruby to call the method ignoring visibility
    checks.

> **Note**: It's not possible to silence this error using `T.unsafe`. Wrapping
> the receiver in `T.unsafe` will in fact hide the method call from Sorbet
> statically, but because method visibility is checked **syntactically**, even
> using `T.unsafe(self).foo` will cause a call to a private method `foo` to be
> rejected by the Ruby VM at runtime.
>
> If you must call a private method and also silence any type errors from
> calling it, you must use `self.send(:foo)` instead.

## 7032

When passing type arguments to generic classes, to use [shape types](shapes.md),
use curly brackets around the keys and values of the shape type:

```ruby
# CORRECT
T::Array[{key: Integer}].new

# BAD
T::Array[key: Integer].new
```

## 7033

**Note**: this error is only reported when Sorbet is passed the
`--ruby3-keyword-args` command line flag. For further information about Ruby 3
and keyword args, see the
[official blog post](https://www.ruby-lang.org/en/news/2019/12/12/separation-of-positional-and-keyword-arguments-in-ruby-3-0/).

```ruby
# typed: true
extend T::Sig

sig {params(x: Integer, y:Integer).void}
def takes_kwargs(x, y:)
end

arghash = {y: 42}

# GOOD EXAMPLE
takes_kwargs(99, **arghash)

# BAD EXAMPLE
takes_kwargs(99, arghash)
#                ^^^^^^^ error: Keyword argument hash without `**`
```

[→ View on sorbet.run](https://sorbet.run/?arg=--ruby3-keyword-args#%23%20typed%3A%20true%0Aextend%20T%3A%3ASig%0A%0Asig%20%7Bparams%28x%3A%20Integer%2C%20y%3AInteger%29.void%7D%0Adef%20takes_kwargs%28x%2C%20y%3A%29%0Aend%0A%0Aarghash%20%3D%20%7By%3A%2042%7D%0Atakes_kwargs%2899%2C%20arghash%29)

In Ruby 2.7 and earlier, Ruby allowed a positional `Hash` argument at the end of
a method call's list of arguments to implicitly splat into the keyword
parameters of the method. In Ruby 3.0 and later, a positional `Hash` argument is
always treated as a positional argument. To avoid this error, prefix the `Hash`
argument with `**`, explicitly requesting to treat it as keyword arguments.

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

## 7035

Sorbet can sometimes detect when a method is passed a block despite not
accepting a block.

```ruby
# typed: strict
extend T::Sig

sig {void}
def takes_no_block; end

# BAD EXAMPLE
takes_no_block {} # error: does not take a block
```

Why only sometimes? Technically all methods in Ruby are allowed to accept
blocks. (Unlike positional arguments, if a method does not declare that it
accepts a block argument but a caller passes one anyways the Ruby VM does not
raise an exception.)

For historical reasons, Sorbet did not require that a `sig` mention the `blk`
parameter if its method used `yield` from the beginning of Sorbet adoption. It
later required this, but for reasons of backwards compatibility, it's only
checked in `# typed: strict` or higher files.

Therefore, error 7035 is somewhat special in that it can be reported in
`# typed: true` files, but **only** if the method being passed a block is itself
defined in a `# typed: strict` file.

Regardless, to fix this error, either:

1.  Remove the block from this call site (using the autocorrect), or
2.  Update the called method's definition to mention block argument, or
3.  Drop the strictness level of file containing the called method's definition
    to `# typed: true` or lower (**only** as a **last resort**).

## 7036

Sorbet supports declaring package-private methods. These methods can only be
called from within the package where they are declared

To fix this error, either remove the `package_private` (or
`package_private_class_method`) annotation from the method definition, or
rewrite the code in question to not call the private method.

As a **last** resort, you can use `T.unsafe` to hide the method call from
Sorbet, silencing the error.

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
Sorbet behaves this way, and we do not foresee these reasons changing.)

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

For more information, see the docs on
[generic methods](generics.md#generic-methods).

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

To allow code like this, use [interfaces](abstract.md) with
[intersection types](intersection-types.md):

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

Sometimes this error happens due to a call to `is_a?` on a type parameter. To
sidestep this error, rewrite the program to use `case`:

```ruby
# ...
def example(x)
  if x.is_a?(Integer) # error
    # ...
  end

  case x
  when Integer # OK
    # ...
  end
end
```

Or if it's imperative to continue using `is_a?`, change the type to
`T.all(Kernel, T.type_parameter(:U))`.

## 7039

For more information, see how to place
[bounds on type members](generics.md#bounds-on-type-member-s-and-type-template-s-fixed-upper-lower).

Consider this example:

```ruby
class A
  def only_on_a; end
end

class Box
  extend T::Sig
  extend T::Generic
  Elem = type_member

  sig {params(x: Elem).void}
  def initialize(x)
    x.only_on_a # error, see below for fix
  end
end
```

In this example, we're trying to to call the method `only_on_a`, but we haven't
specified any type bounds on the `Elem` type parameter, which means we can't
make any assumption about what methods it might have.

If we want to always be able to call the method `only_on_a`, we can place a
upper bound on `Elem`:

```ruby
# ...
  Elem = type_member {{upper: A}}
# ...
```

This will guarantee that `Elem` is always at least `A`, which will let Sorbet
allow the call to `x.only_on_a`.

Sometimes this error happens due to a call to `is_a?` on a type member. To
sidestep this error, rewrite the program to use `case`:

```ruby
# ...
def example(x)
  if x.is_a?(Integer) # error
    # ...
  end

  case x
  when Integer # OK
    # ...
  end
end
```

Or if it's imperative to continue using `is_a?`, change the type to
`T.all(Kernel, Elem)` and/or add an upper bound of `Kernel` to the type member.

## 7040

`T.attached_class` is a type annotation that refers to instances of the current
singleton class. For example, a method like this makes sense, because it uses
`T.attached_class` on a singleton class method (the `self.` prefix):

```ruby
class A
  sig {returns(T.attached_class)}
  def self.make
    x = T.let(new, T.attached_class)
  end
end
```

Meanwhile, this snippet doesn't make sense, because `foo` is already an instance
method—there is no attached class to speak of for non-singleton classes:

```ruby
class A
  sig {returns(T.attached_class)} # error!
  def foo
    x = T.let(new, T.attached_class) # error!
  end
end
```

For more information see the [`T.attached_class`](attached-class.md) docs.

It may also be interesting to compare and contrast
[`T.self_type`](self-type.md).

## 7043

In `# typed: strict` files, Sorbet requires that all instance and class
variables are annotated with a `T.let`.

For how to fix, see [Type Annotations](type-annotations.md).

See also: [5028](#5028), [6002](#6002), [7017](#7017), [7027](#7027).

## 7044

Sorbet has built-in support for the `dig` method on `Array` and `Hash`. In
certain cases, Sorbet can detect that there have been too many arguments
provided to a `dig` call. For example:

```ruby
arr = T::Array[NilClass].new
arr.dig(0, 0)
```

This tries to get the 0<sup>th</sup> element of the 0<sup>th</sup> element of
the array, if it exists.

However, Sorbet can know that the 0<sup>th</sup> element of the first array is
always `nil` if it exists, so the second 0<sup>th</sup> element will never be
accessed, and is thus redundant.

To fix this, either delete the redundant arguments, or use an
[Escape Hatch](troubleshooting.md#escape-hatches) to hide the call to `dig` from
Sorbet:

```ruby
arr = T::Array[NilClass].new
T.unsafe(arr).dig(0, 0)
```

## 7045

Sorbet sometimes assumes an expression has a certain type—even when it has no
guarantee whether that's the case—because the assumption will be correct almost
all the time and assuming the type means not having to given an explicit type
annotation.

This error is reported when those assumptions are wrong. Rather than go back and
attempt to invalidate the assumption by redoing work it already did (but this
time under the correct assumptions), it reports an error asking the user to
provide an explicit type annotation so that no assumption is necessary in the
first place. This enables Sorbet to finish type checking quickly on large
codebases.

To fix this error, provide an explicit annotation (or simply accept the
[autocorrect suggestion](cli.md#accepting-autocorrect-suggestions)).

For more information, read
[Why does Sorbet sometimes need type annotations?](why-type-annotations.md).

## 7046

For a limited number of types, Sorbet checks whether it looks like a call to
`==` is out of place. Currently, Sorbet only does these checks when the left
operand of `==` is:

- `Symbol`
- `String`

Sorbet is unable to apply these checks for all types, because `==` can be
overridden in arbitrary ways, including to allow for implicit conversion between
unrelated types. This means that Sorbet will sometimes miss reporting this error
in places where we would like it to, and can't be changed to report an error
without breaking valid code.

To fix this error, ensure that the left and right operands' types match before
doing the comparison. For example, try converting `String`s to `Symbol`s with
`to_sym` (or vice versa with `to_s`).

## 7047

This error code is an implementation detail of Sorbet's "highlight untyped in
editor" mode. It indicates that the given piece of code has type
[`T.untyped`](untyped.md). Untyped code can be dangerous, because it circumvents
the guarantees of the type system.

This feature is opt-in. See [VS Code](vscode.md) for instructions on how to turn
it on.

<!-- -->

[report an issue]: https://github.com/sorbet/sorbet/issues

<script src="/js/error-reference.js"></script>
