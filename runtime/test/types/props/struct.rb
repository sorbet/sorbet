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
    assert_raises(T::Props::InvalidValueError) do
      StructWithPredefinedHash.new(hash_field1: {a: 'foo', b: 200}, hash_field2: {a: 100, b: 200})
    end
    assert_raises(T::Props::InvalidValueError) do
      StructWithPredefinedHash.new(hash_field1: {a: 100, b: 200}, hash_field2: {a: 'foo', b: 200})
    end
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
end
