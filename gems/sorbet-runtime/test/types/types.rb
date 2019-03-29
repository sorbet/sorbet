# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class TypesTest < Critic::Unit::UnitTest
    module Mixin1; end
    module Mixin2; end
    class Base; end
    class Sub < Base; end
    class WithMixin; include Mixin1; end

    describe "Base" do
      it "raises an error if you override `subtype_of?`" do
        err = assert_raises do
          Class.new(T::Types::Base) do
            def subtype_of?(other)
              false
            end
          end
        end
        assert_includes(err.message, "`subtype_of?` should not be overridden")
      end
    end

    describe "Simple" do
      before do
        @type = T::Utils.coerce(Integer)
      end

      it "passes a validation" do
        msg = @type.error_message_for_obj(1)
        assert_nil(msg)
      end

      it "fails a validation with a different type" do
        msg = @type.error_message_for_obj("1")
        assert_equal("Expected type Integer, got type String with value \"1\"", msg)
      end

      it "fails a validation with nil" do
        msg = @type.error_message_for_obj(nil)
        assert_equal("Expected type Integer, got type NilClass", msg)
      end

      it 'can hand back its raw type' do
        assert_equal(Integer, @type.raw_type)
      end
    end

    describe "Union" do
      before do
        @type = T.nilable(Integer)
      end

      it "passes a validation with a non-nil value" do
        msg = @type.error_message_for_obj(1)
        assert_nil(msg)
      end

      it "passes a validation with a nil value" do
        msg = @type.error_message_for_obj(nil)
        assert_nil(msg)
      end

      it "fails a validation with a different type" do
        msg = @type.error_message_for_obj("1")
        assert_equal("Expected type T.nilable(Integer), got type String with value \"1\"", msg)
      end

      it "simplifies a union containing another union" do
        type = T.any(@type, T.nilable(String))
        assert_equal("T.nilable(T.any(Integer, String))", type.name)
      end

      it 'can hand back its underlying types' do
        type = T.any(Integer, Boolean, NilClass)
        value = type.types.map(&:raw_type)
        assert_equal([Integer, Boolean, NilClass], value)
      end

      it "does not crash on anonymous classes" do
        type = T.any(Integer, Class.new, String)
        assert_equal("T.any(Integer, String)", type.name)
      end
    end

    describe "Intersection" do
      before do
        @type = T.all(Mixin1, Mixin2)
        @klass = Class.new
      end

      it "passes validation with both mixins" do
        @klass.include(Mixin1)
        @klass.include(Mixin2)
        msg = @type.error_message_for_obj(@klass.new)
        assert_nil(msg)
      end

      it "fails validation with just Mixin1" do
        @klass.include(Mixin1)
        msg = @type.error_message_for_obj(@klass.new)
        assert_match(
          /Expected type T.all\(Opus::Types::Test::TypesTest::Mixin1, Opus::Types::Test::TypesTest::Mixin2\), got type #{@klass} with hash -?\d+/,
          msg
        )
      end

      it "fails validation with just Mixin2" do
        @klass.include(Mixin2)
        msg = @type.error_message_for_obj(@klass.new)
        assert_match(
          /Expected type T.all\(Opus::Types::Test::TypesTest::Mixin1, Opus::Types::Test::TypesTest::Mixin2\), got type #{@klass} with hash -?\d+/,
          msg
        )
      end

      it "simplifies an intersection containing another intersection" do
        type = T.all(@type, T.all(Mixin1, String))
        assert_equal(
          "T.all(Opus::Types::Test::TypesTest::Mixin1, Opus::Types::Test::TypesTest::Mixin2, String)",
          type.name
        )
      end
    end

    describe "FixedArray" do
      before do
        @type = T::Utils.coerce([String, Boolean])
      end

      it "passes validation" do
        msg = @type.error_message_for_obj(["hello", true])
        assert_nil(msg)
      end

      it "fails validation with a non-array" do
        msg = @type.error_message_for_obj("hello")
        assert_equal("Expected type [String, Boolean], got type String with value \"hello\"", msg)
      end

      it "fails validation with an array of the wrong size" do
        msg = @type.error_message_for_obj(["hello", true, false])
        assert_equal("Expected type [String, Boolean], got array of size 3", msg)
      end

      it "fails validation with an array of the right size but wrong types" do
        msg = @type.error_message_for_obj(["hello", nil])
        assert_equal("Expected type [String, Boolean], got type [String, NilClass]", msg)
      end
    end

    describe "FixedHash" do
      before do
        @type = T::Utils.coerce({a: String, b: Boolean, c: T.nilable(Numeric)})
      end

      it "passes validation" do
        msg = @type.error_message_for_obj({a: "hello", b: true, c: 3})
        assert_nil(msg)
      end

      it "passes validation when nilable fields are missing" do
        msg = @type.error_message_for_obj({a: "hello", b: true})
        assert_nil(msg)
      end

      it "fails validation with a non-hash" do
        msg = @type.error_message_for_obj("hello")
        assert_equal("Expected type {a: String, b: Boolean, c: T.nilable(Numeric)}, got type String with value \"hello\"", msg)
      end

      it "fails validation with a hash of the wrong types" do
        msg = @type.error_message_for_obj({a: true, b: true, c: 3})
        assert_equal("Expected type {a: String, b: Boolean, c: T.nilable(Numeric)}, got type {a: TrueClass, b: TrueClass, c: Integer}", msg)
      end

      it "fails validation if a field is missing" do
        msg = @type.error_message_for_obj({b: true, c: 3})
        assert_equal("Expected type {a: String, b: Boolean, c: T.nilable(Numeric)}, got type {b: TrueClass, c: Integer}", msg)
      end

      it "fails validation if an extra field is present" do
        msg = @type.error_message_for_obj({a: "hello", b: true, d: "ohno"})
        assert_equal("Expected type {a: String, b: Boolean, c: T.nilable(Numeric)}, got type {a: String, b: TrueClass, d: String}", msg)
      end
    end

    describe "TypedArray" do
      it 'fails if value is not an array' do
        type = T::Array[Integer]
        value = 3
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Array[Integer], " \
                         "got type Integer with value 3"
        assert_equal(expected_error, msg)
      end

      it 'can hand back the underlying type' do
        type = T::Array[Boolean]
        assert_equal(Boolean, type.type.raw_type)
      end

      it 'fails if an element of the array is the wrong type' do
        type = T::Array[Integer]
        value = [true]
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Array[Integer], " \
                         "got T::Array[T.any(TrueClass, FalseClass)]"
        assert_equal(expected_error, msg)
      end

      it 'succeeds if all values have the correct type' do
        type = T::Array[T.any(Integer, Boolean)]
        value = [true, 3, false, 4, 5, false]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if any of the values is the wrong type' do
        type = T::Array[T.any(Integer, Boolean)]
        value = [true, 3.0, false, 4, "5", false]
        expected_error = "Expected type T::Array[T.any(Boolean, Integer)], " \
          "got T::Array[T.any(FalseClass, Float, Integer, String, TrueClass)]"
        msg = type.error_message_for_obj(value)
        assert_equal(expected_error, msg)
      end

      it 'proposes a simple type if only one type exists' do
        type = T::Array[String]
        value = [1, 2, 3]
        expected_error = "Expected type T::Array[String], " \
                         "got T::Array[Integer]"
        msg = type.error_message_for_obj(value)
        assert_equal(expected_error, msg)
      end

      it 'proposes a union type if multiple types exist' do
        type = T::Array[String]
        value = [true, false, 1]
        expected_error = "Expected type T::Array[String], " \
          "got T::Array[T.any(FalseClass, Integer, TrueClass)]"
        msg = type.error_message_for_obj(value)
        assert_equal(expected_error, msg)
      end

      it 'is covariant for the type_member for valid?' do
        type = T::Array[Numeric]
        value = [3]
        # In the static type system. This would be:
        #   "Expected type T::Array[Numeric], got T::Array[Integer]"
        # but with type erasure, we have to leave all runtime checks as
        # covariant.
        assert_nil(type.error_message_for_obj(value))
      end

      it 'is not contravariant for the type_member' do
        type = T::Array[Integer]
        value = [Object.new]
        expected_error = "Expected type T::Array[Integer], " \
          "got T::Array[Object]"
        msg = type.error_message_for_obj(value)
        assert_equal(expected_error, msg)
      end

      it 'gives the right error when passed a Hash' do
        type = T::Array[Symbol]
        msg = type.error_message_for_obj({foo: 17})
        assert_equal(
          "Expected type T::Array[Symbol], got T::Hash[Symbol, Integer]",
          msg)
      end

      it 'can have its metatype instantiated' do
        assert_equal([], T::Array[Integer].new)
        assert_equal([nil, nil, nil], T::Array[Integer].new(3))
      end

      it 'is coerced from plain array' do
        assert_equal(T::Array[T.untyped], T::Utils.coerce(::Array))
      end
    end

    describe "TypedHash" do
      it 'describes hashes containing arrays' do
        t = T::Hash[Symbol, String]
        assert_equal(
          "T::Hash[Symbol, T::Array[String]]",
          t.describe_obj({hello: ["world"]}))
      end

      it 'gives the right type error for an array' do
        type = T::Hash[Symbol, String]
        msg = type.error_message_for_obj([:foo])
        assert_equal(
          "Expected type T::Hash[Symbol, String], got T::Array[Symbol]",
          msg)
      end

      it 'can have its metatype instantiated' do
        assert_equal({}, T::Hash[Symbol, Integer].new)
        assert_equal([], T::Hash[Symbol, Integer].new {|h, k| h[k] = []}[:missing])
      end

      it 'is coerced from plain hash' do
        assert_equal(T::Hash[T.untyped, T.untyped], T::Utils.coerce(::Hash))
      end

      it 'fails if a key in the Hash is the wrong type' do
        type = T::Hash[Symbol, Integer]
        value = {
          'oops_string' => 1,
        }
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Hash[Symbol, Integer], got T::Hash[String, Integer]"
        assert_equal(expected_error, msg)
      end

      it 'fails if a value in the Hash is the wrong type' do
        type = T::Hash[Symbol, Integer]
        value = {
          sym: 1.0,
        }
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Hash[Symbol, Integer], got T::Hash[Symbol, Float]"
        assert_equal(expected_error, msg)
      end
    end

    describe "TypedRange" do
      it 'describes ranges' do
        t = T::Range[Integer]
        assert_equal(
          "T::Range[Integer]",
          t.describe_obj(10...20))
      end

      it 'works if the type is right' do
        type = T::Range[Integer]
        value = (3...10)
        msg = type.error_message_for_obj(value)
        assert_nil(msg)
      end

      it 'fails if the type is wrong' do
        type = T::Range[Float]
        value = (3...10)
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Range[Float], " \
                         "got T::Range[Integer]"
        assert_equal(expected_error, msg)
      end

      it 'can have its metatype instantiated' do
        assert_equal((3..4), T::Range[Integer].new(3, 4))
      end
    end

    describe "TypedSet" do
      it 'describes sets' do
        t = T::Set[Integer]
        assert_equal(
          "T::Set[Integer]",
          t.describe_obj(Set.new([1, 2, 3])))
      end

      it 'works if the type is right' do
        type = T::Set[Integer]
        value = Set.new([1, 2, 3])
        msg = type.error_message_for_obj(value)
        assert_nil(msg)
      end

      it 'fails if the type is wrong' do
        type = T::Set[Float]
        value = Set.new([1, 2, 3])
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Set[Float], " \
                         "got T::Set[Integer]"
        assert_equal(expected_error, msg)
      end

      it 'can have its metatype instantiated' do
        assert_equal(Set.new, T::Set[Integer].new)
        assert_equal(Set.new([3, 4]), T::Set[Integer].new([3, 4]))
      end

      it 'is coerced from plain set' do
        assert_equal(T::Set[T.untyped], T::Utils.coerce(::Set))
      end
    end


    describe "Enumerable" do
      it 'fails if value is not an enumerable' do
        type = T::Enumerable[Integer]
        value = 3
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Enumerable[Integer], " \
                         "got type Integer with value 3"
        assert_equal(expected_error, msg)
      end

      it 'can hand back the underlying type' do
        type = T::Enumerable[Boolean]
        assert_equal(Boolean, type.type.raw_type)
      end

      it 'fails if an element of the array is the wrong type' do
        type = T::Enumerable[Integer]
        value = [true]
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Enumerable[Integer], " \
                         "got T::Array[T.any(TrueClass, FalseClass)]"
        assert_equal(expected_error, msg)
      end

      it 'succeeds if all values have the correct type' do
        type = T::Enumerable[T.any(Integer, Boolean)]
        value = [true, 3, false, 4, 5, false]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if any of the values is the wrong type' do
        type = T::Enumerable[T.any(Integer, Boolean)]
        value = [true, 3.0, false, 4, "5", false]
        expected_error = "Expected type T::Enumerable[T.any(Boolean, Integer)], " \
          "got T::Array[T.any(FalseClass, Float, Integer, String, TrueClass)]"
        msg = type.error_message_for_obj(value)
        assert_equal(expected_error, msg)
      end

      it 'proposes a simple type if only one type exists' do
        type = T::Enumerable[String]
        value = [1, 2, 3]
        expected_error = "Expected type T::Enumerable[String], " \
                         "got T::Array[Integer]"
        msg = type.error_message_for_obj(value)
        assert_equal(expected_error, msg)
      end

      it 'proposes a union type if multiple types exist' do
        type = T::Enumerable[String]
        value = [true, false, 1]
        expected_error = "Expected type T::Enumerable[String], " \
          "got T::Array[T.any(FalseClass, Integer, TrueClass)]"
        msg = type.error_message_for_obj(value)
        assert_equal(expected_error, msg)
      end

      it 'wont check unrewindable enumerables' do
        type = T::Enumerable[T.any(Integer, Boolean)]
        value = File.new(__FILE__)
        assert_nil(type.error_message_for_obj(value))
      end

      it 'is covariant for the type_member' do
        type = T::Enumerable[Numeric]
        value = [3]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'is not contravariant for the type_member' do
        type = T::Enumerable[Integer]
        value = [Object.new]
        expected_error = "Expected type T::Enumerable[Integer], " \
          "got T::Array[Object]"
        msg = type.error_message_for_obj(value)
        assert_equal(expected_error, msg)
      end

      it 'does not check lazy enumerables (for now)' do
        type = T::Enumerable[Integer]
        value = ["bad"].lazy
        msg = type.error_message_for_obj(value)
        assert_nil(msg)
      end

      it 'can serialize enumerables whose each throws' do
        type = T::Enumerable[Integer]
        value = Class.new(Array) do
          def each
            raise "bad"
          end
        end.new(['str'])
        msg = type.error_message_for_obj(value)
        expected_error = "Expected type T::Enumerable[Integer], got T::Array[T.untyped]"
        assert_equal(expected_error, msg)
      end
    end

    module TestInterface1
      extend T::Helpers
      interface!

      sig {abstract.returns(T.untyped)}; def hello; end
    end

    module TestInterface2
      extend T::Helpers
      interface!

      sig {abstract.returns(T.untyped)}; def goodbye; end
    end

    class InterfaceImplementor1
      include TestInterface1

      def hello
        'hello'
      end

    end

    class InterfaceImplementor2
      include TestInterface2

      def goodbye
        'goodbye'
      end

    end

    describe "Enum" do
      before do
        @type = T.enum([:foo, :bar])
      end

      it 'passes validation with a value from the enum' do
        msg = @type.error_message_for_obj(:foo)
        assert_nil(msg)
      end

      it 'fails validation with a value not from the enum' do
        msg = @type.error_message_for_obj(:baz)
        assert_equal("Expected type T.enum([:foo, :bar]), got :baz", msg)
      end

      it 'does not coerce types' do
        msg = @type.error_message_for_obj('foo')
        assert_equal('Expected type T.enum([:foo, :bar]), got "foo"', msg)

        type = T.enum(['foo', 'bar'])
        msg = type.error_message_for_obj(:foo)
        assert_equal('Expected type T.enum(["foo", "bar"]), got :foo', msg)
      end

      it 'fails validation with a nil value' do
        msg = @type.error_message_for_obj(nil)
        assert_equal("Expected type T.enum([:foo, :bar]), got nil", msg)
      end
    end

    describe "proc" do
      before do
        @type = T::Utils.coerce(T.proc.params(x: Integer, y: Integer).returns(Integer))
      end

      it 'allows procs' do
        assert_nil(@type.error_message_for_obj(proc {|x, y| x + y}))
      end

      it 'allows lambdas' do
        assert_nil(@type.error_message_for_obj(->(x, y) {x + y}))
      end

      it 'disallows custom callables' do
        k = Class.new do
          def call(x, y)
            x + y
          end
        end

        callable = k.new
        msg = @type.error_message_for_obj(callable)
        assert_match(/Expected type T.proc/, msg)
      end

      it 'pretty-prints' do
        assert_equal("T.proc.params(x: Integer, y: Integer).returns(Integer)",
                     @type.name)
      end

      it 'requires a return type' do
        ex = assert_raises do
          T::Utils.coerce(T.proc.params(x: Integer))
        end

        assert_includes(ex.message, "Procs must specify a return type")
      end

      it 'cannot be mutated after coercion' do
        proto = T.proc.params(x: Integer).returns(Integer)
        T::Utils.coerce(proto)
        ex = assert_raises do
          proto.returns(String)
        end
        assert_includes(ex.message, "You can't modify a signature")
      end

      it 'cannot be abstract' do
        ex = assert_raises do
          T::Utils.coerce(T.proc.params(x: Integer).abstract.returns(String))
        end

        assert_includes(ex.message, "Procs cannot have override/abstract modifiers")
      end
    end

    describe "class_of" do
      before do
        @type = T.class_of(Base)
      end

      it 'passes validation with a subclass' do
        msg = @type.error_message_for_obj(Sub)
        assert_nil(msg)
      end

      it 'passes validation with the class itself' do
        msg = @type.error_message_for_obj(Base)
        assert_nil(msg)
      end

      it 'fails validation with some other class' do
        msg = @type.error_message_for_obj(InterfaceImplementor1)
        assert_equal("Expected type T.class_of(Opus::Types::Test::TypesTest::Base), got Opus::Types::Test::TypesTest::InterfaceImplementor1", msg)
      end

      it 'fails validation with a non-class' do
        msg = @type.error_message_for_obj(1)
        assert_equal('Expected type T.class_of(Opus::Types::Test::TypesTest::Base), got 1', msg)

        msg = @type.error_message_for_obj(Mixin1)
        assert_equal('Expected type T.class_of(Opus::Types::Test::TypesTest::Base), got Opus::Types::Test::TypesTest::Mixin1', msg)
      end

      it 'fails validation with a nil value' do
        msg = @type.error_message_for_obj(nil)
        assert_equal('Expected type T.class_of(Opus::Types::Test::TypesTest::Base), got nil', msg)
      end

      it 'fails validation with when supplied an instance of the subclass' do
        msg = @type.error_message_for_obj(Sub.new)
        assert_match(/Expected type T.class_of\(Opus::Types::Test::TypesTest::Base\), got/, msg)
      end

      it 'works for a mixin' do
        type = T.class_of(Mixin1)
        msg = type.error_message_for_obj(WithMixin)
        assert_nil(msg)
      end

      it 'can not just be .singleton_class' do
        type = T::Utils.coerce(Mixin1.singleton_class)
        msg = type.error_message_for_obj(WithMixin)
        assert_equal('Expected type , got type Class with value Opus::Types::Test::TypesTest::WithMixin', msg)
      end
    end

    describe "subtype_of?" do
      def subtype?(lhs, rhs)
        T::Utils.coerce(lhs).subtype_of?(T::Utils.coerce(rhs))
      end

      def any(*args)
        T.any(*args)
      end

      def all(*args)
        T.all(*args)
      end

      def assert_subtype(lraw, rraw)
        lhs = T::Utils.coerce(lraw)
        rhs = T::Utils.coerce(rraw)

        assert_equal(true, lhs.subtype_of?(rhs), "Expected #{lhs} to be a subtype of #{rhs}")
      end

      def refute_subtype(lhs, rhs)
        lt = T::Utils.coerce(lhs)
        rt = T::Utils.coerce(rhs)

        refute(lt.subtype_of?(rt), "Expected #{lhs} not to be a subtype of #{rhs}")
      end

      describe "simple types" do
        it "returns true for the same type" do
          assert_equal(true, subtype?(Base, Base))
        end

        it "returns true for a subclass" do
          assert_equal(true, subtype?(Sub, Base))
        end

        it "returns false for a superclass" do
          assert_equal(false, subtype?(Base, Sub))
        end

        it "returns nil for an unrelated class" do
          assert_nil(subtype?(Base, Integer))
        end
      end

      describe "unions" do
        it "returns true for a single member" do
          assert_equal(true, subtype?(Integer, any(Float, String, Integer)))
        end

        it "returns true for a subset" do
          assert_equal(true, subtype?(any(Integer, Float), any(Float, String, Integer)))
        end

        it "returns true for a subset involving a non-Simple type" do
          assert_equal(true, subtype?(any(Sub, T::Array[String]), any(Sub, T::Array[String])))
        end

        it "returns false for a superset" do
          assert_equal(false, subtype?(any(Integer, Float, String), any(Integer, Float)))
        end

        it "returns false for a superset involving a non-Simple type" do
          assert_equal(false, subtype?(any(Integer, T::Array[String], String), any(Integer, T::Array[String])))
        end
      end

      describe "intersections" do
        it "returns false for a single member" do
          assert_equal(false, subtype?(Integer, all(Float, String, Integer)))
        end

        it "returns false for a subset" do
          assert_equal(false, subtype?(all(Integer, Float), all(Float, String, Integer)))
        end

        it "returns false for a subset involving a non-Simple type" do
          assert_equal(false, subtype?(all(Integer, T::Array[String]), all(T::Array[String], String, Integer)))
        end

        it "returns true for a superset involving a non-Simple type" do
          assert_equal(true, subtype?(all(Integer, T::Array[String], String), all(Integer, T::Array[String])))
        end
      end

      describe "unions and intersections together" do
        it "returns true for equivalent combinations" do
          # NB: For this to pass, we would need to normalize unions and intersections.
          # Dmitry is looking into this, but we may just punt.
          #
          # assert_equal(
          #   true,
          #   subtype?(
          #     all(any(Integer, Float), T::Array[String]),
          #     any(all(Integer, T::Array[String]), all(Float, T::Array[String]))
          #   )
          # )

          # Same as above but with order reversed
          assert_equal(
            true,
            subtype?(
              any(all(Integer, T::Array[String]), all(Float, T::Array[String])),
              all(any(Integer, Float), T::Array[String])
            )
          )
        end
      end

      describe "procs" do
        it 'is a subtype of itself' do
          p1 = T::Utils.coerce(T.proc.params(x: Integer, y: Integer).returns(Integer))
          p2 = T::Utils.coerce(T.proc.params(x: Integer, y: Integer).returns(Integer))
          assert_subtype(p1, p1)
          assert_subtype(p1, p2)
        end

        it 'ignores argument names' do
          p1 = T::Utils.coerce(T.proc.params(x: Integer, y: Integer).returns(Integer))
          p2 = T::Utils.coerce(T.proc.params(w: Integer, z: Integer).returns(Integer))
          assert_subtype(p1, p1)
          assert_subtype(p1, p2)
        end

        it 'arity must match' do
          p1 = T::Utils.coerce(T.proc.params(x: Integer, y: Integer).returns(Integer))
          p2 = T::Utils.coerce(T.proc.params(x: Integer).returns(Integer))
          refute_subtype(p1, p2)
          refute_subtype(p2, p1)
        end

        it 'args are contravariant' do
          p1 = T::Utils.coerce(T.proc.params(x: Integer, y: String).returns(Integer))
          p2 = T::Utils.coerce(T.proc.params(x: Numeric, y: String).returns(Integer))
          refute_subtype(p1, p2)
          assert_subtype(p2, p1)
        end

        it 'return types are covariant' do
          p1 = T::Utils.coerce(T.proc.params(x: Integer, y: Integer).returns(Integer))
          p2 = T::Utils.coerce(T.proc.params(x: Integer, y: Integer).returns(Numeric))
          assert_subtype(p1, p2)
          refute_subtype(p2, p1)
        end
      end

      describe "enumerable" do
        it "allows the obvious things" do
          assert_subtype(T::Enumerable[String], T::Enumerable[String])
          assert_subtype(T::Array[String], T::Enumerable[String])
          assert_subtype(T::Array[String], T::Array[String])
          assert_subtype(T::Hash[String, String], T::Enumerable[[String, String]])

          refute_subtype(T::Enumerable[String], T::Array[String])
          refute_subtype(T::Hash[String, String], T::Enumerable[String])
          assert_subtype(T::Hash[String, Symbol], T::Enumerable[[Object, Object]])
        end

        it "has the right variance" do
          assert_subtype(T::Enumerable[Integer], T::Enumerable[Numeric])
          refute_subtype(T::Enumerable[Numeric], T::Enumerable[Integer])
        end
      end

      describe "array" do
        it "is covariant" do
          assert_subtype(T::Array[Integer], T::Array[Numeric])
          refute_subtype(T::Array[Numeric], T::Array[Integer])
        end
      end

      describe "hash" do
        it "is covariant" do
          assert_subtype(T::Hash[Integer, Object], T::Hash[Numeric, Object])
          assert_subtype(T::Hash[Object, Integer], T::Hash[Object, Numeric])
          refute_subtype(T::Hash[Numeric, Object], T::Hash[Integer, Object])
          refute_subtype(T::Hash[Object, Numeric], T::Hash[Object, Integer])
        end
      end

      describe "class_of" do
        it "returns true for itself" do
          assert_equal(true, subtype?(T.class_of(Base), T.class_of(Base)))
        end

        it "returns true for a subclass" do
          assert_equal(true, subtype?(T.class_of(Sub), T.class_of(Base)))
        end

        it "returns false for a superclass" do
          assert_equal(false, subtype?(T.class_of(Base), T.class_of(Sub)))
        end
      end

      describe 'tuples' do
        it 'Is covariant' do
          assert_subtype([Integer, String], [Numeric, String])
          assert_subtype([String, Integer], [String, Numeric])

          refute_subtype([Numeric, String], [Integer, String])
          refute_subtype([String, Numeric], [String, Integer])
          refute_subtype([String], [String, Object])
        end
      end

      describe 'untyped' do
        it 'is a subtype of things' do
          assert_subtype(T.untyped, Integer)
          assert_subtype(T.untyped, Numeric)
          assert_subtype(T.untyped, [String, String])
          assert_subtype(T.untyped, T::Array[Integer])
        end

        it 'other things are a subtype of it' do
          assert_subtype(Integer, T.untyped)
          assert_subtype(Numeric, T.untyped)
          assert_subtype([String, String], T.untyped)
          assert_subtype(T::Array[Integer], T.untyped)
        end
      end

      describe 'type variables' do
        it 'type members are subtypes of everything' do
          # rubocop:disable PrisonGuard/UseOpusTypesShortcut
          assert_subtype(T::Types::TypeMember.new(:in), T.untyped)
          assert_subtype(T::Types::TypeMember.new(:in), String)
          assert_subtype(T::Types::TypeMember.new(:in),
                         T::Types::TypeMember.new(:out))
          # rubocop:enable PrisonGuard/UseOpusTypesShortcut
        end

        it 'type parameters are subtypes of everything' do
          # rubocop:disable PrisonGuard/UseOpusTypesShortcut
          assert_subtype(T::Types::TypeParameter.new(:T), T.untyped)
          assert_subtype(T::Types::TypeParameter.new(:T), String)
          assert_subtype(T::Types::TypeParameter.new(:T),
                         T::Types::TypeParameter.new(:V))
          # rubocop:enable PrisonGuard/UseOpusTypesShortcut
        end
      end
    end

    module TestGeneric1
      extend T::Generic

      Elem = type_member
      sig {params(bar: Elem).returns(Elem)}
      def self.foo(bar)
        bar
      end
    end

    class TestGeneric2
      extend T::Generic

      Elem1 = type_member
      Elem2 = type_member
      sig {params(bar: Elem1, baz: Elem2).returns([Elem1, Elem2])}
      def foo(bar, baz)
        [bar, baz]
      end
    end

    class GenericSingleton
      extend T::Generic

      SingletonTP = type_template
      sig {params(arg: SingletonTP).returns(SingletonTP)}
      def self.foo(arg)
        arg
      end
    end

    class GenericSingletonChild < GenericSingleton
      SignletonTP = type_template(fixed: String)
    end




    describe "generics" do
      it 'simply works' do
        assert_equal(:sym, TestGeneric1[Symbol].foo(:sym))
      end

      it 'works with wrong types (at runtime)' do
        assert_equal(:sym, TestGeneric1[String].foo(:sym))
      end

      it 'works with too many types (at runtime)' do
        assert_equal(:sym, TestGeneric1[Symbol, String].foo(:sym))
      end

      it 'works with two types' do
        assert_equal([:sym, 'string'], TestGeneric2[Symbol, String].new.foo(:sym, 'string'))
      end

      it 'works for generic singleton classes' do
        assert_equal(:sym, GenericSingletonChild.foo(:sym))
      end
    end
  end
end
