# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class TypesTest < Critic::Unit::UnitTest
    module Mixin1; end
    module Mixin2; end
    class Base; end
    class Sub < Base; end
    class WithMixin; include Mixin1; end
    class ReloadedClass; end

    private def counting_allocations
      before = GC.stat[:total_allocated_objects]
      yield
      GC.stat[:total_allocated_objects] - before - 1 # Subtract one for the allocation by GC.stat itself
    end

    private def check_alloc_counts
      @check_alloc_counts = Gem::Version.new(RUBY_VERSION) < Gem::Version.new('3.0')
    end

    # this checks that both the recursive and nonrecursive path should
    # have the same behavior
    private def check_error_message_for_obj(type, value)
      nonrecursive_msg = type.error_message_for_obj(value)
      recursive_msg = type.error_message_for_obj_recursive(value)
      assert(
        nonrecursive_msg == recursive_msg,
        "Differing output from valid? and recursively_valid?: #{nonrecursive_msg.inspect} != #{recursive_msg.inspect}",
      )
      nonrecursive_msg
    end

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
        msg = check_error_message_for_obj(@type, 1)
        assert_nil(msg)
      end

      it "fails a validation with a different type" do
        msg = check_error_message_for_obj(@type, "1")
        assert_equal("Expected type Integer, got type String with value \"1\"", msg)
      end

      it "fails a validation with nil" do
        msg = check_error_message_for_obj(@type, nil)
        assert_equal("Expected type Integer, got type NilClass", msg)
      end

      it 'can hand back its raw type' do
        assert_equal(Integer, @type.raw_type)
      end

      it 'valid? does not allocate' do
        skip unless check_alloc_counts
        allocs_when_valid = counting_allocations {@type.valid?(0)}
        assert_equal(0, allocs_when_valid)

        allocs_when_invalid = counting_allocations {@type.valid?(0.1)}
        assert_equal(0, allocs_when_invalid)
      end

      it 'uses structural, not reference, equality' do
        x = T::Types::Simple.new(String)
        y = T::Types::Simple.new(String)
        refute_equal(x.object_id, y.object_id)
        assert_equal(x, y)
        assert_equal(x.hash, y.hash)
      end

      it 'pools correctly' do
        x = T::Types::Simple::Private::Pool.type_for_module(String)
        y = T::Types::Simple::Private::Pool.type_for_module(String)
        assert_equal(x.object_id, y.object_id)
      end

      it 'does not blow up when pooling with frozen module' do
        m = Module.new
        m.freeze

        x = T::Types::Simple::Private::Pool.type_for_module(m)
        assert_instance_of(T::Types::Simple, x)
      end

      it 'does not add any instance variables to the module' do
        m = Module.new
        ivars = m.instance_variables

        _ = T::Types::Simple::Private::Pool.type_for_module(m)
        assert_equal(ivars, m.instance_variables)
      end

      it "shows a detailed error for constant reloading problems" do
        klass = ReloadedClass

        expected_id = ReloadedClass.__id__
        type = T::Utils.coerce(ReloadedClass)
        Opus::Types::Test::TypesTest.send(:remove_const, :ReloadedClass)

        Opus::Types::Test::TypesTest.const_set(:ReloadedClass, Class.new)
        actual_id = ReloadedClass.__id__
        obj = ReloadedClass.new
        Opus::Types::Test::TypesTest.send(:remove_const, :ReloadedClass)

        msg = check_error_message_for_obj(type, obj)
        assert_equal(<<~MSG.strip, msg)
          Expected type #{klass}, got type #{klass} with hash #{obj.hash}

          The expected type and received object type have the same name but refer to different constants.
          Expected type is #{klass} with object id #{expected_id}, but received type is #{klass} with object id #{actual_id}.

          There might be a constant reloading problem in your application.
        MSG
      end
    end

    describe "Union" do
      describe "with simple nilable type" do
        before do
          @type = T.nilable(Integer)
        end

        it "passes a validation with a non-nil value" do
          msg = check_error_message_for_obj(@type, 1)
          assert_nil(msg)
        end

        it "passes a validation with a nil value" do
          msg = check_error_message_for_obj(@type, nil)
          assert_nil(msg)
        end

        it "fails a validation with a different type" do
          msg = check_error_message_for_obj(@type, "1")
          assert_equal("Expected type T.nilable(Integer), got type String with value \"1\"", msg)
        end

        it 'valid? does not allocate' do
          skip unless check_alloc_counts
          allocs_when_valid = counting_allocations {@type.valid?(0)}
          assert_equal(0, allocs_when_valid)

          allocs_when_invalid = counting_allocations {@type.valid?(0.1)}
          assert_equal(0, allocs_when_invalid)
        end

        it 'can hand back its underlying types' do
          value = @type.types.map(&:raw_type)
          assert_equal([Integer, NilClass], value)
        end
      end

      describe "with complex type" do
        before do
          @type = T.nilable(T.any(Integer, T::Boolean))
        end

        it "passes a validation with a non-nil value" do
          msg = check_error_message_for_obj(@type, 1)
          assert_nil(msg)
        end

        it "passes a validation with a nil value" do
          msg = check_error_message_for_obj(@type, nil)
          assert_nil(msg)
        end

        it "fails a validation with a different type" do
          msg = check_error_message_for_obj(@type, "1")
          assert_equal("Expected type T.nilable(T.any(Integer, T::Boolean)), got type String with value \"1\"", msg)
        end

        it 'valid? does not allocate' do
          skip unless check_alloc_counts
          allocs_when_valid = counting_allocations {@type.valid?(0)}
          assert_equal(0, allocs_when_valid)

          allocs_when_invalid = counting_allocations {@type.valid?(0.1)}
          assert_equal(0, allocs_when_invalid)
        end

        it 'can hand back its underlying types' do
          value = @type.types.map(&:raw_type)
          assert_equal([Integer, TrueClass, FalseClass, NilClass], value)
        end
      end

      it "simplifies a union containing another union" do
        type = T.any(T.nilable(Integer), T.nilable(String))
        assert_equal("T.nilable(T.any(Integer, String))", type.name)
      end

      it "simplifies doubly-nested union" do
        type = T.any(T.nilable(T.any(Integer, Float)), String)
        assert_equal("T.nilable(T.any(Float, Integer, String))", type.name)
      end

      it "does not crash on anonymous classes" do
        type = T.any(Integer, Class.new, String)
        assert_equal("T.any(Integer, String)", type.name)
      end

      it "unwraps aliased types" do
        type = T.any(String, Integer, T::Private::Types::TypeAlias.new(-> {Integer}))
        assert_equal("T.any(Integer, String)", type.name)
      end

      it 'uses structural, not reference, equality' do
        x = T::Types::Union.new([String, NilClass])
        y = T::Types::Union.new([String, NilClass])
        refute_equal(x.object_id, y.object_id)
        assert_equal(x, y)
        assert_equal(x.hash, y.hash)
      end

      it 'handles equality between simple pair and complex unions' do
        x = T::Types::Union.new([String, NilClass])
        y = T::Private::Types::SimplePairUnion.new(
          T::Utils.coerce(String),
          T::Utils.coerce(NilClass),
        )
        assert_equal(x, y)
        assert_equal(x.hash, y.hash)
      end

      it 'pools correctly for simple nilable types' do
        x = T::Types::Union::Private::Pool.union_of_types(
          T::Utils.coerce(String),
          T::Utils.coerce(NilClass),
        )
        y = T::Types::Union::Private::Pool.union_of_types(
          T::Utils.coerce(String),
          T::Utils.coerce(NilClass),
        )
        assert_equal(x.object_id, y.object_id)
        assert_equal(
          T::Private::Types::SimplePairUnion,
          x.class,
        )
      end

      it 'uses fast path for T::Boolean' do
        assert_equal(
          T::Private::Types::SimplePairUnion,
          T::Boolean.aliased_type.class,
        )
      end

      it 'deduplicates type, fast path' do
        assert_equal(
          'Integer',
          T.any(Integer, Integer).name,
        )
      end

      it 'deduplicates type, slow path' do
        assert_equal(
          'T.all(Opus::Types::Test::TypesTest::Mixin1, Opus::Types::Test::TypesTest::Mixin2)',
          T.any(T.all(Mixin1, Mixin2), T.all(Mixin1, Mixin2)).name,
        )
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
        msg = check_error_message_for_obj(@type, @klass.new)
        assert_nil(msg)
      end

      it "fails validation with just Mixin1" do
        @klass.include(Mixin1)
        msg = check_error_message_for_obj(@type, @klass.new)
        assert_match(
          /Expected type T.all\(Opus::Types::Test::TypesTest::Mixin1, Opus::Types::Test::TypesTest::Mixin2\), got type #{@klass} with hash -?\d+/,
          msg
        )
      end

      it "fails validation with just Mixin2" do
        @klass.include(Mixin2)
        msg = check_error_message_for_obj(@type, @klass.new)
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

      it "unwraps aliased types" do
        type_alias = T::Private::Types::TypeAlias.new(-> {Mixin1})
        type = T.all(@type, type_alias)
        assert_equal("T.all(Opus::Types::Test::TypesTest::Mixin1, Opus::Types::Test::TypesTest::Mixin2)", type.name)
      end

      it 'valid? does not allocate' do
        skip unless check_alloc_counts
        @klass.include(Mixin1)
        @klass.include(Mixin2)

        allocs_when_valid = counting_allocations {@type.valid?(@klass)}
        assert_equal(0, allocs_when_valid)

        allocs_when_invalid = counting_allocations {@type.valid?(Mixin1)}
        assert_equal(0, allocs_when_invalid)
      end
    end

    describe "FixedArray" do
      before do
        @type = T::Utils.coerce([String, T::Boolean])
      end

      it "passes validation" do
        msg = check_error_message_for_obj(@type, ["hello", true])
        assert_nil(msg)
      end

      it "fails validation with a non-array" do
        msg = check_error_message_for_obj(@type, "hello")
        assert_equal("Expected type [String, T::Boolean], got type String with value \"hello\"", msg)
      end

      it "fails validation with an array of the wrong size" do
        msg = check_error_message_for_obj(@type, ["hello", true, false])
        assert_equal("Expected type [String, T::Boolean], got array of size 3", msg)
      end

      it "fails validation with an array of the right size but wrong types" do
        msg = check_error_message_for_obj(@type, ["hello", nil])
        assert_equal("Expected type [String, T::Boolean], got type [String, NilClass]", msg)
      end

      it 'valid? does not allocate' do
        skip unless check_alloc_counts
        arr = ["foo", false]
        allocs_when_valid = counting_allocations {@type.valid?(arr)}
        assert_equal(0, allocs_when_valid)

        arr = [1, 2]
        allocs_when_invalid = counting_allocations {@type.valid?(arr)}
        assert_equal(0, allocs_when_invalid)
      end
    end

    describe "FixedHash" do
      before do
        @type = T::Utils.coerce({a: String, b: T::Boolean, c: T.nilable(Numeric)})
      end

      it "passes validation" do
        msg = check_error_message_for_obj(@type, {a: "hello", b: true, c: 3})
        assert_nil(msg)
      end

      it "passes validation when nilable fields are missing" do
        msg = check_error_message_for_obj(@type, {a: "hello", b: true})
        assert_nil(msg)
      end

      it "fails validation with a non-hash" do
        msg = check_error_message_for_obj(@type, "hello")
        assert_equal("Expected type {a: String, b: T::Boolean, c: T.nilable(Numeric)}, got type String with value \"hello\"", msg)
      end

      it "fails validation with a hash of the wrong types" do
        msg = check_error_message_for_obj(@type, {a: true, b: true, c: 3})
        assert_equal("Expected type {a: String, b: T::Boolean, c: T.nilable(Numeric)}, got type {a: TrueClass, b: TrueClass, c: Integer}", msg)
      end

      it "fails validation with a hash of wrong typed keys" do
        msg = check_error_message_for_obj(@type, {"a" => true, :"foo bar" => true, :foo => 3})
        assert_equal("Expected type {a: String, b: T::Boolean, c: T.nilable(Numeric)}, got type {\"a\" => TrueClass, :\"foo bar\" => TrueClass, foo: Integer}", msg)
      end

      it "fails validation if a field is missing" do
        msg = check_error_message_for_obj(@type, {b: true, c: 3})
        assert_equal("Expected type {a: String, b: T::Boolean, c: T.nilable(Numeric)}, got type {b: TrueClass, c: Integer}", msg)
      end

      it "fails validation if an extra field is present" do
        msg = check_error_message_for_obj(@type, {a: "hello", b: true, d: "ohno"})
        assert_equal("Expected type {a: String, b: T::Boolean, c: T.nilable(Numeric)}, got type {a: String, b: TrueClass, d: String}", msg)
      end

      it 'valid? does not allocate' do
        skip unless check_alloc_counts
        h = {a: 'foo', b: false, c: nil}
        allocs_when_valid = counting_allocations {@type.valid?(h)}
        assert_equal(0, allocs_when_valid)

        h = {a: 'foo', b: 1, c: nil}
        allocs_when_invalid_type = counting_allocations {@type.valid?(h)}
        assert_equal(0, allocs_when_invalid_type)

        h = {a: 'foo', b: false, c: nil, d: 'nope'}
        allocs_when_invalid_key = counting_allocations {@type.valid?(h)}
        assert_equal(0, allocs_when_invalid_key)
      end
    end

    describe "TypedArray" do
      class TestEnumerable
        include Enumerable

        def each;
          yield "something";
        end
      end

      it 'fails if value is not an array' do
        type = T::Array[Integer]
        value = 3
        msg = check_error_message_for_obj(type, value)
        expected_error = "Expected type T::Array[Integer], " \
                         "got type Integer with value 3"
        assert_equal(expected_error, msg)
      end

      it 'can hand back the underlying type' do
        type = T::Array[Integer]
        assert_equal(Integer, type.type.raw_type)
      end

      it 'does not fail if an element of the array is the wrong type under shallow checking' do
        type = T::Array[Integer]
        value = [true]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if an element of the array is the wrong type under deep checking' do
        type = T::Array[Integer]
        value = [true]
        msg = type.error_message_for_obj_recursive(value)
        expected_error = "Expected type T::Array[Integer], " \
                         "got T::Array[T::Boolean]"
        assert_equal(expected_error, msg)
      end

      it 'succeeds if all values have the correct type' do
        type = T::Array[T.any(Integer, T::Boolean)]
        value = [true, 3, false, 4, 5, false]
        assert_nil(check_error_message_for_obj(type, value))
      end

      it 'does not fail if any of the values is the wrong type under shallow checking' do
        type = T::Array[T.any(Integer, T::Boolean)]
        value = [true, 3.0, false, 4, "5", false]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if any of the values is the wrong type under deep checking' do
        type = T::Array[T.any(Integer, T::Boolean)]
        value = [true, 3.0, false, 4, "5", false]
        expected_error = "Expected type T::Array[T.any(Integer, T::Boolean)], " \
          "got T::Array[T.any(Float, Integer, String, T::Boolean)]"
        msg = type.error_message_for_obj_recursive(value)
        assert_equal(expected_error, msg)
      end

      it 'does not proposes anything under shallow checking' do
        type = T::Array[String]
        value = [1, 2, 3]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'proposes a simple type if only one type exists under deep checking' do
        type = T::Array[String]
        value = [1, 2, 3]
        expected_error = "Expected type T::Array[String], " \
                         "got T::Array[Integer]"
        msg = type.error_message_for_obj_recursive(value)
        assert_equal(expected_error, msg)
      end

      it 'does not proposes a union type under shallow checking' do
        type = T::Array[String]
        value = [true, false, 1]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'proposes a union type if multiple types exist under deep checking' do
        type = T::Array[String]
        value = [true, false, 1]
        expected_error = "Expected type T::Array[String], " \
          "got T::Array[T.any(Integer, T::Boolean)]"
        msg = type.error_message_for_obj_recursive(value)
        assert_equal(expected_error, msg)
      end

      it 'is covariant for the type_member for valid?' do
        type = T::Array[Numeric]
        value = [3]
        # In the static type system. This would be:
        #   "Expected type T::Array[Numeric], got T::Array[Integer]"
        # but with type erasure, we have to leave all runtime checks as
        # covariant.
        assert_nil(check_error_message_for_obj(type, value))
      end

      it 'does not care about contravariance for the type_member under shallow checking' do
        type = T::Array[Integer]
        value = [Object.new]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'is not contravariant for the type_member under deep checking' do
        type = T::Array[Integer]
        value = [Object.new]
        expected_error = "Expected type T::Array[Integer], " \
          "got T::Array[Object]"
        msg = type.error_message_for_obj_recursive(value)
        assert_equal(expected_error, msg)
      end

      it 'gives the right error when passed a Hash' do
        type = T::Array[Symbol]
        msg = check_error_message_for_obj(type, {foo: 17})
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

      it 'valid? does not allocate' do
        skip unless check_alloc_counts
        type = T::Array[Integer]
        valid = [1]
        invalid = {}

        allocs_when_valid = counting_allocations {type.valid?(valid)}
        assert_equal(0, allocs_when_valid)

        allocs_when_invalid = counting_allocations {type.valid?(invalid)}
        assert_equal(0, allocs_when_invalid)
      end

      it 'gives the right error when passed an unrelated enumerable' do
        type = T::Array[String]
        msg = check_error_message_for_obj(type, TestEnumerable.new)
        assert_equal(
          "Expected type T::Array[String], got Opus::Types::Test::TypesTest::TestEnumerable",
          msg)
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
        msg = check_error_message_for_obj(type, [:foo])
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

      it 'does not fail if a key in the Hash is the wrong type under shallow checking' do
        type = T::Hash[Symbol, Integer]
        value = {
          'oops_string' => 1,
        }
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if a key in the Hash is the wrong type under deep checking' do
        type = T::Hash[Symbol, Integer]
        value = {
          'oops_string' => 1,
        }
        msg = type.error_message_for_obj_recursive(value)
        expected_error = "Expected type T::Hash[Symbol, Integer], got T::Hash[String, Integer]"
        assert_equal(expected_error, msg)
      end

      it 'does not fail if a value in the Hash is the wrong type under shallow checking' do
        type = T::Hash[Symbol, Integer]
        value = {
          sym: 1.0,
        }
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if a value in the Hash is the wrong type under deep checking' do
        type = T::Hash[Symbol, Integer]
        value = {
          sym: 1.0,
        }
        msg = type.error_message_for_obj_recursive(value)
        expected_error = "Expected type T::Hash[Symbol, Integer], got T::Hash[Symbol, Float]"
        assert_equal(expected_error, msg)
      end

      it 'valid? does not allocate' do
        skip unless check_alloc_counts
        type = T::Hash[String, Integer]
        valid = {'one' => 1}
        invalid = []

        allocs_when_valid = counting_allocations {type.valid?(valid)}
        assert_equal(0, allocs_when_valid)

        allocs_when_invalid = counting_allocations {type.valid?(invalid)}
        assert_equal(0, allocs_when_invalid)
      end
    end

    describe "TypedEnumerator" do
      it 'describes enumerators' do
        t = T::Enumerator[Integer]
        assert_equal(
          "T::Enumerator[Integer]",
          t.describe_obj([1, 2, 3].each))
      end

      it 'works if the type is right' do
        type = T::Enumerator[Integer]
        value = [1, 2, 3].each
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'can have its metatype instantiated' do
        assert_equal([1, 2, 3], T::Enumerator[Integer].new do |x|
          x << 1
          x << 2
          x << 3
        end.to_a)
      end
    end

    describe "TypedEnumeratorLazy" do
      it 'describes enumerators' do
        t = T::Enumerator::Lazy[Integer]
        assert_equal(
          "T::Enumerator::Lazy[Integer]",
          t.describe_obj([1, 2, 3].each.lazy))
      end

      it 'works if the type is right' do
        type = T::Enumerator::Lazy[Integer]
        value = [1, 2, 3].each.lazy
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'can have its metatype instantiated' do
        assert_equal([2, 4, 6], T::Enumerator::Lazy[Integer].new([1, 2, 3]) do |yielder, value|
          yielder << value * 2
        end.to_a)
      end
    end

    describe "TypedEnumeratorChain" do
      it 'describes enumerators' do
        t = T::Enumerator::Chain[Integer]
        assert_equal(
          "T::Enumerator::Chain[Integer]",
          t.describe_obj([1, 2].chain([3])))
      end

      it 'works if the type is right' do
        type = T::Enumerator::Chain[Integer]
        value = [1, 2].chain([3])
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'can have its metatype instantiated' do
        assert_equal([2, 4, 6], T::Enumerator::Chain[Integer].new([1, 2], [3]).map do |value|
          value * 2
        end.to_a)
      end
    end

    describe "TypedRange" do
      it 'describes ranges' do
        t = T::Range[Integer]
        assert_equal(
          "T::Range[Integer]",
          t.describe_obj(10...20))
      end

      it 'works if the range has a start and end' do
        type = T::Range[Integer]
        value = (3...10)
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      # Ruby 2.6 does not support ranges with boundless starts
      if RUBY_VERSION >= '2.7'
        it 'works if the range has no beginning' do
          type = T::Range[Integer]
          value = (nil...10)
          msg = check_error_message_for_obj(type, value)
          assert_nil(msg)
        end
      end

      it 'works if the range has no end' do
        type = T::Range[Integer]
        value = (1...nil)
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'works if the range has no start or end' do
        type = T::Range[T.untyped]
        value = (nil...nil)
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'does not fail if the type is wrong under shallow checking' do
        type = T::Range[Float]
        value = (3...10)
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if the type is wrong under deep checking' do
        type = T::Range[Float]
        value = (3...10)
        msg = type.error_message_for_obj_recursive(value)
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
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'does not fail if the type is wrong under shallow checking' do
        type = T::Set[Float]
        value = Set.new([1, 2, 3])
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if the type is wrong under deep checking' do
        type = T::Set[Float]
        value = Set.new([1, 2, 3])
        msg = type.error_message_for_obj_recursive(value)
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
        msg = check_error_message_for_obj(type, value)
        expected_error = "Expected type T::Enumerable[Integer], " \
                         "got type Integer with value 3"
        assert_equal(expected_error, msg)
      end

      it 'can hand back the underlying type' do
        type = T::Enumerable[Integer]
        assert_equal(Integer, type.type.raw_type)
      end

      it 'does not fail if an element of the array is the wrong type under shallow checking' do
        type = T::Enumerable[Integer]
        value = [true]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if an element of the array is the wrong type under deep checking' do
        type = T::Enumerable[Integer]
        value = [true]
        msg = type.error_message_for_obj_recursive(value)
        expected_error = "Expected type T::Enumerable[Integer], " \
                         "got T::Array[T::Boolean]"
        assert_equal(expected_error, msg)
      end

      it 'succeeds if all values have the correct type' do
        type = T::Enumerable[T.any(Integer, T::Boolean)]
        value = [true, 3, false, 4, 5, false]
        assert_nil(check_error_message_for_obj(type, value))
      end

      it 'does not fail if any of the values is the wrong type under shallow checking' do
        type = T::Enumerable[T.any(Integer, T::Boolean)]
        value = [true, 3.0, false, 4, "5", false]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'fails if any of the values is the wrong type under deep checking' do
        type = T::Enumerable[T.any(Integer, T::Boolean)]
        value = [true, 3.0, false, 4, "5", false]
        expected_error = "Expected type T::Enumerable[T.any(Integer, T::Boolean)], " \
          "got T::Array[T.any(Float, Integer, String, T::Boolean)]"
        msg = type.error_message_for_obj_recursive(value)
        assert_equal(expected_error, msg)
      end

      it 'does not propose a simple type under shallow checking' do
        type = T::Enumerable[String]
        value = [1, 2, 3]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'proposes a simple type if only one type exists under deep checking' do
        type = T::Enumerable[String]
        value = [1, 2, 3]
        expected_error = "Expected type T::Enumerable[String], " \
                         "got T::Array[Integer]"
        msg = type.error_message_for_obj_recursive(value)
        assert_equal(expected_error, msg)
      end

      it 'does not propose a union type if multiple types exist under shallow checking' do
        type = T::Enumerable[String]
        value = [true, false, 1]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'proposes a union type if multiple types exist under deep checking' do
        type = T::Enumerable[String]
        value = [true, false, 1]
        expected_error = "Expected type T::Enumerable[String], " \
          "got T::Array[T.any(Integer, T::Boolean)]"
        msg = type.error_message_for_obj_recursive(value)
        assert_equal(expected_error, msg)
      end

      it 'wont check unrewindable enumerables' do
        type = T::Enumerable[T.any(Integer, T::Boolean)]
        value = File.new(__FILE__)
        assert_nil(check_error_message_for_obj(type, value))
      end

      it 'is covariant for the type_member' do
        type = T::Enumerable[Numeric]
        value = [3]
        assert_nil(check_error_message_for_obj(type, value))
      end

      it 'does not care about contravariance for the type_member under shallow checking' do
        type = T::Enumerable[Integer]
        value = [Object.new]
        assert_nil(type.error_message_for_obj(value))
      end

      it 'is not contravariant for the type_member under deep checking' do
        type = T::Enumerable[Integer]
        value = [Object.new]
        expected_error = "Expected type T::Enumerable[Integer], " \
          "got T::Array[Object]"
        msg = type.error_message_for_obj_recursive(value)
        assert_equal(expected_error, msg)
      end

      it 'does not check lazy enumerables (for now)' do
        type = T::Enumerable[Integer]
        value = ["bad"].lazy
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'does not check chain enumerables (for now)' do
        type = T::Enumerable[Integer]
        value = ["bad"].chain([])
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'does not check potentially non-finite enumerables' do
        type = T::Enumerable[Integer]
        value = ["bad"].cycle
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'does not find a problem for enumerables whose each throws under shallow checking' do
        type = T::Enumerable[Integer]
        value = Class.new(Array) do
          def each
            raise "bad"
          end
        end.new(['str'])
        assert_nil(type.error_message_for_obj(value))
      end

      it 'can serialize enumerables whose each throws under deep checking' do
        type = T::Enumerable[Integer]
        value = Class.new(Array) do
          def each
            raise "bad"
          end
        end.new(['str'])
        msg = type.error_message_for_obj_recursive(value)
        expected_error = "Expected type T::Enumerable[Integer], got T::Array[T.untyped]"
        assert_equal(expected_error, msg)
      end
    end

    describe "TypedClass" do
      it 'works if the type is right' do
        type = T::Class[Base]
        value = Base
        msg = check_error_message_for_obj(type, value)
        assert_nil(msg)
      end

      it 'works if the type is wrong, but a class' do
        type = T::Class[Sub]
        value = Base
        assert_nil(type.error_message_for_obj(value))
      end

      it 'cannot have its metatype instantiated' do
        # People might assume that this creates a class with a supertype of
        # `Base`. It doesn't, because generics are erased, and also `[...]` can
        # hold an arbitrary type, not necessarily a class type.
        #
        # Also, `Class.new` already has a sig that infers a _better_ type:
        # Class.new(Base) has an inferred type of `T.class_of(Base)`, which is
        # more narrow.
        assert_raises(NoMethodError) do
          T::Class[Base].new
        end
      end

      it 'is not coerced from plain class literal' do
        # This is for backwards compatibility. If this poses problems for the
        # sake of runtime checking and reflection, we may want to make this
        # behavior more like the static system, where `::Class` has type
        # `T.class_of(Class)`. It looks like we already don't treat `::A` as
        # coercing to `T.class_of(A)`, which is why I don't know whether it
        # particularly matters.
        #
        # (It's also worth noting: Sorbet doesn't have a separate notion of
        # `T::Types::Simple` and `T::Types::ClassOf`. A ClassType is used to
        # model `T::Types::Simple` and _was_ used to model `T.class_of(...)`
        # until we made all singleton classes generic with `<AttachedClass>`,
        # when they became AppliedType.)
        type = T::Utils.coerce(::Class)
        assert_instance_of(T::Types::Simple, type)
        assert_equal(::Class, type.raw_type)
      end
    end

    describe 'TypeAlias' do
      it 'delegates name' do
        type = T.type_alias {T.any(Integer, String)}
        assert_equal('T.any(Integer, String)', type.name)
      end

      it 'delegates equality' do
        assert(T.any(Integer, String) == T.type_alias {T.any(Integer, String)})
        assert(T.type_alias {T.any(Integer, String)} == T.any(Integer, String))
        assert(T.type_alias {T.any(Integer, String)} == T.type_alias {T.any(Integer, String)})

        refute(T.type_alias {T.any(Integer, Float)} == T.type_alias {T.any(Integer, String)})
      end

      it 'passes a validation' do
        type = T.type_alias {T.any(Integer, String)}
        msg = check_error_message_for_obj(type, 1)
        assert_nil(msg)
      end

      it 'provides errors on failed validation' do
        type = T.type_alias {T.any(Integer, String)}
        msg = check_error_message_for_obj(type, true)
        assert_equal('Expected type T.any(Integer, String), got type TrueClass', msg)
      end

      it 'defers block evaluation' do
        crash_type = T.type_alias {raise 'crash'}
        assert_raises(RuntimeError) do
          check_error_message_for_obj(crash_type, 1)
        end
      end
    end

    module TestInterface1
      extend T::Sig
      extend T::Helpers
      interface!

      sig {abstract.returns(T.untyped)}; def hello; end
    end

    module TestInterface2
      extend T::Sig
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

    describe "T.deprecated_enum" do
      before do
        @type = T.deprecated_enum(%i[foo bar])
      end

      it 'passes validation with a value from the enum' do
        msg = check_error_message_for_obj(@type, :foo)
        assert_nil(msg)
      end

      it 'fails validation with a value not from the enum' do
        msg = check_error_message_for_obj(@type, :baz)
        assert_equal("Expected type T.deprecated_enum([:foo, :bar]), got :baz", msg)
      end

      it 'does not coerce types' do
        msg = check_error_message_for_obj(@type, 'foo')
        assert_equal('Expected type T.deprecated_enum([:foo, :bar]), got "foo"', msg)

        type = T.deprecated_enum(%w[foo bar])
        msg = check_error_message_for_obj(type, :foo)
        assert_equal('Expected type T.deprecated_enum(["foo", "bar"]), got :foo', msg)
      end

      it 'fails validation with a nil value' do
        msg = check_error_message_for_obj(@type, nil)
        assert_equal("Expected type T.deprecated_enum([:foo, :bar]), got nil", msg)
      end
    end

    describe "TEnum" do
      before do
        class ::MyEnum < ::T::Enum
          enums do
            A = new
            B = new
            C = new
          end
        end
      end

      after do
        ::Object.send(:remove_const, :MyEnum)
      end

      it 'allows T::Enum values when coercing' do
        a = T::Utils.coerce(::MyEnum::A)
        assert_instance_of(T::Types::TEnum, a)
        assert_equal(a.val, ::MyEnum::A)
        assert_equal(a.name, 'MyEnum::A')
      end

      it 'allows T::Enum values in a sig params' do
        c = Class.new do
          extend T::Sig
          sig {params(x: MyEnum::A).void}
          def self.foo(x); end
        end

        c.foo(::MyEnum::A) # should not raise
        assert_raises(TypeError) do
          c.foo(::MyEnum::B)
        end
        assert_raises(TypeError) do
          c.foo(MyEnum::C)
        end
      end

      it 'allows T::Enum values in a sig returns' do
        c = Class.new do
          extend T::Sig
          sig {returns(MyEnum::A)}
          def self.good_return;
            MyEnum::A;
          end

          sig {returns(MyEnum::B)}
          def self.bad_return;
            MyEnum::C;
          end
        end

        assert_equal(c.good_return, MyEnum::A)

        assert_raises(TypeError) do
          c.bad_return
        end
      end

      it 'allows T::Enum values in a union' do
        c = Class.new do
          extend T::Sig
          sig {params(x: T.any(MyEnum::A, MyEnum::B)).void}
          def self.foo(x); end
        end

        c.foo(::MyEnum::A) # should not raise
        c.foo(::MyEnum::B) # should not raise
        assert_raises(TypeError) do
          c.foo(MyEnum::C)
        end

        runtime_type = T.any(MyEnum::A, MyEnum::B)
        assert_equal("T.any(MyEnum::A, MyEnum::B)", runtime_type.to_s)
      end
    end

    describe "proc" do
      before do
        @type = T::Utils.coerce(T.proc.params(x: Integer, y: Integer).returns(Integer))
      end

      it 'allows procs' do
        assert_nil(check_error_message_for_obj(@type, proc {|x, y| x + y}))
      end

      it 'allows lambdas' do
        assert_nil(check_error_message_for_obj(@type, ->(x, y) {x + y}))
      end

      it 'disallows custom callables' do
        k = Class.new do
          def call(x, y)
            x + y
          end
        end

        callable = k.new
        msg = check_error_message_for_obj(@type, callable)
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
        msg = check_error_message_for_obj(@type, Sub)
        assert_nil(msg)
      end

      it 'passes validation with the class itself' do
        msg = check_error_message_for_obj(@type, Base)
        assert_nil(msg)
      end

      it 'fails validation with some other class' do
        msg = check_error_message_for_obj(@type, InterfaceImplementor1)
        assert_equal("Expected type T.class_of(Opus::Types::Test::TypesTest::Base), got Opus::Types::Test::TypesTest::InterfaceImplementor1", msg)
      end

      it 'fails validation with a non-class' do
        msg = check_error_message_for_obj(@type, 1)
        assert_equal('Expected type T.class_of(Opus::Types::Test::TypesTest::Base), got 1', msg)

        msg = check_error_message_for_obj(@type, Mixin1)
        assert_equal('Expected type T.class_of(Opus::Types::Test::TypesTest::Base), got Opus::Types::Test::TypesTest::Mixin1', msg)
      end

      it 'fails validation with a nil value' do
        msg = check_error_message_for_obj(@type, nil)
        assert_equal('Expected type T.class_of(Opus::Types::Test::TypesTest::Base), got nil', msg)
      end

      it 'fails validation with when supplied an instance of the subclass' do
        msg = check_error_message_for_obj(@type, Sub.new)
        assert_match(/Expected type T.class_of\(Opus::Types::Test::TypesTest::Base\), got/, msg)
      end

      it 'works for a mixin' do
        type = T.class_of(Mixin1)
        msg = check_error_message_for_obj(type, WithMixin)
        assert_nil(msg)
      end

      it 'can not just be .singleton_class' do
        type = T::Utils.coerce(Mixin1.singleton_class)
        msg = check_error_message_for_obj(type, WithMixin)
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

        it "returns true for a simple superclass" do
          assert_equal(true, subtype?(T.class_of(Base), Module))
          assert_equal(true, subtype?(T.class_of(Sub), Module))

          custom_module = Class.new(Module)
          mod = custom_module.new

          assert_equal(true, subtype?(T.class_of(mod), Module))
          assert_equal(true, subtype?(T.class_of(mod), custom_module))
        end

        it "returns false for a simple unrelated class" do
          assert_equal(false, subtype?(T.class_of(Base), Mixin1))
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

        it 'compares upwards to TypedArray' do
          assert_subtype([], T::Array[Integer])
          assert_subtype([], T::Array[String])
          assert_subtype([Integer], T::Array[Integer])
          assert_subtype([Integer, String], T::Array[T.any(Integer, String)])

          refute_subtype([Integer], T::Array[String])
          refute_subtype([Integer, String], T::Array[Integer])
        end
      end

      describe 'shapes' do
        it 'compares upwards to TypedHash' do
          assert_subtype({}, T::Hash[Integer, String])
          assert_subtype({}, T::Hash[String, Symbol])
          assert_subtype({key: Integer}, T::Hash[Symbol, Integer])
          assert_subtype({'key' => Integer}, T::Hash[String, Integer])
          assert_subtype({key: Integer, 'another' => Float}, T::Hash[T.any(Symbol, String), T.any(Integer, Float)])

          refute_subtype({key: Integer}, T::Hash[String, Integer])
          refute_subtype({key: Integer}, T::Hash[Symbol, Float])
          refute_subtype({key: Integer, 'another' => Float}, T::Hash[Symbol, T.any(Integer, Float)])
          refute_subtype({key: Integer, 'another' => Float}, T::Hash[T.any(Symbol, String), Integer])
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

      describe 'noreturn' do
        it 'is a subtype of things' do
          assert_subtype(T.noreturn, Integer)
          assert_subtype(T.noreturn, Numeric)
          assert_subtype(T.noreturn, [String, String])
          assert_subtype(T.noreturn, T::Array[Integer])
          assert_subtype(T.noreturn, T.untyped)
        end

        it 'other things are not a subtype of it' do
          refute_subtype(Integer, T.noreturn)
          refute_subtype(Numeric, T.noreturn)
          refute_subtype([String, String], T.noreturn)
          refute_subtype(T::Array[Integer], T.noreturn)

          # except this one
          assert_subtype(T.untyped, T.noreturn)
        end
      end

      describe 'anything' do
        it 'is not a subtype of things' do
          refute_subtype(T.anything, Integer)
          refute_subtype(T.anything, Numeric)
          refute_subtype(T.anything, [String, String])
          refute_subtype(T.anything, T::Array[Integer])

          # except this one
          assert_subtype(T.anything, T.untyped)
        end

        it 'other things are a subtype of it' do
          assert_subtype(Integer, T.anything)
          assert_subtype(Numeric, T.anything)
          assert_subtype([String, String], T.anything)
          assert_subtype(T::Array[Integer], T.anything)

          assert_subtype(T.untyped, T.anything)
        end
      end

      describe 'type variables' do
        it 'type members are subtypes of everything' do
          assert_subtype(T::Types::TypeMember.new(:in), T.untyped)
          assert_subtype(T::Types::TypeMember.new(:in), String)
          assert_subtype(T::Types::TypeMember.new(:in),
                         T::Types::TypeMember.new(:out))
        end

        it 'everything is a subtype of type members' do
          assert_subtype(T.untyped, T::Types::TypeMember.new(:in))
          assert_subtype(String, T::Types::TypeMember.new(:in))
          assert_subtype(T::Types::TypeMember.new(:out),
                         T::Types::TypeMember.new(:in))
        end

        it 'type parameters are subtypes of everything' do
          assert_subtype(T::Types::TypeParameter.new(:T), T.untyped)
          assert_subtype(T::Types::TypeParameter.new(:T), String)
          assert_subtype(T::Types::TypeParameter.new(:T),
                         T::Types::TypeParameter.new(:V))
        end
      end

      describe 'untyped containers' do
        it 'untyped containers are subtypes of typed containers' do
          assert_subtype(T::Array[T.untyped], T::Array[Integer])
          assert_subtype(T::Array[T.untyped], T::Enumerable[Integer])
          assert_subtype(T::Set[T.untyped], T::Enumerable[Integer])
          assert_subtype(T::Set[T.untyped], T::Set[Integer])
          assert_subtype(T::Hash[T.untyped, T.untyped], T::Hash[Integer, String])
          assert_subtype(T::Enumerator[T.untyped], T::Enumerator[Integer])
        end

        it 'typed containers are subtypes of untyped containers' do
          assert_subtype(T::Array[Integer], T::Array[T.untyped])
          assert_subtype(T::Array[Integer], T::Enumerable[T.untyped])
          assert_subtype(T::Set[Integer], T::Enumerable[T.untyped])
          assert_subtype(T::Set[Integer], T::Set[T.untyped])
          assert_subtype(T::Hash[Integer, String], T::Hash[T.untyped, T.untyped])
          assert_subtype(T::Enumerator[Integer], T::Enumerator[T.untyped])
        end
      end
    end

    module TestGeneric1
      extend T::Sig
      extend T::Generic

      Elem = type_member
      sig {params(bar: Elem).returns(Elem)}
      def self.foo(bar)
        bar
      end
    end

    class TestGeneric2
      extend T::Sig
      extend T::Generic

      Elem1 = type_member
      Elem2 = type_member
      sig {params(bar: Elem1, baz: Elem2).returns([Elem1, Elem2])}
      def foo(bar, baz)
        [bar, baz]
      end
    end

    class TestGeneric3
      extend T::Sig
      extend T::Generic

      Elem = type_member {{fixed: Integer}}
      sig {params(x: Elem).void}
      def foo(x); end
    end

    class GenericSingleton
      extend T::Sig
      extend T::Generic

      SingletonTP = type_template
      sig {params(arg: SingletonTP).returns(SingletonTP)}
      def self.foo(arg)
        arg
      end
    end

    class GenericSingletonChild < GenericSingleton
      SingletonTP = type_template {{fixed: String}}
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

    describe "names" do
      it 'can name T.any with runtime-created classes' do
        klass = Class.new
        name = T.any(klass, String).name
        assert_equal(name, "T.any(String)")

        name = T.any(String, klass).name
        assert_equal(name, "T.any(String)")
      end

      it 'can name T.all with runtime-created classes' do
        klass = Class.new
        name = T.all(klass, String).name
        assert_equal(name, "T.all(String)")

        name = T.all(String, klass).name
        assert_equal(name, "T.all(String)")
      end
    end
  end
end
