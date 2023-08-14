# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class ValidateOverrideTypesTest < Critic::Unit::UnitTest
    class BaseFoo1; end
    class SubFoo1 < BaseFoo1; end
    class BaseFoo2; end
    class SubFoo2 < BaseFoo2; end
    class BaseFoo3; end
    class SubFoo3 < BaseFoo3; end

    class SuccessBase
      extend T::Sig
      sig {overridable.params(pos: SubFoo1, kw: SubFoo2).returns(BaseFoo3)}
      def foo(pos, kw:); end
    end

    class FailureBase
      extend T::Sig
      sig {overridable.params(pos: BaseFoo1, kw: BaseFoo2).returns(SubFoo3)}
      def foo(pos, kw:); end
    end

    it "succeeds if the override types match" do
      klass = Class.new(SuccessBase) do
        extend T::Sig
        sig {override.params(pos: SubFoo1, kw: SubFoo2).returns(BaseFoo3)}
        def foo(pos, kw:)
          BaseFoo3.new
        end
      end
      klass.new.foo(SubFoo1.new, kw: SubFoo2.new)
    end

    it "succeeds if a positional param type is contravariant" do
      klass = Class.new(SuccessBase) do
        extend T::Sig
        sig {override.params(pos: BaseFoo1, kw: SubFoo2).returns(BaseFoo3)}
        def foo(pos, kw:)
          BaseFoo3.new
        end
      end
      klass.new.foo(SubFoo1.new, kw: SubFoo2.new)
    end

    it "succeeds if a keyword param type is contravariant" do
      klass = Class.new(SuccessBase) do
        extend T::Sig
        sig {override.params(pos: SubFoo1, kw: BaseFoo2).returns(BaseFoo3)}
        def foo(pos, kw:)
          BaseFoo3.new
        end
      end
      klass.new.foo(SubFoo1.new, kw: SubFoo2.new)
    end

    it "succeeds if the return type is covariant" do
      klass = Class.new(SuccessBase) do
        extend T::Sig
        sig {override.params(pos: SubFoo1, kw: SubFoo2).returns(SubFoo3)}
        def foo(pos, kw:)
          SubFoo3.new
        end
      end
      klass.new.foo(SubFoo1.new, kw: SubFoo2.new)
    end

    it "raises if a positional param type is covariant" do
      klass = Class.new(FailureBase) do
        extend T::Sig
        sig {override.params(pos: SubFoo1, kw: BaseFoo2).returns(SubFoo3)}
        def foo(pos, kw:); end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(err.message, "Incompatible type for arg #1 (`pos`) in signature for override of method `foo`")
    end

    it "raises if a keyword param type is covariant" do
      klass = Class.new(FailureBase) do
        extend T::Sig
        sig {override.params(pos: BaseFoo1, kw: SubFoo2).returns(SubFoo3)}
        def foo(pos, kw:); end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(err.message, "Incompatible type for arg `kw` in signature for override of method `foo`")
    end

    it "raises if the return type is contravariant" do
      klass = Class.new(FailureBase) do
        extend T::Sig
        sig {override.params(pos: BaseFoo1, kw: BaseFoo2).returns(BaseFoo3)}
        def foo(pos, kw:); end
      end

      err = assert_raises(RuntimeError) do
        klass.new.foo
      end

      assert_includes(err.message, "Incompatible return type in signature for override of method `foo`")
    end

    it "allows T::Class to be compatible with itself" do
      parent = Class.new do
        extend T::Sig
        sig {overridable.returns(T::Class[T.anything])}
        def example; Object; end
      end

      child = Class.new(parent) do
        extend T::Sig
        sig {override.returns(T::Class[T.anything])}
        def example; Object; end
      end

      child.new.example
    end

    it "allows T::Class to be compatible with T.class_of in child" do
      parent = Class.new do
        extend T::Sig
        sig {overridable.returns(T::Class[T.anything])}
        def example; Object; end
      end

      child = Class.new(parent) do
        extend T::Sig
        sig {override.returns(T.class_of(Object))}
        def example; Object; end
      end

      child.new.example
    end

    it "allows Object to be compatible with T::Hash in child" do
      parent = Class.new do
        extend T::Sig
        sig {overridable.returns(Object)}
        def example; nil; end
      end

      child = Class.new(parent) do
        extend T::Sig
        sig {override.returns(T::Hash[Symbol, T.untyped])}
        def example; {}; end
      end

      child.new.example
    end

  end
end
