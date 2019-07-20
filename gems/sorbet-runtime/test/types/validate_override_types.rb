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
        def foo(pos, kw:); BaseFoo3.new; end
      end
      klass.new.foo(SubFoo1.new, kw: SubFoo2.new)
    end

    it "succeeds if a positional param type is contravariant" do
      klass = Class.new(SuccessBase) do
        extend T::Sig
        sig {override.params(pos: BaseFoo1, kw: SubFoo2).returns(BaseFoo3)}
        def foo(pos, kw:); BaseFoo3.new; end
      end
      klass.new.foo(SubFoo1.new, kw: SubFoo2.new)
    end

    it "succeeds if a keyword param type is contravariant" do
      klass = Class.new(SuccessBase) do
        extend T::Sig
        sig {override.params(pos: SubFoo1, kw: BaseFoo2).returns(BaseFoo3)}
        def foo(pos, kw:); BaseFoo3.new; end
      end
      klass.new.foo(SubFoo1.new, kw: SubFoo2.new)
    end

    it "succeeds if the return type is covariant" do
      klass = Class.new(SuccessBase) do
        extend T::Sig
        sig {override.params(pos: SubFoo1, kw: SubFoo2).returns(SubFoo3)}
        def foo(pos, kw:); SubFoo3.new; end
      end
      klass.new.foo(SubFoo1.new, kw: SubFoo2.new)
    end

    describe "overriding with a type member in the subclass" do
      it "succeeds when a positional param is a type member in the subclass" do
        class GenericPositionalSub < SuccessBase
          extend T::Generic
          Elem = type_member
          sig {override.params(pos: Elem, kw: SubFoo2).returns(BaseFoo3)}
          def foo(pos, kw:)
            BaseFoo3.new
          end
        end

        GenericPositionalSub[NilClass].new.foo(nil, kw: SubFoo2.new)
      end

      it "succeeds when a keyword param is a type member in the subclass" do
        class GenericKeywordSub < SuccessBase
          extend T::Generic
          Elem = type_member
          sig {override.params(pos: SubFoo1, kw: Elem).returns(BaseFoo3)}
          def foo(pos, kw:)
            BaseFoo3.new
          end
        end

        GenericKeywordSub[NilClass].new.foo(SubFoo1.new, kw: nil)
      end

      it "succeeds when the return type is a type member in the subclass" do
        class GenericReturnSub < SuccessBase
          extend T::Generic
          Elem = type_member
          sig {override.params(pos: SubFoo1, kw: SubFoo2).returns(Elem)}
          def foo(pos, kw:); end
        end

        GenericReturnSub[NilClass].new.foo(SubFoo1.new, kw: SubFoo2.new)
      end
    end

    describe "overriding when a type member is in the base class method signature" do
      it "succeeds when a positional param is a type member in the base class" do
        class GenericPositionalBase
          extend T::Generic
          extend T::Sig
          Elem = type_member
          sig {overridable.params(pos: Elem).void}
          def foo(pos); end
        end

        class GenericPositionalSub2 < GenericPositionalBase
          Elem = type_member(fixed: T::Array[String])
          sig {override.params(pos: T::Array[String]).void}
          def foo(pos); end
        end

        GenericPositionalSub2.new.foo(["hi"])
      end

      it "succeeds when a keyword param is a type member in the base class" do
        class GenericKeywordBase
          extend T::Generic
          extend T::Sig
          Elem = type_member
          sig {overridable.params(key: Elem).void}
          def foo(key:); end
        end

        class GenericKeywordSub2 < GenericKeywordBase
          Elem = type_member(fixed: T::Array[String])
          sig {override.params(key: T::Array[String]).void}
          def foo(key:); end
        end

        GenericKeywordSub2.new.foo(key: ["hello"])
      end

      it "succeeds when the return type in the base class is a type member" do
        class GenericReturnBase
          extend T::Generic
          extend T::Sig
          Elem = type_member
          sig {overridable.returns(Elem)}
          def foo; end
        end

        class GenericReturnSub2 < GenericReturnBase
          Elem = type_member(fixed: T::Array[String])
          sig {override.returns(T::Array[String])}
          def foo
            ["greetings"]
          end
        end

        GenericReturnSub2.new.foo
      end
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

      assert_includes(err.message, "Incompatible type for arg #1 (`pos`) in override of method `foo`")
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

      assert_includes(err.message, "Incompatible type for arg `kw` in override of method `foo`")
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

      assert_includes(err.message, "Incompatible return type in override of method `foo`")
    end

  end
end
