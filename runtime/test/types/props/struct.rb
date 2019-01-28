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
