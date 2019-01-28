# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::MethodModesTest < Critic::Unit::UnitTest
  module AbstractMixin
    extend T::Helpers
    abstract!

    sig {abstract.returns(Object)}
    def foo; end
  end

  describe "with a super method" do
    it "succeeds when overriding an unannotated method with .override" do
      mixin = Module.new do
        def foo; end
      end

      mod = Module.new do
        extend T::Helpers
        include mixin
        sig {override.returns(Object)}
        def foo; end
      end

      klass = Class.new do
        include mod
      end
      klass.new.foo
    end

    it "succeeds when overriding an .override method with .override" do
      mixin = Module.new do
        extend T::Helpers
        sig {override.returns(Object)}
        def self.name; 'foo'; end
      end

      mod = Module.new do
        extend T::Helpers
        extend mixin
        sig {override.returns(Object)}
        def self.name; 'bar'; end
      end

      mod.name
    end

    it "succeeds when overriding an .overridable with .override" do
      mixin = Module.new do
        extend T::Helpers
        sig {overridable.returns(Object)}
        def foo; end
      end

      mod = Module.new do
        extend T::Helpers
        include mixin
        sig {override.returns(Object)}
        def foo; end
      end

      klass = Class.new do
        include mod
      end
      klass.new.foo
    end

    it "succeeds when overriding an .overridable.implementation method with .override" do
      mixin = Module.new do
        include AbstractMixin
        extend T::Helpers
        sig {implementation.overridable.returns(Object)}
        def foo; end
      end

      mod = Module.new do
        include mixin
        extend T::Helpers
        sig {override.returns(Object)}
        def foo; end
      end

      klass = Class.new do
        include mod
      end
      klass.new.foo
    end

    it "raises when overriding a standard method with .override" do
      parent = Class.new do
        extend T::Helpers
        sig {returns(Object)}
        def foo; end
      end

      klass = Class.new(parent) do
        extend T::Helpers
        sig {override.returns(Object)}
        def foo; end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(
        err.message,
        "You declared `foo` as .override, but the method it overrides is not declared as `overridable`.\n" \
        "  Parent definition: #{parent} at #{__FILE__}"
      )
    end

    it "raises when overriding an abstract method with .override" do
      klass = Class.new do
        extend T::Helpers
        include AbstractMixin
        sig {override.returns(Object)}
        def foo; end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(
        err.message,
        "You declared `foo` as .override, but the method it overrides is not declared as `overridable`.\n" \
        "  Parent definition: #{AbstractMixin} at #{__FILE__}"
      )
    end

    it "raises when overriding an implementation method with .override" do
      parent = Class.new do
        extend T::Helpers
        include AbstractMixin
        sig {implementation.returns(Object)}
        def foo; end
      end

      klass = Class.new(parent) do
        sig {override.returns(Object)}
        def foo; end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(
        err.message,
        "You declared `foo` as .override, but the method it overrides is not declared as `overridable`.\n" \
        "  Parent definition: #{parent} at #{__FILE__}"
      )
    end

    it "succeeds when overriding an unannotated method with bare sig" do
      parent = Class.new do
        def foo; end
      end

      klass = Class.new(parent) do
        extend T::Helpers
        sig {returns(Object)}
        def foo; end
      end

      klass.new.foo
    end

    it "succeeds when overriding a standard method with bare sig" do
      parent = Class.new do
        extend T::Helpers
        sig {returns(Object)}
        def foo; end
      end

      klass = Class.new(parent) do
        extend T::Helpers
        sig {returns(Object)}
        def foo; end
      end

      klass.new.foo
    end

    it "raises when overriding an abstract method with a bare sig" do
      klass = Class.new do
        extend T::Helpers
        include AbstractMixin
        sig {returns(Object)}
        def foo; end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(
        err.message,
        "You must use `.implementation` when overriding the abstract method `foo`.\n" \
        "  Abstract definition: #{AbstractMixin} at #{__FILE__}"
      )
    end

    it "raises when overriding an .overridable method with .implementation" do
      parent = Class.new do
        extend T::Helpers
        sig {overridable.returns(Object)}
        def foo; end
      end

      klass = Class.new(parent) do
        sig {implementation.returns(Object)}
        def foo; end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(
        err.message,
        "You declared `foo` as .implementation, but the method it overrides is not declared as abstract.\n" \
        "  Either mark foo as `abstract.` in the parent: #{parent} at #{__FILE__}"
      )
    end
  end

  describe "without a super method" do
    it "raises when using .override with no override" do
      klass = Class.new do
        extend T::Helpers
        sig {override.returns(Object)}
        def foo; end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(
        err.message,
        "You marked `foo` as .override, but that method doesn't already exist"
      )
    end

    it "raises when using .implementation_method on a non-override" do
      klass = Class.new do
        extend T::Helpers
        sig {implementation.returns(Object)}
        def foo; end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(
        err.message,
        "You marked `foo` as .implementation, but it doesn't match up with a corresponding abstract method."
      )
    end
  end
end
