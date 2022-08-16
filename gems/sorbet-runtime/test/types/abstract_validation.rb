# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::AbstractValidationTest < Critic::Unit::UnitTest
  after do
    T::Private::DeclState.current.reset!
  end

  module AbstractMixin
    extend T::Sig
    extend T::Helpers
    abstract!

    sig {abstract.returns(Object)}
    def foo; end

    sig {abstract.returns(Object)}
    def bar; end

    sig {returns(Object)}
    def concrete_standard; end

    def concrete_no_signature; end
  end

  class AbstractClass
    extend T::Sig
    extend T::Helpers
    abstract!

    sig {abstract.returns(Object)}
    def self.foo; end

    sig {abstract.returns(Object)}
    def bar; end
  end

  it "raises an error when defining an abstract class method on a module" do
    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      abstract!
      sig {abstract.returns(Object)}
      def self.foo; end
    end

    err = assert_raises(RuntimeError) do
      mod.foo
    end

    assert_includes(
      err.message,
      "Defining an overridable class method (via .abstract) on a module is not allowed.",
    )
  end

  it "allows overriding an abstract method with an unannotated method" do
    mod = Module.new do
      extend T::Helpers
      include AbstractMixin

      def foo; end
      def bar; end
    end

    klass = Class.new do
      include mod
    end
    klass.new.foo
    klass.new.bar
  end

  it "succeeds when overriding an abstract method with a sig via a mixin" do
    mixin = Module.new do
      extend T::Sig
      extend T::Helpers

      sig {returns(Object)}
      def foo; end

      sig {returns(Object)}
      def bar; end
    end

    mod = Module.new do
      extend T::Helpers
      include AbstractMixin
      include mixin
    end

    klass = Class.new do
      include mod
    end
    klass.new.foo
    klass.new.bar
  end

  it "raises an error when calling an abstract method" do
    err = assert_raises(NotImplementedError) do
      AbstractClass.foo
    end
    assert_equal(
      "The method `foo` on #<Class:Opus::Types::Test::AbstractValidationTest::AbstractClass> is declared as `abstract`. It does not have an implementation.",
      err.message
    )
  end

  it "succeeds if a concrete module implements all abstract methods" do
    mod = Module.new do
      extend T::Sig
      extend T::Helpers
      include AbstractMixin

      sig {override.returns(Object)}
      def foo; end

      sig {override.returns(Object)}
      def bar; end
    end

    klass = Class.new do
      include mod
    end
    klass.new.foo
    klass.new.bar
  end

  it "fails if a concrete module doesn't implement abstract methods" do
    mod = Module.new do
      extend T::Helpers
      include AbstractMixin
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_subclass(mod)
    end

    assert_includes(err.message, "Missing implementation for abstract instance method(s) in #{mod}")
    assert_includes(err.message, "`foo` declared in #{AbstractMixin}")
    assert_includes(err.message, "`bar` declared in #{AbstractMixin}")
  end

  describe "class methods" do
    it "succeeds if a concrete module implements all abstract methods" do
      mod = Module.new do
        extend T::Sig
        extend T::Helpers
        extend AbstractMixin

        sig {override.returns(Object)}
        def self.foo; end

        sig {override.returns(Object)}
        def self.bar; end
      end

      T::Private::Abstract::Validate.validate_subclass(mod)
    end

    it "fails if a concrete module doesn't implement abstract methods" do
      mod = Module.new do
        extend T::Helpers
        extend AbstractMixin
      end

      err = assert_raises(RuntimeError) do
        T::Private::Abstract::Validate.validate_subclass(mod.singleton_class)
      end

      assert_includes(err.message, "Missing implementation for abstract class method(s) in #{mod.singleton_class}")
      assert_includes(err.message, "`foo` declared in #{AbstractMixin}")
      assert_includes(err.message, "`bar` declared in #{AbstractMixin}")
    end
  end

  describe "handling existing methods when including an abstract mixin" do
    it "succeeds when the method comes from a parent class" do
      parent = Class.new do
        def foo
          :foo
        end

        def bar
          :bar
        end
      end

      klass = Class.new(parent) do
        include AbstractMixin
      end

      assert_equal(:foo, klass.new.foo)
      assert_equal(:bar, klass.new.bar)
    end

    it "succeeds when the method comes from a mixin" do
      mixin = Module.new do
        def foo; end
        def bar; end
      end

      mod = Module.new do
        extend T::Helpers
        abstract!
        include mixin
        include AbstractMixin
      end

      T::Private::Abstract::Validate.validate_subclass(mod)
    end

    it "succeeds when the interface comes second" do
      mixin = Module.new do
        extend T::Sig
        extend T::Helpers
        abstract!
        sig {abstract.params(arg: T.untyped).returns(T.untyped)}
        def foo(arg); end
      end

      impl = Module.new do
        def foo(arg)
          arg
        end
      end

      klass = Class.new do
        include impl
        include mixin
      end

      assert_equal(3, klass.new.foo(3))
      T::Private::Abstract::Validate.validate_abstract_module(mixin)
      T::Private::Abstract::Validate.validate_abstract_module(klass)
    end

    it "succeeds when two abstract modules declare the same method and two mixins implement it" do
      mixin1 = Module.new do
        extend T::Sig
        extend T::Helpers
        abstract!
        sig {abstract.returns(T.untyped)}
        def foo; end
      end

      mixin2 = Module.new do
        extend T::Sig
        extend T::Helpers
        abstract!
        sig {abstract.returns(T.untyped)}
        def foo; end
      end

      impl1 = Module.new do
        def foo
          :impl1
        end
      end

      impl2 = Module.new do
        def foo
          :impl2
        end
      end

      klass = Class.new do
        include mixin1
        include impl1
        include mixin2
        include impl2
      end

      assert_equal(:impl2, klass.new.foo)
    end

    it "suceeds when the method is defined directly on the receiving class" do
      mod = Module.new do
        extend T::Helpers
        abstract!
        def foo; end
        include AbstractMixin
      end

      T::Private::Abstract::Validate.validate_abstract_module(mod)
    end

    it "succeeds when including an abstract mixin indirectly a second time" do
      mixin = Module.new do
        extend T::Helpers
        abstract!
        include AbstractMixin
      end

      mod = Module.new do
        extend T::Helpers
        abstract!
        include AbstractMixin
        include mixin
      end

      T::Private::Abstract::Validate.validate_abstract_module(mod)
    end
  end

  describe "classes with abstract class and instance methods" do
    it "succeeds when everything is implemented" do
      klass = Class.new(AbstractClass) do
        extend T::Sig
        extend T::Helpers
        sig {override.returns(Object)}
        def self.foo; end

        sig {override.returns(Object)}
        def bar; end
      end
      klass.foo
      klass.new.bar
    end

    it "fails if a class method is unimplemented" do
      klass = Class.new(AbstractClass) do
        extend T::Sig
        extend T::Helpers
        sig {override.returns(Object)}
        def bar; end
      end

      err = assert_raises(RuntimeError) do
        T::Private::Abstract::Validate.validate_subclass(klass.singleton_class)
      end

      assert_includes(
        err.message,
        "Missing implementation for abstract class method(s) in #{klass.singleton_class}"
      )
      assert_includes(err.message, "`foo` declared in #{AbstractClass.singleton_class}")
    end

    it "fails if an instance method is unimplemented" do
      klass = Class.new(AbstractClass) do
        extend T::Sig
        extend T::Helpers
        sig {override.returns(Object)}
        def self.foo; end
      end

      err = assert_raises(RuntimeError) do
        T::Private::Abstract::Validate.validate_subclass(klass)
      end

      assert_includes(
        err.message,
        "Missing implementation for abstract instance method(s) in #{klass}"
      )
      assert_includes(err.message, "`bar` declared in #{AbstractClass}")
    end

    it "fails when instantiating the class" do
      err = assert_raises(RuntimeError) do
        AbstractClass.new
      end

      assert_equal(
        "#{AbstractClass} is declared as abstract; it cannot be instantiated",
        err.message
      )
    end

    it "succeeds when instantiating a concrete subclass" do
      klass = Class.new(AbstractClass) do
        extend T::Sig
        extend T::Helpers
        sig {override.returns(Object)}
        def self.foo; end

        sig {override.returns(Object)}
        def bar
          "baz"
        end
      end
      assert_equal("baz", klass.new.bar)
    end

    it 'succeeds with abstract methods from parents' do
      parent = Class.new do
        def foo; end
      end
      mod = Module.new do
        extend T::Sig
        extend T::Helpers
        interface!
        sig {abstract.void}
        def foo; end
      end
      klass = Class.new(parent) do
        include mod
      end
      klass.new.foo
      T::Private::Abstract::Validate.validate_subclass(klass)
    end

    it 'can override methods using type members' do
      parent = Class.new do
        extend T::Sig
        extend T::Generic

        tpl = type_template

        sig {abstract.returns(tpl)}
        def load_one; end
      end

      child = Class.new(parent) do
        extend T::Sig
        tpl = type_template {{fixed: Integer}}

        sig {override.returns(tpl)}
        def load_one
          0
        end
      end

      assert_equal(0, child.new.load_one)
    end

    it 'can override methods using type parameters' do
      parent = Class.new do
        extend T::Sig
        extend T::Generic

        sig do
          type_parameters(:T)
          .abstract
          .params(x: T.type_parameter(:T))
          .returns(T.type_parameter(:T))
        end
        def id(x); end
      end

      child = Class.new(parent) do
        extend T::Sig
        sig do
          type_parameters(:T)
          .override
          .params(x: T.type_parameter(:T))
          .returns(T.type_parameter(:T))
        end
        def id(x)
          x
        end
      end

      assert_equal(0, child.new.id(0))
    end
  end

  it "handles splats and kwargs" do
    parent = Class.new do
      extend T::Sig
      extend T::Helpers
      abstract!

      sig {abstract.params(args: Integer, opts: Integer).void}
      def foo(*args, **opts); end
    end

    child = Class.new(parent) do
      def foo(*args, **opts); end
    end

    T::Private::Abstract::Validate.validate_subclass(child)

    noopts = Class.new(parent) do
      def foo(*args); end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_subclass(noopts)
    end
    assert_includes(
      err.message,
      "Your definition of `foo` must have `**opts` to be compatible with the method it implements",
    )

    noargs = Class.new(parent) do
      def foo(**opts); end
    end

    err = assert_raises(RuntimeError) do
      T::Private::Abstract::Validate.validate_subclass(noargs)
    end
    assert_includes(
      err.message,
      "Your definition of `foo` must have `*args` to be compatible with the method it implements",
    )

    parent = Class.new do
      def initialize(foo:)
        super()
        @foo = foo
      end
    end

    abstract = Class.new(parent) do
      extend T::Sig
      extend T::Helpers
      abstract!
    end

    concrete = Class.new(abstract)
    concrete.new(foo: 1)
  end

  it "calls super abstract! if such a method exists" do
    mod = Module.new do
      def abstract!
        raise "Called abstract! in parent"
      end
    end
    exn = assert_raises(RuntimeError) do
      Class.new do
        extend mod
        extend T::Helpers
        abstract!
      end
    end
    assert_equal("Called abstract! in parent", exn.message)
  end
end
