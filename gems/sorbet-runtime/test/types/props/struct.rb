# frozen_string_literal: true
require_relative '../../test_helper'

class Opus::Types::Test::Props::StructTest < Critic::Unit::UnitTest
  class MyStruct < T::Struct
  end

  class StructWithClassProp < T::Struct
    prop :class, String, without_accessors: true
  end

  class StructWithObjectIdProp < T::Struct
    prop :object_id, String, clobber_existing_method!: true # FIXME: This overrides a builtin method
  end

  class StructWithPredefinedHash < T::Struct
    prop :hash_field1, {a: Integer, b: Integer}
    prop :hash_field2, T.nilable({a: Integer, b: Integer})
  end

  it 'errors if you try to set a "class" prop' do
    assert_raises(ArgumentError) do
      MyStruct.prop :class, String
    end
  end

  it 'errors if you try to set an "object_id" prop' do
    assert_raises(ArgumentError) do
      MyStruct.prop :object_id, String
    end
  end

  it 'can set hash field correspondingly' do
    doc = StructWithPredefinedHash.new(hash_field1: {a: 100, b: 200}, hash_field2: nil)
    assert_equal(100, doc.hash_field1[:a])
    assert_equal(200, doc.hash_field1[:b])
    assert_nil(doc.hash_field2)
  end

  it 'can set nilable hash field correspondingly' do
    doc = StructWithPredefinedHash.new(hash_field1: {a: 100, b: 200}, hash_field2: {a: 100, b: 200})
    assert_equal(100, doc.hash_field2[:a])
    assert_equal(200, doc.hash_field2[:b])
  end

  it 'type check hash field correspondingly' do
    assert_raises(TypeError) do
      StructWithPredefinedHash.new(hash_field1: {a: 'foo', b: 200}, hash_field2: {a: 100, b: 200})
    end
    assert_raises(TypeError) do
      StructWithPredefinedHash.new(hash_field1: {a: 100, b: 200}, hash_field2: {a: 'foo', b: 200})
    end
  end

  it 'uses the original value when value is invalid in a soft error environment' do
    T::Configuration.call_validation_error_handler = proc do
      # no raise
    end

    hash_field1 = {a: 'foo', b: 200}
    doc = StructWithPredefinedHash.new(hash_field1: hash_field1)
    assert_equal('foo', doc.hash_field1[:a])
  ensure
    T::Configuration.call_validation_error_handler = nil
  end

  it 'can initialize a struct with a prop named "class" if without_accessors is true' do
    doc = StructWithClassProp.new(class: "the_class")
    assert_equal(StructWithClassProp, doc.class)
    assert_equal("the_class", doc.class.decorator.prop_get(doc, :class))
  end

  it 'can initialize a struct with a prop named "object_id" if clobber_existing_method! is true' do
    doc = StructWithObjectIdProp.new(object_id: 'object_id')
    assert_equal('object_id', doc.object_id)
    assert_equal('object_id', doc.class.decorator.prop_get(doc, :object_id))
  end

  class SubStruct < T::Struct
    prop :bar1, String
    prop :bar2, String
  end

  class TestStruct < T::Struct
    prop :foo0, Integer
    prop :foo1, Integer
    prop :foo2, String
    prop :foo3, T.nilable(String)
    prop :foo4, T.nilable(T::Array[String])
    prop :foo5, T.nilable(T::Array[SubStruct])
  end

  class StructWithRequiredField < T::Struct
    prop :foo1, Integer
    prop :foo2, Integer
  end

  describe 'required/optional prop test' do
    it 'Test optional: false' do
      assert_equal(true, T::Props::Utils.required_prop?(TestStruct.props[:foo0]))
      assert_nil(TestStruct.props[:foo0][:optional])

      assert_equal(true, T::Props::Utils.required_prop?(TestStruct.props[:foo1]))
      assert_nil(TestStruct.props[:foo1][:optional])

      assert_equal(true, T::Props::Utils.required_prop?(TestStruct.props[:foo2]))
      assert_nil(TestStruct.props[:foo2][:optional])

      assert_equal(true, T::Props::Utils.optional_prop?(TestStruct.props[:foo3]))
      assert_nil(TestStruct.props[:foo3][:optional])

      assert_equal(true, T::Props::Utils.optional_prop?(TestStruct.props[:foo4]))
      assert_nil(TestStruct.props[:foo4][:optional])

      assert_equal(true, T::Props::Utils.optional_prop?(TestStruct.props[:foo5]))
      assert_nil(TestStruct.props[:foo5][:optional])
    end

    it 'tstruct tnilable field type_object' do
      c = Class.new(T::Struct) do
        prop :foo, T.nilable(String)
        prop :wday, T.nilable(String), enum: %w[mon tue]
      end
      assert_equal(T.nilable(String), c.props[:foo][:type_object])
      assert_equal(T.nilable(T.all(String, T.deprecated_enum(%w[mon tue]))), c.props[:wday][:type_object])
    end

    it 'tstruct deserialize optional fields' do
      doc = TestStruct.from_hash(
        {'foo0' => 0, 'foo1' => 1, 'foo2' => '2', 'foo3' => '3', 'foo5' => [{'bar1' => 'bar1', 'bar2' => 'bar2'}]})
      assert_equal('bar1', doc.foo5[0].bar1)

      assert_equal(String, TestStruct.props[:foo2][:type])
      assert_equal(T::Types::Simple, TestStruct.props[:foo2][:type_object].class)
      assert_equal(String, TestStruct.props[:foo2][:type_object].raw_type)

      assert_equal(String, TestStruct.props[:foo3][:type])
      assert_equal(T.nilable(String), TestStruct.props[:foo3][:type_object])

      assert_equal(T::Array[String], TestStruct.props[:foo4][:type])
      assert_equal(T.nilable(T::Array[String]), TestStruct.props[:foo4][:type_object])

      assert_equal(T::Array[Opus::Types::Test::Props::StructTest::SubStruct], TestStruct.props[:foo5][:type])
      assert_equal(
        T.nilable(T::Array[Opus::Types::Test::Props::StructTest::SubStruct]), TestStruct.props[:foo5][:type_object])
    end

    it 'tstruct need to initialize required fields' do
      doc = StructWithRequiredField.new(foo1: 10, foo2: 20)
      assert_equal(10, doc.foo1)
      assert_equal(20, doc.foo2)

      assert_raises(ArgumentError) do
        StructWithRequiredField.new(foo2: 20)
      end
      assert_raises(ArgumentError) do
        StructWithRequiredField.new(foo1: 10)
      end
    end

    it 'tstruct deserialize different fields' do
      doc = StructWithRequiredField.from_hash({'foo1' => 10, 'foo2' => 20})
      assert_equal(10, doc.foo1)
      assert_equal(20, doc.foo2)

      assert_raises(RuntimeError) do
        StructWithRequiredField.from_hash({'foo2' => 20})
      end

      # The code should behave for deserialization.
      assert_raises(RuntimeError) do
        StructWithRequiredField.from_hash({'foo1' => 10})
      end
    end
  end
end
