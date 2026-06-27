# typed: true
# enable-experimental-rspec: true

# This test shows that RSpec DSL methods like `let`, `it`, and `context`
# are recognized by Sorbet with no errors.
# RSpec.describe accepts multiple arguments (metadata tags), and the first
# argument can be a string, class, symbol, or other type.

# Define minimal RSpec module for testing
module RSpec
  module Core
    class ExampleGroup
    end
  end

  def self.describe(description, *args, &block); end
end

# Test RSpec.describe with let, it, and nested context
RSpec.describe "User" do
  let(:name) { "Alice" }

  it "has a name" do
    name
  end

  context "when verified" do
    let(:verified) { true }

    it "shows verified status" do
      name
      verified
    end
  end
end

# Test RSpec.describe with a string and a symbol metadata tag
RSpec.describe "User", :needs_macos do
  let(:path) { "/some/path" }

  it "has a path" do
    path
  end
end

# Test RSpec.describe with a string and multiple metadata tags
RSpec.describe "User", :needs_macos, :slow do
  let(:value) { 42 }

  it "has a value" do
    value
  end
end

# Test RSpec.describe with a string, symbol, and hash metadata
RSpec.describe "User", :needs_macos, focus: true do
  let(:count) { 10 }

  it "has a count" do
    count
  end
end

# Test RSpec.describe with a class as the first argument
class UserClass
end

RSpec.describe UserClass do
#              ^^^^^^^^^ hover: T.class_of(UserClass)
  let(:name) { "Bob" }

  it "has a name" do
    name
  end

  it "reveals the typed described_class" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(UserClass)`
    #             ^^^^^^^^^^^^^^^ hover: sig { returns(T.class_of(UserClass)) }
  end

  it "rejects calls that don't exist on described_class" do
    described_class.no_such_method # error: Method `no_such_method` does not exist on `T.class_of(UserClass)`
  end
end

# Test that `subject` / `let` bodies see the typed `described_class`. The return
# type of `subject` / `let` is `T.untyped` without an explicit sig, but the body
# is typechecked, so bogus method calls on `described_class` are caught.
RSpec.describe UserClass do
  subject(:user) { described_class.new }
  let(:bogus) { described_class.no_such_method } # error: Method `no_such_method` does not exist on `T.class_of(UserClass)`
end

# Test that an explicit sig on `subject` / `let` propagates the typed return
# type to consumers of the helper, composing with the synthesized
# `described_class`.
RSpec.describe UserClass do
  extend T::Sig

  sig { returns(UserClass) }
  subject(:user) { described_class.new }

  sig { returns(T.class_of(UserClass)) }
  let(:klass) { described_class }

  it "propagates the typed return through subject and let" do
    T.reveal_type(user)  # error: Revealed type: `UserClass`
    T.reveal_type(klass) # error: Revealed type: `T.class_of(UserClass)`
  end
end

# Test that `RSpec.describe` works on a module, not just a class. `described_class`
# resolves to `T.class_of(SomeModule)`, so module-level methods are callable but
# `.new` (which doesn't exist on modules) errors.
module DescribedModule
  def self.helper; end
end

RSpec.describe DescribedModule do
  it "described_class is the singleton type of the module" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(DescribedModule)`
    described_class.helper
    described_class.new # error: Method `new` does not exist on `T.class_of(DescribedModule)`
  end
end

# Test that `before` and `after` hooks see the typed `described_class`. These
# are rewritten to instance methods on the synthesized describe class, so they
# share the same type information as `it` blocks.
RSpec.describe UserClass do
  before do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(UserClass)`
    described_class.no_such_method # error: Method `no_such_method` does not exist on `T.class_of(UserClass)`
  end

  after do
    described_class.also_missing # error: Method `also_missing` does not exist on `T.class_of(UserClass)`
  end
end

# Test that a user-defined `let(:described_class)` overrides the synthesized
# method. The rewriter detects the existing definition in the class body and
# skips synthesis so the user's type wins.
#
# These tests use dedicated outer classes (rather than reusing `UserClass`)
# because every `RSpec.describe UserClass do ... end` block in this file is
# rewritten into the same `<describe 'UserClass'>` synthetic class, and a user
# override here would collide with the synthesized `described_class` from a
# sibling block of the same name.
class AlternateClass
end

class UserClassWithLetOverride
end

class UserClassWithDefOverride
end

class UserClassWithSubjectOverride
end

RSpec.describe UserClassWithLetOverride do
  extend T::Sig

  sig { returns(T.class_of(AlternateClass)) }
  let(:described_class) { AlternateClass }

  it "honors the user-defined described_class via let" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(AlternateClass)`
  end
end

# Same as above, but using `def described_class` instead of `let`.
RSpec.describe UserClassWithDefOverride do
  extend T::Sig

  sig { returns(T.class_of(AlternateClass)) }
  def described_class
    AlternateClass
  end

  it "honors the user-defined described_class via def" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(AlternateClass)`
  end
end

# Same as above, but using `subject(:described_class)`. `subject` with a name
# argument rewrites to a `def described_class`, so the user-override detector
# catches it the same way.
RSpec.describe UserClassWithSubjectOverride do
  extend T::Sig

  sig { returns(T.class_of(AlternateClass)) }
  subject(:described_class) { AlternateClass }

  it "honors the user-defined described_class via subject" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(AlternateClass)`
  end
