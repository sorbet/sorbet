---
id: minitest
title: Minitest's Spec DSL
sidebar_label: minitest
---

Sorbet has special support for type checking [minitest]-defined tests that use `describe` and `it` blocks:

[minitest]: https://github.com/minitest/minitest

minitest is a Ruby gem for writing tests that ships by default with the Ruby VM. It looks like this:

```ruby
require 'minitest/autorun'

class MyClass
  def greet = 'hello'
end

class MyTest < Minitest::Spec
  describe 'my class' do
    it 'works' do
      assert_equal('hello', MyClass.new.greet)
    end
  end
end
```

## A note on typing tests

When adopting Sorbet in a codebase, don't agonize over getting all tests to `# typed: true` at first.

Tests provide their own kind of confidence: simply running the tests confirms whether there are errors in the tests or not. In essence, most test files themselves have 100% test coverage, because by definition the tests run all the time.

By contrast, it's much higher value to prioritize adopting type checking in non-test files. Non-test files might not actually be covered by tests, having behavior that only gets invoked in production.

There is definitely value in typing test (e.g., it will [enable IDE features](lsp-typed-level.md)), but typing tests are often **harder** than typing non-test code, because of the DSLs involved in defining tests.

The rest of this doc should help once you've decided that typing tests is worth the investment.

## Spec methods

Sorbet understands various minitest spec methods. These spec methods are _syntactically_ translated to other Ruby constructs internally before type checking a file to help Sorbet understand that file's structure.

- `describe 'something' { ... }`

  `describe` blocks are translated to Ruby classes. They have access to methods defined in the enclosing class where the `describe` block is defined:

  ```ruby
  def foo; end

  describe 'my test' do
    it 'calls foo' do
      foo
    end
  end
  ```

- `it 'something' { ... }`

  `it` blocks are translated to Ruby instance methods. When inside a `describe`, they have access to the methods described inside all the enclosing `describe` blocks, and the enclosing class.

- `before { ... }` / `after { ... }`

  `before` and `after` blocks are also translated to instance methods, so they have access to instance methods defined in the enclosing context.

  **Also**, `before { ... }` blocks are given special treatment: instance variables declared inside `before` blocks behave like those [defined inside `initialize` constructors]: they are allowed to be non-nilable.

- `let(:foo) { ... }`

  `let`-defined helper methods are translated to the corresponding methods.

  ```ruby
  let(:foo) { 42 }

  it 'calls foo' do
    assert_equal(42, foo)
  end
  ```

[defined inside `initialize` constructors]: type-annotations.md#declaring-class-and-instance-variables

