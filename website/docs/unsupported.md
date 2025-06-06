---
id: unsupported
title: Unsupported Ruby Features
---

Some features of Ruby Sorbet intentionally does not support. This doc aims to document the most common such features with explanations and alternatives.

> **Note**: This page is not exhaustive.
>
> If something is missing from this page, it says nothing about whether Sorbet supports it, supports it but has a bug in the implementation, or does not ever intend to support it.
>
> When in doubt, please [open an issue](https://github.com/sorbet/sorbet/issues/new/choose).

## Dynamic constant references

```ruby
class A
  X = 42
end

class B
  X = 'hello'
end

def takes_class(cls)
  p(cls::X)
end
takes_class(A)
takes_class(B)

def takes_inst(val)
  p(val.class::X)
end
takes_inst(A.new)
takes_inst(B.new)
```

### Alternative

This pattern amounts to dynamic dispatch, so it's better to use methods.

```ruby
class A
  def self.x = 42
end

class B
  def self.x = 'hello'
end

def takes_class(cls)
  p(cls.x)
end
takes_class(A)
takes_class(B)

def takes_inst(val)
  p(val.class.x)
end
takes_inst(A.new)
takes_inst(B.new)
```

You can even get clever with this approach: methods can start with a capital letter (like constants) as long as it's clear from context that it's a method call, not a constant reference. So it's possible to keep the constant's original `X` name as long as you use one of these forms to call it: `X()`, `self.X`, `cls::X()`, etc. This can help minimize the size of the edit when refactoring code to not use constants.

As a last resort, if you absolutely cannot refactor the code to use methods (which can then be typed), you can use `const_get` instead:

```ruby
cls.const_get(:X)
```

Sorbet treats all calls to `const_get` as untyped.

### Why?

There are three main reasons.

1.  It breaks a cyclic dependency in the implementation of the type checker.

    Sorbet resolves all constant definitions and their ancestors to be able to understand the type of an expression. Sorbet cannot know the type of an expression like `x.y::Z` until it has first resolved the type of `x.y`, which requires knowing the type of `x`, its full class hierarchy, the methods defined in that hierarchy, and their types. All of that information requires knowing which constants are defined, which thus cannot require knowing the types of arbitrary expressions.

    Sorbet could technically allow dynamic constant references in positions where they have no impact on the ancestor hierarchy or type resolution (basically, inside method bodies excluding in type assertions like `T.let`). This would still exclude their use in superclasses, `include`, `extend`, `T.type_alias`, `sig` types, and types of constants.

1.  It would require treating constants like methods anyways.

    This mode of use of constants acts like dynamic dispatch. Specifically, Sorbet would need to validate the same [override checking](override-checking.md) constraints that it validates for methods, meaning that Sorbet would need `overridable`/`override` syntax for constants (and likely also `abstract`).

    Stylistically, constants should be for static information, and methods should be the solution for dynamic dispatch.

1.  It simplifies all other constant handling inside Sorbet.

    Sorbet can assume that the scope of constant literal is always another constant literal (or nothing). This invariant substantially simplifies all other code that reads and manipulates constant references in the codebase—no need to defensively handle arbitrary code.

## Constant resolution through constant scopes via inheritance

```ruby
class Parent
  X = 1
end

class Child < Parent
  p(X)  # ✅ okay in both
end

p(Child::X)  # in Ruby   => ✅ 1
             # in Sorbet => ❌ error
```

Sorbet does not support constant resolution through inheritance when given an explicit scope.

### Alternative

Use `Parent::X` instead of `Child::X`

### Why?

- Performance

  Constant resolution is one of the most performance sensitive parts of Sorbet.

- Understandability

  In this case, the code is easier to understand by simply replacing `Child::X` with `Parent::X`. This can always be done because Sorbet does not support dynamic constant references, so the scope is always known statically.

## `prepend`

```ruby
module WillBePrepended
  def foo
    puts 'WillBePrepended#foo'
  end
end

class Example
  prepend WillBePrepended
  def foo
    puts 'Example#foo'
  end
end

Example.new.foo # => WillBePrepended#foo
```

Sorbet does not model inheritance relationships introduced by `prepend`.

### Alternative

- Usually, static support for `prepend` is not required, even in codebases that make heavy use of `prepend`.
- In the rare cases where `prepend` is required, we recommend using [escape hatches](troubleshooting.md#escape-hatches) to work around the problems.

### Why?

- Support for `prepend` would force Sorbet to use more memory throughout an entire codebase, even if the codebase makes no use of `prepend`. Usage of `prepend` is far more rare than the cost it would inflict in terms of memory.

- Supporting `prepend` would add implementation complexity to Sorbet's internals. For example: consider how to do [Override Checking](override-checking.md) and [generic bounds checking](generics.md#bounds-on-type_members-and-type_templates-fixed-upper-lower) in the presence of prepended modules.

- Historically, Sorbet was developed at Stripe, which lints against usage of `prepend`.

- Historically and maybe still today: `sorbet-runtime` had (has?) poor support for runtime-checked type annotations on methods defined with prepended modules.

## Refinements and `refine do`

```ruby
class C
  def foo
    puts "C#foo"
  end
end

module M
  refine C do
    def foo
      puts "C#foo in M"
    end
  end
end

using M

c = C.new

c.foo # prints "C#foo in M"
```

Sorbet does not support refinements.

### Alternatives

- Use an RBI file to define the methods. Sorbet will assume that the methods are defined everywhere, not just where the `using` directive lives. This means that Sorbet will not reject code that would not have caused problems at runtime, at the expense of not catching situations that might have.

- Use an [Escape Hatch](troubleshooting.md#escape-hatches).

- Use a monkey patch.

### Why?

- While refinements are better than monkey patching, they still amount to monkey patching. Sorbet's role as a type checker is not only to catch errors, but to steer people towards simpler designs.

- Historically, Sorbet was developed at Stripe, which does not use refinements.

- Support for refinements would add implementation complexity to Sorbet.

- Supporting refinements would require doing program-wide work to discover and use refinements even if a codebase does not use them at all, which comes with a performance cost.

## Creating method aliases to methods in parent classes

```ruby
class Parent
  def defined_in_parent; end
end

class Child < Parent
  alias_method :defined_in_child, :defined_in_parent
  # Sorbet thinks this method doesn't exist ^
end
```

Sorbet does not support aliasing to a method defined in a parent class from a child class.

### Alternative

1.  Override the parent method in the child, and have the implementation just call `super`:

    ```ruby
    class Parent
      def defined_in_parent; end
    end

    class Child < Parent
      def defined_in_parent
        super
      end

      alias_method :defined_in_child, :defined_in_parent
    end
    ```

1.  Use [RBI files](rbi.md) to define the methods that would be defined this way:

    ```ruby
    # -- foo.rb --
    class Parent
      def defined_in_parent; end
    end

    class Child < Parent
      # Hide the alias_method call from Sorbet to silence the error
      T.unsafe(self).alias_method :defined_in_child, :defined_in_parent
    end

    # -- foo.rbi --
    class Child < Parent
      # Define the method that will be defined with `alias_method` at runtime
      def defined_in_child; end
    end
    ```

1.  Use an [Escape Hatch](troubleshooting.md#escape-hatches) to silence errors at call sites.

### Why?

Due to original design decisions made in Sorbet's architecture, all methods are defined before inheritance information is resolved.

There is nothing fundamental or performance sensitive which requires making this choice (i.e., resolving ancestor information does not require knowing the set of defined methods). But backing out this design decision at this point would require more work than we currently believe the payoff is.

Because this is not a fundamental nor ideological limitation, it's possible this feature may gain support in the future.

## Multi-line calls to to keyword-named methods with trailing `.`

```ruby
# 1. single-line method call
x.end() # ok

# 2. multi-line method call, leading `.`
x
  .end() # ok

# 3. multi-line method call, trailing `.`
x.
  end() # not ok
```

Ruby allows methods to be defined with names that are nominally reserved for keywords—like the method called `end()` above, even though there is a keyword called `end`.

For methods which share a name with a Ruby keyword, Sorbet does not allow a newline to appear between the `.` token and the method name.

### Alternative

Use a leading `.` for chained multi-line method calls, instead of a trailing `.`.

Note: newer versions of `irb` and `pry` support the leading `.` syntax about as well as the trailing `.` syntax. In old versions of `irb` and `pry`, the REPLs did a poor job of detecting multi-line pastes, and would eagerly evaluate each line instead of waiting for the full paste and evaluating the entire snippet. Newer versions of `irb` and `pry` detect the terminal emulator's [bracketed paste](https://github.com/ruby/irb/commit/45aeb52575) functionality and pause evaluation until the paste finishes.

### Why?

One of the most common syntax errors in a Ruby program looks like this:

```ruby
def example(x)
  x.
end
```

At a glance, it looks like the syntax error is that the user has forgotten or is in the process of typing the method name after the `x.`. But to the Ruby parser, the method name **was** provided—it's a method named `end`. Instead, the syntax error the Ruby parser sees is that the user forgot to terminate their method definition with an `end` keyword after the last line.

In order to make error messages and autocompletion suggestions better, Sorbet reverts to treating the characters `end` as a keyword, not a method name, after it sees a sequence of `.` followed by `\n`. This is a small change to the Ruby grammar with a small cost to implement, for a large improvement in developer ergonomics when working in an IDE, with a straightforward workaround.

Note that this applies to **all** keywords, not just `end`. For a complete list of Ruby keywords, see [the Ruby docs](https://docs.ruby-lang.org/en/master/keywords_rdoc.html).

For more, see [#1993](https://github.com/sorbet/sorbet/pull/1993).

## Tracking code loading order

```ruby
# -- a.rb --
class A; end

# -- b.rb --
# ... does not require/autoload `a.rb` ...

puts(A) # => in Ruby: ❌ NameError
        # => in Sorbet: ✅
```

Sorbet does not track `require`, `require_relative`, or `autoload` declarations.

This means that Sorbet does not report errors for these errors or warnings from the Ruby VM:

- Accessing constants that are defined somewhere in the project but haven't been loaded yet when the code runs.
- Reassigning a constant (`X = 1; X = 2`), which are warnings in the Ruby VM, so long as the declared type of the constant is the same in both definitions.
- Redefining a method (`def f; end; def f; end`), so long as the arity of the method is the same in both definitions
- Accessing an instance variable before it's been initialized.
- _etc._

### Alternative

Use runtime code loading mechanisms (e.g. tests or other runtime checks) to make sure that code can be loaded, possibly opting into more verbose warning checking in the Ruby VM.

### Why?

- Different projects use different code loading paths for gems. Rather than have Sorbet reimplement the algorithm Ruby/rbenv/rvm/etc. use to load gems out of system directories, Sorbet requests that all gems are declared with RBI files included in the args specified at the command line.

- Certain require statements will be computed dynamically, either behind `if`/`else` expressions, inside method calls, or even with non-static string arguments. Sorbet cannot analyze these—the problem would reduce to having Sorbet statically evaluate Ruby code.

- Many projects, especially Rails projects, use a path-based autoloader, like [zeitwerk](https://github.com/fxn/zeitwerk). Projects using code loaders like this typically do not make their `autoload` statements visible to Sorbet at all: the `autoload` statements are dynamically generated by the project at runtime.

- Sorbet does not track whether or in what order code loads. For example, a project might define two versions of a file: one which is loaded on old versions of Ruby, one which is loaded on newer versions of Ruby. The files might define the same classes and methods, but with different implementations. The project itself knows that at runtime only one of these will be loaded, but there would be no way to indicate that to Sorbet.