end

# Test that `xdescribe` and `fdescribe` (skipped/focused) with a constant arg
# also synthesize a typed `described_class`, since they share the same case
# labels in the rewriter as `describe`.
RSpec.xdescribe UserClass do
  it "xdescribe propagates described_class" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(UserClass)`
  end
end

RSpec.fdescribe UserClass do
  it "fdescribe propagates described_class" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(UserClass)`
  end
end

# Test RSpec.describe with a class and metadata
RSpec.describe UserClass, :needs_macos do
#              ^^^^^^^^^ hover: T.class_of(UserClass)
  let(:value) { 100 }

  it "has a value" do
    value
  end

  it "reveals the typed described_class with metadata args" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(UserClass)`
  end
end

# Test RSpec.describe with namespaced classes sharing the same leaf name
module Foo
  module Bar
    class Baz; end
  end
  module Qux
    class Baz; end
  end
end

RSpec.describe Foo::Bar::Baz do
  let(:instance) { Foo::Bar::Baz.new }

  it "creates an instance" do
    instance
  end
end

RSpec.describe Foo::Qux::Baz do
  let(:instance) { Foo::Qux::Baz.new }

  it "creates an instance" do
    instance
  end
end

# Test that nested `describe`/`context` with a constant arg overrides
# `described_class` to the inner constant, while non-constant nested blocks
# inherit `described_class` from the enclosing constant.
RSpec.describe Foo::Bar::Baz do
  it "outer described_class" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(Foo::Bar::Baz)`
  end

  describe Foo::Qux::Baz do
    before do
      T.reveal_type(described_class) # error: Revealed type: `T.class_of(Foo::Qux::Baz)`
    end

    it "inner describe overrides described_class" do
      T.reveal_type(described_class) # error: Revealed type: `T.class_of(Foo::Qux::Baz)`
    end
  end

  describe "a string argument" do
    it "does not override described_class" do
      T.reveal_type(described_class) # error: Revealed type: `T.class_of(Foo::Bar::Baz)`
    end
  end

  context :a_symbol_argument do
    it "does not override described_class" do
      T.reveal_type(described_class) # error: Revealed type: `T.class_of(Foo::Bar::Baz)`
    end
  end
end

# Test that a user-defined `let(:described_class)` in a *nested* describe skips
# synthesis only for the inner class — the outer still synthesizes its own
# typed `described_class`. The nested ClassDef is opaque to the outer's
# user-override check, so the two are independent.
class OuterForNestedOverride
end

class InnerForNestedOverride
end

RSpec.describe OuterForNestedOverride do
  extend T::Sig

  it "outer still synthesizes described_class" do
    T.reveal_type(described_class) # error: Revealed type: `T.class_of(OuterForNestedOverride)`
  end

  describe InnerForNestedOverride do
    sig { returns(T.class_of(AlternateClass)) }
    let(:described_class) { AlternateClass }

    it "inner override wins" do
      T.reveal_type(described_class) # error: Revealed type: `T.class_of(AlternateClass)`
    end
  end
end

# Test RSpec.describe with a symbol as the first argument
RSpec.describe :user_symbol do
  let(:data) { "test" }

  it "has data" do
    data
  end
end

# Test RSpec.describe with a symbol and metadata
RSpec.describe :user_symbol, :slow do
  let(:result) { true }

  it "has a result" do
    result
  end
end

# Test `describe` with a constant bound to a *value* (not a class or module).
# RSpec permutation specs commonly `describe` such constants, e.g.
# `describe ReferenceData::DepositKey::KyJohnsonCountyOlf` where the constant is
# `= SomeClass.new`. `T.class_of(<value>)` is invalid, so rather than reporting
# "T.class_of can't be used with a constant field" (5004) or a bare-value-in-type
# error (7009) on a `T.class_of` the user never wrote, the synthesized
# `described_class` degrades to `T.untyped`.
class SomeValueClass; end
SOME_VALUE_CONSTANT = SomeValueClass.new

RSpec.describe SOME_VALUE_CONSTANT do
  it "does not error on a value-constant describe arg" do
    T.reveal_type(described_class) # error: Revealed type: `T.untyped`
    described_class.anything_at_all
  end
end

# Nested `describe` over a value constant behaves the same as the top-level case.
RSpec.describe UserClass do
  describe SOME_VALUE_CONSTANT do
    it "value-constant nested describe does not error" do
      T.reveal_type(described_class) # error: Revealed type: `T.untyped`
    end
  end
end

# A `T.type_alias` constant is handled the same way as a value constant: the
# synthesized `T.class_of(<type alias>)` would otherwise report "T.class_of
# can't be used with a T.type_alias" (5004), which the user can't act on, so it
# degrades to `T.untyped`. (RSpec specs describe type aliases in practice, e.g.
# `describe Payments::Types::TransmissionRecord`.)
MyTypeAlias = T.type_alias { T.any(Integer, String) }

RSpec.describe MyTypeAlias do
  it "does not error on a type-alias describe arg" do
    T.reveal_type(described_class) # error: Revealed type: `T.untyped`
  end
end

RSpec.describe UserClass do
  describe MyTypeAlias do
    it "type-alias nested describe does not error" do
      T.reveal_type(described_class) # error: Revealed type: `T.untyped`
    end
  end
end
