# frozen_string_literal: true
require_relative '../../test_helper'

class Opus::Types::Test::Props::SerializableTest < Critic::Unit::UnitTest
  def assert_prop_error(match, &blk)
    ex = assert_raises(ArgumentError) do
      Class.new do
        include T::Props::Serializable
        class_exec(&blk)
      end
    end
    assert_match(match, ex.message)
  end

  class MySerializable
    include T::Props::Serializable
    prop :name, T.nilable(String), raise_on_nil_write: true
    prop :foo, T.nilable(T::Hash[T.any(String, Symbol), Object])
  end

  def a_serializable
    m = MySerializable.new
    m.name = "Bob"
    m.foo  = {
      'age' => 7,
      'color' => 'red',
    }
    m
  end

  class MyPIISerializable
    include T::Props::Serializable
    extend Opus::Sensitivity::PIIable
    prop :pii, T::Hash[String, String], sensitivity: %w{name email_address}
  end

  class DefaultsStruct
    include T::Props::Serializable
    include T::Props::WeakConstructor

    prop :prop1, T.nilable(String), default: "this is prop 1"
    prop :prop2, T.nilable(Integer), factory: -> {raise "don't call me"}
    prop     :trueprop, Boolean, default: true
    prop     :falseprop, Boolean, default: false
  end

  describe ':default and :factory' do
    it 'defaults do not override on from_hash' do
      m = DefaultsStruct.from_hash('prop1' => 'value',
                                   'prop2' => 17)
      assert_equal('value', m.prop1)
      assert_equal(17, m.prop2)
    end

    it 'defaults do not replace nil on from_hash' do
      m = DefaultsStruct.from_hash('prop1' => 'value')
      assert_nil(m.prop2)

      m = DefaultsStruct.from_hash('prop2' => 30)
      assert_nil(m.prop1)
    end

    it 'does not call factories on from_hash' do
      m = DefaultsStruct.from_hash({})
      assert_nil(m.prop2)
    end
  end

  it 'returns the right required props' do
    assert_equal(
      Set[:trueprop, :falseprop],
      DefaultsStruct.decorator.required_props.to_set
    )
  end

  it 'serializes' do
    m = a_serializable

    hash = m.serialize
    assert_kind_of(Hash, hash)
    assert_equal("Bob", hash['name'])
    assert_equal(7, hash['foo']['age'])
  end

  describe '.inspect' do
    it 'inspects' do
      obj = a_serializable
      str = obj.inspect
      assert_equal('<Opus::Types::Test::Props::SerializableTest::MySerializable foo={"age"=>7, "color"=>"red"}, name="Bob">', str)
    end

    it 'inspects with extra props' do
      obj = a_serializable
      obj = obj.class.from_hash(obj.serialize.merge('not_a_prop' => 'but_here_anyway'))
      str = obj.inspect
      assert_equal('<Opus::Types::Test::Props::SerializableTest::MySerializable foo={"age"=>7, "color"=>"red"}, name="Bob" @_extra_props=<not_a_prop="but_here_anyway">>', str)
    end

    it 'redacts PII' do
      obj = MyPIISerializable.new
      obj.pii = {"private" => "s3cr3t"}
      str = obj.inspect
      assert_equal('<Opus::Types::Test::Props::SerializableTest::MyPIISerializable pii=<REDACTED name, email_address>>', str)
    end
  end

  describe '.from_hash' do
    it 'round-trips' do
      m = a_serializable
      assert_equal(m.serialize, m.class.from_hash(m.serialize).serialize)
    end

    it 'does not call the constructor' do
      MySerializable.any_instance.expects(:initialize).never
      MySerializable.from_hash({})
    end
  end

  describe 'hash props' do
    it 'does not share structure on serialize' do
      m = MySerializable.new
      m.name = 'hi'
      m.foo = {'hello' => {'world' => 1}}
      h = m.serialize
      refute_equal(m.foo.object_id, h['foo'].object_id, "`foo` is the same object")
      refute_equal(m.foo['hello'].object_id, h['foo']['hello'].object_id, "`foo.hello` is the same object")
    end

    it 'does not share structure on deserialize' do
      h = {
        'name' => 'hi',
        'foo' => {'hello' => {'world' => 1}},
      }
      m = MySerializable.from_hash(h)
      refute_equal(m.foo.object_id, h['foo'].object_id, "`foo` is the same object")
      refute_equal(m.foo['hello'].object_id, h['foo']['hello'].object_id, "`foo.hello` is the same object")
    end
  end

  describe "prop declaration" do
    it ':name must be a string' do
      assert_prop_error(/Invalid name in prop/) do
        prop :foo, String, name: :goats
      end
    end
  end

  class MyNilableSerializable
    include T::Props::Serializable
    prop :name, T.nilable(String)
    prop :address, T.nilable(String)
  end

  describe "nilable" do
    it 'can serialize nilables' do
      obj = MyNilableSerializable.new
      obj.name = 'Avi'
      assert_equal({'name' => 'Avi'}, obj.serialize)

      obj = MyNilableSerializable.new
      obj.name = nil
      assert_equal({}, obj.serialize)
    end

    it 'can serialize optionals' do
      obj = MyNilableSerializable.new
      obj.address = 'TEST'
      assert_equal({'address' => 'TEST'}, obj.serialize)

      obj = MyNilableSerializable.new
      obj.address = nil
      assert_equal({}, obj.serialize)
    end
  end

  class MigratingNilFieldModel < T::Struct
    prop :foo, T.nilable(Integer), notify_on_nil_write: 'storage'
    prop :bar, T.nilable(String), notify_on_nil_write: Opus::Project.storage
  end

  describe 'notify_on_nil_write' do
    it 'requires a string or project value' do
      assert_prop_error(/must be a string or project/) do
        prop :foo, T.nilable(String), notify_on_nil_write: 1
      end
    end

    it 'it soft asserts on nil writes' do
      foo = MigratingNilFieldModel.new
      ex = assert_raises do
        foo.serialize
      end
      assert_includes(ex.message, 'nil value written to prop with :notify_on_nil_write set')
    end

    it 'does not assert when strict=false' do
      foo = MigratingNilFieldModel.new
      foo.serialize(false)
    end
  end

  class MigratingNilFieldModel2 < T::Struct
    prop :foo, T.nilable(Integer), raise_on_nil_write: true
    prop :bar, T.nilable(String), raise_on_nil_write: true
  end

  describe 'raise_on_nil_write' do
    it 'requires the value to be true' do
      assert_prop_error(/if specified must be `true`/) do
        prop :foo, T.nilable(String), raise_on_nil_write: false
      end

      assert_prop_error(/if specified must be `true`/) do
        prop :foo, T.nilable(String), raise_on_nil_write: 1
      end
    end

    it 'throw exception on nil writes' do
      foo = MigratingNilFieldModel2.new
      ex = assert_raises do
        foo.serialize
      end
      assert_includes(ex.message, 'Opus::Types::Test::Props::SerializableTest::MigratingNilFieldModel2.foo not set for non-optional prop')
    end

    it 'does not assert when strict=false' do
      foo = MigratingNilFieldModel2.new
      foo.serialize(false)
    end
  end

  describe 'raise_on_nil_write and notify_on_nil_write' do
    it 'does not allow both to be specified at once' do
      ex = assert_raises do
        Class.new(T::Struct) do
          prop :foo, T.nilable(String), raise_on_nil_write: true, notify_on_nil_write: 'storage'
        end
      end
      assert_match(/You can only specify one of `raise_on_nil_write` and `notify_on_nil_write`/, ex.message)
    end
  end
end
