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

### Using RSpec.describe

To enable proper type checking with RSpec's custom matchers (like `eq`, `include`, `match_array`, etc.), use `RSpec.describe` instead of bare `describe` at the top level:
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

The `RSpec.` prefix ensures `RSpec::Core::ExampleGroup` is part of the ancestry chain of the translated classes.

### Type signatures for RSpec methods

To provide better type checking for RSpec, define [RBI type signatures](rbi.md) for the RSpec methods used in the project. These signatures tell Sorbet about the available methods and their parameters.

The example below shows common RSpec method signatures. You may want to curate this list based on your usage of RSpec.

```ruby
# sorbet/rbi/manual/rspec.rbi

# typed: strict

module RSpec
  sig {params(args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.describe(*args, &example_group_block); end

  sig {params(args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.xdescribe(*args, &example_group_block); end

  sig {params(args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.context(*args, &example_group_block); end

  sig {params(name: String, args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.shared_examples_for(name, *args, &example_group_block); end

  sig {params(name: String, args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).params(args: T.untyped).void).void}
  def self.shared_examples(name, *args, &example_group_block); end

  sig {params(name: String, args: T.untyped, example_group_block: T.proc.bind(Core::ExampleGroup).void).void}
  def self.shared_context(name, *args, &example_group_block); end

  def self.test_each(*args, &block); end

  module Core
    class ExampleGroup

      def run; end

      def call; end

      def to_proc; end

      def self.test_each(*args, &block); end

      def self.test_each_hash(*args, &block); end

      def test_each(*args, &block); end

      def include(*args); end

      def array_excluding(*args); end

      def array_including(*args); end

      def a_collection_containing_exactly(*args); end

      def be_an_instance_of(*args); end

      def an_instance_of(*args); end

      def hash_not_including(*args); end

      def satisfying(&blk); end

      def travel_to(*args, &block); end

      def within(*args); end

      def let(name, &block); end

      def let!(name, &block); end

      sig { returns(T.untyped) }
      sig { params(name: T.nilable(Symbol), block: T.proc.bind(T.untyped).void).returns(T.untyped) }
      def subject(name = nil, &block); end

      sig { params(name: T.nilable(String), block: T.nilable(T.proc.bind(RSpec::Core::ExampleGroup).void)).returns(T.untyped) }
      def its(name, &block); end

      sig { params(block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      sig { params(scope: Symbol, block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def before(scope = :each, &block); end

      sig { params(block: T.proc.params(arg0: RSpec::Core::ExampleGroup).bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def around(&block); end

      sig { params(scope: Symbol, block: T.proc.bind(T.untyped).void).returns(T.untyped) }
      def after(scope = :each, &block); end

      sig { params(name: String, type: T.nilable(Symbol), block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def xit(name, type: nil, &block); end

      sig { params(block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      sig { params(name: T.nilable(String), type: T.nilable(Symbol), block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def it(name = nil, type: nil, &block); end

      sig { params(block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      sig { params(name: T.nilable(String), type: T.nilable(Symbol), block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def self.it(name = nil, type: nil, &block); end

      def context(name, **metadata, &block); end

      def aggregate_failures(**options, &block); end

      def yield_control; end

      def pending(name, **metadata, &block); end

      def xcontext(name, &block); end

      def create(*args); end

      def attributes_for(*args); end

      def build_list(*args); end

      def create_list(*args); end

      def build_stubbed(*args); end

      def build(*args); end

      def expect(*args); end

      def allow(*args); end

      def instance_double(*args); end

      def class_double(*args); end

      def be_an(*args); end

      def output(*args); end

      def all(*args); end

      def instance_of(*args); end

      def allow_any_instance_of(*args); end

      def having_attributes(*args); end

      sig { params(name: String, block: T.nilable(T.proc.bind(RSpec::Core::ExampleGroup).void)).returns(T.untyped) }
      def include_context(name, &block); end

      sig { params(name: String, block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def shared_examples(name, &block); end

      sig { params(name: String, block: T.proc.bind(RSpec::Core::ExampleGroup).void).returns(T.untyped) }
      def shared_context(name, &block); end

      def it_behaves_like(*args); end

      def stub_const(*args); end

      def include_examples(*args); end

      def be(*args); end

      def eql(*args); end

      def eq(*args); end

      def equal(*args); end

      def start_with(*args); end

      def be_starts_with(*args); end

      def exceed_query_limit(*args); end

      def be_a_kind_of(*args); end

      def match_array(*args); end

      def be_within(*args); end

      def be_between(*args); end

      def is_expected;end

      def a_kind_of(*args); end

      def double(*args); end

      def be_valid; end

      def instance_double(*args); end

      def have_http_status(*args); end

      sig { params(name: T.nilable(T.any(T.any(String, Symbol), T::Array[T.any(String, Symbol)])), block: T.proc.bind(T.untyped).void).returns(T.untyped) }
      def its(name = nil, &block); end

      def expect(*args, &block); end

      def respond_to(*args); end

      def params; end

      def headers; end

      def have_key(*args); end

      def be_in(*args); end

      def be_falsey; end

      def be_instance_of(*args); end

      def assert_response(*args); end

      def expect_offense(*args); end

      def expect_correction(*args); end

      def expect_no_offenses(*args); end

      def an_object_having_attributes(*args); end

      def kind_of(*args); end

      def be_completed; end

      def a_value; end

      def be_kind_of(*args); end

      def env; end

      def expect_any_instance_of(*args); end

      def change(*args, &block); end

      def receive_message_chain(*args); end

      def not_change(*args); end

      def expect_successful_response(*args); end

      def any_args; end

      def anything; end

      def match_response(*args); end

      def have_attributes(*args); end

      def contain_exactly(*args); end

      def a_collection_containing_exactly(*args); end

      def have_received(*args); end

      def hash_including(*args); end

      def satisfy(*args); end

      def response; end

      def json; end

      def body; end

      def be_present; end

      def end_with(*args); end

      def be_persisted; end

      def a_string_matching(*args); end

      def be_empty; end

      def exist; end

      def be_nil; end

      def be_truthy; end

      def a_truthy_value; end

      def post(*args); end

      def request; end

      def head(*args); end

      def patch(*args); end

      def get(*args); end

      def delete(*args); end

      def a_hash_including(*args); end

      def a_string_including(*args); end

      def put(*args); end

      def receive(*args); end

      def raise_error(*args); end

      def raise_exception(*args); end

      def assert_equal(*args); end

      def be_falsy; end

      def match(*args); end

      def be_a(*args); end

      def shared_examples_for(*args); end
    end
  end
end

module RSpec::Mocks::ExampleMethods
  def allow(*args); end

  def allow_any_instance_of(*args); end

  def instance_double(*args); end

  def receive(*args); end
end

module RSpec::Matchers
  include Kernel
  def a_collection_including(*args); end
  def a_hash_including(*args); end
  def a_string_including(*args); end
  def a_truthy_value(*args); end
  def allow(*args); end
  def allow_any_instance_of(*args); end
  def an_object_having_attributes(*args); end
  def an_object_satisfying(description = nil, &block); end
  def anything; end
  def be(*args); end
  def be_nil; end
  def change(*args); end
  def contain_exactly(*args); end
  def eq(*args); end
  def equals(*args); end
  def eql(*args); end
  def expect(*args); end
  def have_attributes(*args); end
  def include(*args); end
  def match(*args); end
  def match_array(*args); end
  def not_change(*args); end
  def receive(*args); end
  def satisfy; end
end

module RSpec::Rails::RequestExampleGroup
  sig { params(url: String, as: T.untyped, params: T::Hash[T.untyped, T.untyped], headers: T::Hash[T.untyped, T.untyped]).void }
  def put(url, as:, params:, headers:); end

  def post; end
  def response; end
end


class RSpec::Matchers::BuiltIn::Match
  def initialize(obj)
  end

  def matches?(obj)
  end

  def failure_message
  end
end

module RSpec::Core::Hooks
  sig { params(args: T.untyped, block: T.proc.params(example: T.untyped).bind(T.attached_class).void).void}
  def around(*args, &block); end
end
```
