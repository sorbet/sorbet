# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::FinalMethodTest < Critic::Unit::UnitTest
  before do
    T::Configuration.enable_final_checks_on_hooks
  end

  after do
    T::Private::DeclState.current.reset!
    T::Configuration.reset_final_checks_on_hooks
  end

  it "allows declaring an instance method as final" do
    Class.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
  end

  it "allows declaring a class method as final" do
    Class.new do
      extend T::Sig
      sig(:final) {void}
      def self.foo; end
    end
  end

  it "forbids redefining a final instance method with a final sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def foo; end
        sig(:final) {void}
        def foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:0x[0-9a-f]+> was declared as final and cannot be redefined$/, err.message)
  end

  it "forbids redefining a final class method with a final sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def self.foo; end
        sig(:final) {void}
        def self.foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:#<Class:0x[0-9a-f]+>> was declared as final and cannot be redefined$/, err.message)
  end

  it "forbids redefining a final instance method with a regular sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def foo; end
        sig {void}
        def foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:0x[0-9a-f]+> was declared as final and cannot be redefined$/, err.message)
  end

  it "forbids redefining a final class method with a regular sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def self.foo; end
        sig {void}
        def self.foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:#<Class:0x[0-9a-f]+>> was declared as final and cannot be redefined$/, err.message)
  end

  it "forbids redefining a final instance method with no sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def foo; end
        def foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:0x[0-9a-f]+> was declared as final and cannot be redefined$/, err.message)
  end

  it "forbids redefining a final class method with no sig" do
    err = assert_raises(RuntimeError) do
      Class.new do
        extend T::Sig
        sig(:final) {void}
        def self.foo; end
        def self.foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:#<Class:0x[0-9a-f]+>> was declared as final and cannot be redefined$/, err.message)
  end

  it "allows redefining a regular instance method to be final" do
    Class.new do
      extend T::Sig
      def foo; end
      sig(:final) {void}
      def foo; end
    end
  end

  it "allows redefining a regular class method to be final" do
    Class.new do
      extend T::Sig
      def self.foo; end
      sig(:final) {void}
      def self.foo; end
    end
  end

  it "forbids overriding a final instance method" do
    c = Class.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new(c) do
        def foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:0x[0-9a-f]+> was declared as final and cannot be overridden in #<Class:0x[0-9a-f]+>$/, err.message)
  end

  it "forbids overriding a final class method" do
    c = Class.new do
      extend T::Sig
      sig(:final) {void}
      def self.foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new(c) do
        def self.foo; end
      end
    end
    assert_match(/^The method `foo` on #<Class:#<Class:0x[0-9a-f]+>> was declared as final and cannot be overridden in #<Class:#<Class:0x[0-9a-f]+>>$/, err.message)
  end

  it "forbids overriding a final method from an included module" do
    m = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        include m
        def foo; end
      end
    end
    assert_match(/^The method `foo` on #<Module:0x[0-9a-f]+> was declared as final and cannot be overridden in #<Class:0x[0-9a-f]+>$/, err.message)
  end

  it "forbids overriding a final method from an extended module" do
    m = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        extend m
        def self.foo; end
      end
    end
    assert_match(/^The method `foo` on #<Module:0x[0-9a-f]+> was declared as final and cannot be overridden in #<Class:#<Class:0x[0-9a-f]+>>$/, err.message)
  end

  it "forbids overriding a final method by including two modules" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    m2 = Module.new do
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        include m2, m1
      end
    end
    assert_match(/^The method `foo` on #<Module:0x[0-9a-f]+> was declared as final and cannot be overridden in #<Class:0x[0-9a-f]+>$/, err.message)
  end

  it "forbids overriding a final method by extending two modules" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    m2 = Module.new do
      def foo; end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        extend m2, m1
      end
    end
    assert_match(/^The method `foo` on #<Module:0x[0-9a-f]+> was declared as final and cannot be overridden in #<Class:0x[0-9a-f]+>$/, err.message)
  end

  it "allows calling final methods" do
    m = Module.new do
      extend T::Sig
      sig(:final) {void}
      def self.n0; end
      sig(:final) {params(x: Integer).void}
      def self.n1(x); end
      sig(:final) {params(x: Integer, y: Integer).void}
      def self.n2(x, y); end
      sig(:final) {params(x: Integer, y: Integer, z: Integer).void}
      def self.n3(x, y, z); end
    end
    m.n0
    m.n1 1
    m.n2 1, 2
    m.n3 1, 2, 3
  end

  it "calls a user-defined included" do
    m = Module.new do
      @calls = 0
      extend T::Sig
      sig(:final) {returns(Integer)}
      def self.calls
        @calls
      end
      def self.included(x)
        @calls += 1
      end
    end
    Class.new do
      include m
    end
    assert_equal(1, m.calls)
    Class.new do
      include m
    end
    assert_equal(2, m.calls)
  end

  it "calls a user-defined extended" do
    m = Module.new do
      @calls = 0
      extend T::Sig
      sig(:final) {returns(Integer)}
      def self.calls
        @calls
      end
      def self.extended(x)
        @calls += 1
      end
    end
    Class.new do
      extend m
    end
    assert_equal(1, m.calls)
    Class.new do
      extend m
    end
    assert_equal(2, m.calls)
  end

  it "calls an exotic user-defined included" do
    m2 = Module.new do
      def self.included(arg)
        arg.include(Module.new do
          extend T::Sig
          sig(:final) {void}
          def foo; end
        end)
      end
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        include m2
        def foo; end
      end
    end
    assert_match(/^The method `foo` on #<Module:0x[0-9a-f]+> was declared as final and cannot be overridden in #<Class:0x[0-9a-f]+>$/, err.message)
  end

  it "forbids overriding through many levels of include" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    m2 = Module.new do
      include m1
    end
    m3 = Module.new do
      include m2
    end
    err = assert_raises(RuntimeError) do
      Class.new do
        include m3
        def foo; end
      end
    end
    assert_match(/^The method `foo` on #<Module:0x[0-9a-f]+> was declared as final and cannot be overridden in #<Class:0x[0-9a-f]+>$/, err.message)
  end

  it "allows including modules again" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    Module.new do
      include m1, m1
    end
  end

  it "allows extending modules again" do
    m1 = Module.new do
      extend T::Sig
      sig(:final) {void}
      def foo; end
    end
    Module.new do
      extend m1, m1
    end
  end

  it "has a good error if you use the wrong syntax" do
    err = assert_raises(ArgumentError) do
      m = Module.new do
        extend T::Sig
        sig {final.void}
        def self.foo; end
      end
      m.foo
    end
    assert_includes(err.message, "The syntax for declaring a method final is `sig(:final) {...}`, not `sig {final. ...}`")
  end
end
