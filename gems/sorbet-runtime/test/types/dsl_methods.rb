# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::DSLMethodsTest < Critic::Unit::UnitTest
  before do
    T::Private::DeclState.current.reset!
  end

  after do
    T::Private::DeclState.current.reset!
  end

  describe "abstract" do
    it "works with abstract!" do
      parent = Class.new do
        extend T::Sig, T::DefMods, T::Helpers
        abstract!
        sig { void }
        abstract def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig
        sig { override.void }
        def foo; end
      end

      T::Private::Abstract::Validate.validate_subclass(child)
    end

    it "dispatches via super (matching sig { abstract } behavior)" do
      mod = Module.new do
        extend T::Sig, T::DefMods, T::Helpers
        interface!
        sig { returns(String) }
        abstract def foo; end
      end
      parent = Class.new do
        extend T::Sig
        sig { returns(String) }
        def foo; "from parent"; end
      end
      child = Class.new(parent) do
        include mod
      end

      # The abstract method on the module should dispatch to parent's foo via super
      assert_equal("from parent", child.new.foo)
    end

    it "raises NotImplementedError if no super" do
      mod = Module.new do
        extend T::Sig, T::DefMods, T::Helpers
        interface!
        sig { returns(String) }
        abstract def foo; end
      end
      klass = Class.new do
        include mod
      end

      err = assert_raises(NotImplementedError) do
        klass.new.foo
      end
      assert_includes(err.message, "abstract")
    end

    it "errors without preceding sig" do
      err = assert_raises(ArgumentError) do
        Class.new do
          extend T::Sig, T::DefMods, T::Helpers
          abstract!
          def foo; end
          abstract :foo
        end
      end
      assert_includes(err.message, "must declare a `sig`")
    end

    it "errors if called for wrong method" do
      err = assert_raises(ArgumentError) do
        Class.new do
          extend T::Sig, T::DefMods, T::Helpers
          abstract!
          sig { void }
          def foo; end
          sig { void }
          def bar; end
          abstract :foo
        end
      end
      assert_includes(err.message, "previously sig'd method")
    end

    it "errors if called twice" do
      err = assert_raises(ArgumentError) do
        Class.new do
          extend T::Sig, T::DefMods, T::Helpers
          abstract!
          sig { void }
          abstract def foo; end
          abstract :foo
        end
      end
      assert_includes(err.message, "twice")
    end

    it "errors when combined with sig { abstract }" do
      parent = Class.new do
        extend T::Sig, T::DefMods, T::Helpers
        abstract!
        sig { abstract.void }
        abstract def foo; end
      end

      child = Class.new(parent) do
        extend T::Sig
        sig { override.void }
        def foo; end
      end

      # The error is deferred until sig is forced (on first call)
      err = assert_raises(ArgumentError) do
        child.new.foo
      end
      assert_includes(err.message, "abstract")
      assert_includes(err.message, "repeated")
    end
  end

  describe "override" do
    it "works when parent has abstract method" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { void }
        override def foo; end
      end

      T::Private::Abstract::Validate.validate_subclass(child)
    end

    it "works with allow_incompatible" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { void }
        private def foo; end
        override(:foo, allow_incompatible: true)
      end

      T::Private::Abstract::Validate.validate_subclass(child)
    end

    it "works with allow_incompatible using inline def" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { void }
        private override(def foo; end, allow_incompatible: true)
      end

      T::Private::Abstract::Validate.validate_subclass(child)
    end

    it "works with allow_incompatible: :visibility" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { void }
        private def foo; end
        override(:foo, allow_incompatible: :visibility)
      end

      T::Private::Abstract::Validate.validate_subclass(child)
    end

    it "allow_incompatible: :visibility only suppresses visibility errors" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.returns(Integer) }
        def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { returns(String) }
        def foo; "not an int"; end
        override(:foo, allow_incompatible: :visibility)
      end

      err = assert_raises(RuntimeError) do
        T::Private::Abstract::Validate.validate_subclass(child)
      end
      assert_includes(err.message, 'Incompatible return type in signature for implementation of method `foo`')
    end

    it "errors eagerly if override used without overriding something" do
      err = assert_raises(RuntimeError) do
        Class.new do
          extend T::Sig, T::DefMods
          sig { void }
          override def foo; end
        end
      end
      assert_includes(err.message, "You marked `foo` as override, but that method doesn't already exist in this class/module to be overridden")
      assert_match(/remove override .*#{__FILE__}:#{__LINE__ - 4}/, err.message)
    end

    it "errors without preceding sig" do
      err = assert_raises(ArgumentError) do
        Class.new do
          extend T::Sig, T::DefMods
          def foo; end
          override :foo
        end
      end
      assert_includes(err.message, "must declare a `sig`")
    end

    it "errors if called twice" do
      err = assert_raises(ArgumentError) do
        parent = Class.new do
          extend T::Sig, T::Helpers
          abstract!
          sig { abstract.void }
          def foo; end
        end
        Class.new(parent) do
          extend T::Sig, T::DefMods
          sig { void }
          override def foo; end
          override :foo
        end
      end
      assert_includes(err.message, "twice")
    end

    it "errors when combined with sig { override }" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { override.void }
        override def foo; end
      end

      # The error is deferred until sig is forced
      err = assert_raises(ArgumentError) do
        child.new.foo
      end
      assert_includes(err.message, "override")
      assert_includes(err.message, "repeated")
    end
  end

  describe "final" do
    before do
      T::Configuration.enable_final_checks_on_hooks
    end

    after do
      T::Configuration.reset_final_checks_on_hooks
    end

    it "prevents overriding in subclass" do
      parent = Class.new do
        extend T::Sig, T::DefMods
        sig { void }
        final def foo; end
      end

      err = assert_raises(RuntimeError) do
        Class.new(parent) do
          extend T::Sig
          sig { void }
          def foo; end
        end
      end
      assert_includes(err.message, "final")
    end

    it "errors when combined with sig(:final)" do
      err = assert_raises(ArgumentError) do
        Class.new do
          extend T::Sig, T::DefMods
          sig(:final) { void }
          final def foo; end
        end
      end
      assert_includes(err.message, "final")
    end

    it "errors without preceding sig" do
      err = assert_raises(ArgumentError) do
        Class.new do
          extend T::Sig, T::DefMods
          def bar; end
          final :foo
        end
      end
      assert_includes(err.message, "must declare a `sig`")
    end
  end

  describe "overridable" do
    it "allows child to override" do
      parent = Class.new do
        extend T::Sig, T::DefMods
        sig { void }
        overridable def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig
        sig { override.void }
        def foo; end
      end

      T::Private::Abstract::Validate.validate_subclass(child)
    end

    it "errors if called twice" do
      err = assert_raises(ArgumentError) do
        Class.new do
          extend T::Sig, T::DefMods
          sig { void }
          overridable def foo; end
          overridable :foo
        end
      end
      assert_includes(err.message, "twice")
    end

    it "errors when combined with sig { overridable }" do
      klass = Class.new do
        extend T::Sig, T::DefMods
        sig { overridable.void }
        overridable def foo; end
      end

      # The error is deferred until sig is forced
      err = assert_raises(ArgumentError) do
        klass.new.foo
      end
      assert_includes(err.message, "overridable")
      assert_includes(err.message, "repeated")
    end
  end

  describe "composition with visibility" do
    it "abstract private def" do
      parent = Class.new do
        extend T::Sig, T::DefMods, T::Helpers
        abstract!
        sig { void }
        abstract private def foo; end
      end

      assert(parent.private_method_defined?(:foo))

      # force the sig, make sure it's still private
      _sig = T::Utils.signature_for_method(parent.instance_method(:foo))

      assert(parent.private_method_defined?(:foo))
    end

    it "private abstract def" do
      parent = Class.new do
        extend T::Sig, T::DefMods, T::Helpers
        abstract!
        sig { void }
        private abstract def foo; end
      end

      assert(parent.private_method_defined?(:foo))

      # force the sig, make sure it's still private
      _sig = T::Utils.signature_for_method(parent.instance_method(:foo))

      assert(parent.private_method_defined?(:foo))
    end

    it "override private def" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        private def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { void }
        override private def foo; end
      end

      assert(child.private_method_defined?(:foo))

      T::Private::Abstract::Validate.validate_subclass(child)

      assert(child.private_method_defined?(:foo))
    end

    it "private override def" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        private def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { void }
        private override def foo; end
      end

      assert(child.private_method_defined?(:foo))

      T::Private::Abstract::Validate.validate_subclass(child)

      assert(child.private_method_defined?(:foo))
    end

    it "final private def" do
      klass = Class.new do
        extend T::Sig, T::DefMods
        sig { void }
        final private def foo; end
      end

      assert(klass.private_method_defined?(:foo))
    end
  end

  describe "singleton class methods" do
    before do
      T::Configuration.enable_final_checks_on_hooks
    end

    after do
      T::Configuration.reset_final_checks_on_hooks
    end

    it "final def self.foo prevents overriding in subclass" do
      parent = Class.new do
        extend T::Sig, T::DefMods
        sig { void }
        final def self.foo; end
      end

      err = assert_raises(RuntimeError) do
        Class.new(parent) do
          extend T::Sig
          sig { void }
          def self.foo; end
        end
      end
      assert_includes(err.message, "final")
    end
  end

  describe "overridable + override combo" do
    it "overridable override def" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { void }
        overridable override def foo; end
      end
      grandchild = Class.new(child) do
        extend T::Sig
        sig { override.void }
        def foo; end
      end

      T::Private::Abstract::Validate.validate_subclass(child)
      T::Private::Abstract::Validate.validate_subclass(grandchild)
    end
  end

  describe "T::Sig::WithoutRuntime interaction" do
    it "errors because WithoutRuntime.sig does not create a declaration" do
      # WithoutRuntime.sig is a no-op at runtime. It doesn't register a
      # declaration, so the method is treated as having no sig.
      # We extend T::Sig to ensure _on_method_added fires and clears state.
      err = assert_raises(ArgumentError) do
        Class.new do
          extend T::Sig, T::DefMods
          T::Sig::WithoutRuntime.sig { void }
          def foo; end
          abstract :foo
        end
      end
      assert_includes(err.message, "must declare a `sig`")
    end
  end

  describe "postfix form" do
    it "abstract :method_name after def" do
      parent = Class.new do
        extend T::Sig, T::DefMods, T::Helpers
        abstract!
        sig { void }
        def foo; end
        abstract :foo
      end
      child = Class.new(parent) do
        extend T::Sig
        sig { override.void }
        def foo; end
      end

      T::Private::Abstract::Validate.validate_subclass(child)
    end

    it "override :method_name after def" do
      parent = Class.new do
        extend T::Sig, T::Helpers
        abstract!
        sig { abstract.void }
        def foo; end
      end
      child = Class.new(parent) do
        extend T::Sig, T::DefMods
        sig { void }
        def foo; end
        override :foo
      end

      T::Private::Abstract::Validate.validate_subclass(child)
    end
  end

  describe "type error for non-symbol" do
    it "abstract raises TypeError for non-Symbol" do
      err = assert_raises(TypeError) do
        Class.new do
          extend T::Sig, T::DefMods, T::Helpers
          abstract!
          abstract "foo"
        end
      end
      assert_includes(err.message, "Symbol")
    end

    it "override raises TypeError for non-Symbol" do
      err = assert_raises(TypeError) do
        Class.new do
          extend T::Sig, T::DefMods
          override "foo"
        end
      end
      assert_includes(err.message, "Symbol")
    end
  end
end