**Note**: Sorbet only looks for spec methods at the top-level of a class body or `describe` block. This mostly matters for [table driven tests](#table-driven-tests-tests-defined-with-each), but can also matter when defining tests using project-specific test metaprogramming helpers.

## Table-driven tests: tests defined with `each`

A common pattern in certain kinds of tests is to try to run the same test over multiple values:

```ruby
def handle_val(val)
  # ...
end

VALUES.each do |val|
  it "works for #{val}" do
    handle_val(val) # 💥 Method `handle_val` does not exist
  end
end
```

Tests written in this fashion defeat Sorbet's support minitest spec methods: Sorbet only looks for minitest spec methods at the top-level of a class body.

To type patterns like this, Sorbet requires projects to define a method called `test_each`, and rewrite the call to `each` with a call to `test_each`:

```ruby
sig do
  type_parameters(:U)
    .params(iter: T::Enumerable[T.type_parameter(:U)], blk: T.proc.params(arg0: T.type_parameter(:U)).void)
    .void
end
def self.test_each(iter, &blk)
  iter.each(&blk)
end

def handle_val(val)
  # ...
end

test_each(VALUES) do |val|
  it "works for #{val}" do
    handle_val(val) # ✅ Method exists
  end
end
```

Only other minitest spec methods can appear at the top-level of the `test_each` block.

Sorbet provides `test_each` because type-checking this in the general case is quite complicated. If you try using `my_expression.each` instead, then this will work at runtime but Sorbet might report anomalous static errors related to method and variable scoping.

`test_each` is not perfect—Sorbet will only be able to infer a type for the `x` variable in the block parameter when the argument to `test_each` is **syntactically** an array, not when it is some arbitrary expression that evaluates to an array.

Sorbet also has support for iterating over `Hash` literals:

```ruby
sig do
  type_parameters(:K, :V)
    .params(
      hash: T::Hash[T.type_parameter(:K), T.type_parameter(:V)],
      blk: T.proc.params(arg0: [T.type_parameter(:K), T.type_parameter(:V)]).void
    )
    .void
end
def self.test_each_hash(hash, &blk)
  hash.each(&blk)
end

def handle_val(key, val)
  # ...
end

test_each(KEY_VAL_PAIRS) do |key, val|
  it "works for #{key}=#{val}" do
    handle_val(key, val) # ✅ Method exists
  end
end
```

For more on correct and incorrect usage, consider reading the error documentation for [error code 3507](error-reference.md#3507).

### `test_each` and `test_each_hash` definitions

These definitions can either:

- be put in a module that is mixed into (`extend`'d into) the tests that want to use them
- be put in a shared test base class that the project uses (e.g., a subclass of `Minitest::Spec`)
- be duplicated verbatim into any test that wants to use them

```ruby
sig do
  type_parameters(:U)
    .params(iter: T::Enumerable[T.type_parameter(:U)], blk: T.proc.params(arg0: T.type_parameter(:U)).void)
    .void
end
def self.test_each(iter, &blk)
  iter.each(&blk)
end

sig do
  type_parameters(:K, :V)
    .params(
      hash: T::Hash[T.type_parameter(:K), T.type_parameter(:V)],
      blk: T.proc.params(arg0: [T.type_parameter(:K), T.type_parameter(:V)]).void
    )
    .void
end
def self.test_each_hash(hash, &blk)
  hash.each(&blk)
end
```

## RSpec

Sorbet supports RSpec's testing DSL using the same spec method translation as Minitest. RSpec and Minitest share common constructs like `describe`, `it`, `before`, `after`, and `let`, which are all translated to Ruby classes and methods before type checking.

When Sorbet sees `RSpec.describe`, it translates this to a subclass of `RSpec::Core::ExampleGroup`:

```ruby
RSpec.describe MyClass do
  # ... body ...
end

# ^ becomes:

class made-up-class-name < RSpec::Core::ExampleGroup
  # ... translated body ...
end
```

This models what the `RSpec.describe` DSL will do at runtime. Ensure that there is a suitable RBI defining `RSpec::Core::ExampleGroup` (with any relevant monkey patches) contained in the project somewhere (either generated via `tapioca gem` or fetched from RBI Central).

```ruby
RSpec.describe MyClass do
  it 'works' do
    expect(MyClass.foo).to eq 'foo'
  end

  context 'nested contexts' do
    it 'works' do
      expect(MyClass.foo).to eq 'foo'
    end
  end

  describe 'nested describes' do
    it 'works' do
      expect(MyClass.foo).to eq 'foo'
    end
  end
end
```

The prefix is necessary for Sorbet to be able to support your customer RSpec matchers (see the RBI section below).

### Type signatures for RSpec methods

To provide better type checking for RSpec, define [RBI type signatures](rbi.md) for the RSpec methods used in the project. These signatures tell Sorbet about the available methods and their parameters.

The example below shows an abbreviated example. See [this gist](https://gist.github.com/alexevanczuk/dafa927f0fd18cf82608bd58bab66ec0)
 for a complete example that can be used to type a standard RSpec test suite. You may want to curate this list based on your usage of RSpec.

```ruby
# sorbet/rbi/manual/rspec.rbi

# typed: strict

module RSpec
  sig {params(args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.describe(*args, &example_group_block); end

  sig {params(args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.context(*args, &example_group_block); end

  module Core
    class ExampleGroup
      def array_including(*args); end

      sig { params(block: T.proc.params(arg0: RSpec::Core::ExampleGroup).bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def around(&block); end

      def expect(*args); end

      def allow(*args); end

      def instance_double(*args); end
    end
  end
end

```
