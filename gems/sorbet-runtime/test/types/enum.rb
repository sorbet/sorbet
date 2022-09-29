# frozen_string_literal: true
# typed: false
require_relative '../test_helper'

module T::Enum::Test; end
class T::Enum::Test::EnumTest < Critic::Unit::UnitTest
  class CardSuit < T::Enum
    enums do
      CLUB = new
      SPADE = new
      DIAMOND = new
      HEART = new
    end
  end

  class CardSuitCustom < T::Enum
    enums do
      CLUB = new('_club_')
      SPADE = new('_spade_')
      DIAMOND = new('_diamond_')
      HEART = new('_heart_')
    end
  end

  describe 'serialize' do
    describe 'default serialization' do
      it 'downcases the constant names' do
        assert_equal('club', CardSuit::CLUB.serialize)
        assert_equal('spade', CardSuit::SPADE.serialize)
        assert_equal('diamond', CardSuit::DIAMOND.serialize)
        assert_equal('heart', CardSuit::HEART.serialize)
      end
    end

    describe 'custom serialized values' do
      it 'uses the provided serialized values' do
        assert_equal('_club_', CardSuitCustom::CLUB.serialize)
        assert_equal('_spade_', CardSuitCustom::SPADE.serialize)
        assert_equal('_diamond_', CardSuitCustom::DIAMOND.serialize)
        assert_equal('_heart_', CardSuitCustom::HEART.serialize)
      end
    end
  end

  describe 'desired properties' do
    it 'basic ancestry' do
      assert(CardSuit::DIAMOND.is_a?(CardSuit))
      assert(CardSuit < T::Enum)
    end

    it 'works with case statements' do
      suit = CardSuit::SPADE
      case suit
      when CardSuit::HEART, CardSuit::DIAMOND, CardSuit::CLUB
        flunk
      when CardSuit::SPADE
        true
      else
        flunk
      end
    end
  end

  describe 'inspect' do
    it 'prints the full constant name' do
      assert_equal('#<T::Enum::Test::EnumTest::CardSuit::SPADE>', CardSuit::SPADE.inspect)
    end
  end

  describe 'to_s' do
    it 'prints the full constant name' do
      assert_equal('#<T::Enum::Test::EnumTest::CardSuit::SPADE>', CardSuit::SPADE.to_s)
    end
  end

  describe 'values' do
    it 'returns an array of enum instances, sorted by code order' do
      # note this is *not* alphabetical order (S before D)
      assert_equal(
        [CardSuit::CLUB, CardSuit::SPADE, CardSuit::DIAMOND, CardSuit::HEART],
        CardSuit.values
      )
    end

    it 'is a frozen array' do
      ex = assert_raises(RuntimeError) do
        CardSuit.values << 'foo'
      end
      assert_match(/can't modify frozen Array/, ex.message)
    end
  end

  describe 'each_value' do
    it 'yields values given block' do
      result = []
      CardSuit.each_value do |val|
        result << val
      end

      assert_equal(
        [CardSuit::CLUB, CardSuit::SPADE, CardSuit::DIAMOND, CardSuit::HEART],
        result,
      )
    end

    it 'returns values given no block' do
      result = CardSuit.each_value.to_a

      assert_equal(
        [CardSuit::CLUB, CardSuit::SPADE, CardSuit::DIAMOND, CardSuit::HEART],
        result,
      )
    end
  end

  describe 'from_serialized' do
    it 'returns the corresponding enum instance' do
      club = CardSuit.from_serialized('club')
      assert_equal(CardSuit::CLUB, club)
      assert_equal(CardSuit::CLUB.object_id, club.object_id)
    end

    it 'returns the corresponding enum instance with custom serialization' do
      club = CardSuitCustom.from_serialized('_club_')
      assert_equal(CardSuitCustom::CLUB, club)
      assert_equal(CardSuitCustom::CLUB.object_id, club.object_id)
    end

    it 'raises KeyError for a value that does it does not contain' do
      assert_raises(KeyError) do
        CardSuit.from_serialized('whoopsies')
      end
    end
  end

  describe 'has_serialized?' do
    it 'works for default serializations' do
      assert_equal(true, CardSuit.has_serialized?('heart'))
      assert_equal(false, CardSuit.has_serialized?('blerg'))
    end

    it 'works for custom serializations' do
      assert_equal(true, CardSuitCustom.has_serialized?('_spade_'))
      assert_equal(false, CardSuitCustom.has_serialized?('spade'))
      assert_equal(false, CardSuitCustom.has_serialized?('blerg'))
    end

    it 'does not break for arbitrary objects' do
      assert_equal(false, CardSuitCustom.has_serialized?(Class.new.new))
    end
  end

  describe 'try_deserialize' do
    it 'returns the value for a valid serialization' do
      suit = CardSuit.try_deserialize('club')
      assert_equal(CardSuit::CLUB, suit)
    end

    it 'returns nil for an invalid serialization' do
      suit = CardSuit.try_deserialize('blerg')
      assert_nil(suit)
    end
  end

  describe 'to_json' do
    it 'serializes to a JSON string' do
      assert_equal('"club"', CardSuit::CLUB.to_json)
    end

    it 'serializes to a string when used as a hash value' do
      h = {
        key1: CardSuit::SPADE,
        key2: CardSuitCustom::HEART,
      }
      assert_equal("{\"key1\":\"spade\",\"key2\":\"_heart_\"}", h.to_json)
    end

    it 'serializes hash keys with the inspect string' do
      h = {
        CardSuit::SPADE => 'x',
        CardSuitCustom::HEART => 'y',
      }
      assert_equal(
        {
          "#<T::Enum::Test::EnumTest::CardSuit::SPADE>" => "x",
          "#<T::Enum::Test::EnumTest::CardSuitCustom::HEART>" => "y",
        },
        JSON.parse(h.to_json))
    end
  end

  describe 'as_json' do
    class CustomSerializedValue
      def as_json(*args)
        1234
      end
    end

    class CustomSerializationEnum < T::Enum
      enums do
        SPADE = new(CustomSerializedValue.new)
      end
    end

    it 'has a JSON representation' do
      assert_equal("club", CardSuit::CLUB.as_json)
    end

    it 'asks for the JSON representation of the serialized value' do
      assert_equal(1234, CustomSerializationEnum::SPADE.as_json)
    end
  end

  describe 'T::Props::CustomType integration' do
    it 'supports instance? checks on the Enum class' do
      assert_equal(true, CardSuit.instance?(CardSuit::SPADE))
      assert_equal(false, CardSuit.instance?(CardSuit))
      assert_equal(false, CardSuit.instance?(CardSuitCustom::HEART))
      assert_equal(false, CardSuit.instance?(nil))
    end

    it 'supports class level serialize' do
      assert_equal('spade', CardSuit.serialize(CardSuit::SPADE))
      assert_equal('_spade_', CardSuitCustom.serialize(CardSuitCustom::SPADE))
    end

    it 'does not allow serialization of strings' do
      ex = assert_raises(RuntimeError) do
        CardSuit.serialize("heart")
      end
      assert_match(/Cannot call #serialize on a value that is not an instance of .*CardSuit/, ex.message)
    end

    it 'does not allow a different instance type to be serialized' do
      ex = assert_raises(RuntimeError) do
        CardSuitCustom.serialize(CardSuit::HEART) # mixed Custom and non-Custom
      end
      assert_match(/Cannot call #serialize on a value that is not an instance of .*CardSuitCustom/, ex.message)
    end

    it 'does not allow T::Enum.serialize to be called directly' do
      ex = assert_raises(RuntimeError) do
        T::Enum.serialize(CardSuit::HEART)
      end
      assert_match(/Cannot call T::Enum.serialize directly/, ex.message)
    end

    it 'supports class-level deserialize' do
      assert_equal(CardSuit::CLUB, CardSuit.deserialize('club'))
    end

    it 'does not allow calling T::Enum.deserialize directly' do
      ex = assert_raises(RuntimeError) do
        T::Enum.deserialize('abcdef')
      end
      assert_match(/Cannot call T::Enum.deserialize directly/, ex.message)
    end
  end

  describe 'enum declaration validation' do
    # NB: Constants declared like `FOO = bar` are not added to classes that are declared with blocks
    # (e.g. `Class.new do ...`). To get around this in these tests we `const_set` directly.
    #
    # For purposes of these tests, the  declaration of the following form
    #
    #   Class.new(T::Enum) do
    #     const_set(:FOO, new)
    #   end
    #
    # is equivalent to:
    #
    #   MyEnum < T::Enum
    #     Foo = new
    #   end

    it 'does not allow duplicated serialized_vals' do
      ex = assert_raises(RuntimeError) do
        Class.new(T::Enum) do
          enums do
            const_set(:FOO, new('a'))
            const_set(:BAR, new('b'))
            const_set(:BAZ, new('a'))
          end
        end
      end
      assert_match(/Enum values must have unique serializations. Value 'a' is repeated/, ex.message)
    end

    it 'does not allow for constants on an enum that are not instances of itself' do
      ex = assert_raises(RuntimeError) do
        Class.new(T::Enum) do
          enums do
            const_set(:FOO, new('a'))
            const_set(:THIS_IS_BAD, 'mystring')
          end
        end
      end
      assert_match(/Invalid constant .*::THIS_IS_BAD on enum/, ex.message)
    end

    it 'does not allow more values to be added to an enum after it has been defined' do
      ex = assert_raises(RuntimeError) do
        CardSuit.send(:new, 'another_val')
      end
      assert_equal(
        'Cannot instantiate a new enum value of T::Enum::Test::EnumTest::CardSuit after it has been initialized.',
        ex.message
      )
    end

    it 'does not allow `_register_instance` after enum has been defined' do
      ex = assert_raises(RuntimeError) do
        CardSuit._register_instance(CardSuit::HEART)
      end
      assert_match(/can't modify frozen Array/, ex.message)
    end

    it 'returns the same object for #dup and #clone' do
      assert_equal(CardSuit::DIAMOND.object_id, CardSuit::DIAMOND.dup.object_id)
      assert_equal(CardSuit::DIAMOND.object_id, CardSuit::DIAMOND.clone.object_id)
    end

    it 'does not allow the T::Enum to be instantiated' do
      ex = assert_raises(RuntimeError) do
        T::Enum.send(:new, 'another_val')
      end
      assert_equal(
        'T::Enum is abstract',
        ex.message
      )
    end

    it 'does not allow Enum classes to be inherited from' do
      ex = assert_raises(RuntimeError) do
        Class.new(CardSuit) do
          const_set(:FOO, new('a'))
        end
      end
      assert_equal('Inheriting from children of T::Enum is prohibited', ex.message)
    end

    it 'raises if an uninitialized enum is serialized' do
      ex = assert_raises(RuntimeError) do
        Class.new(T::Enum) do
          enums do
            const_set(:FOO, new('a'))
            # At this point FOO has not been initialized yet
            const_get(:FOO, false).serialize
          end
        end
      end
      assert_match(
        /Attempting to access Enum value on #<.*> before it has been initialized/,
        ex.message
      )
    end

    it 'does not allow instance without assigning to constant' do
      ex = assert_raises(RuntimeError) do
        Class.new(T::Enum) do
          enums do
            new('foo')
            new
          end
        end
      end
      assert_equal('Enum values must be assigned to constants: ["foo", nil]', ex.message)
    end
  end

  describe 'string value conversion assertions' do
    ENUM_CONVERSION_MSG = /Implicit conversion of Enum instances to strings is not allowed. Call #serialize instead./.freeze
    before do
      T::Configuration.expects(:soft_assert_handler).never
    end

    it 'raises an assertion if to_str is called and also returns the serialized value' do
      ex = assert_raises(NoMethodError) do
        CardSuit::HEART.to_str
      end
      assert_match(ENUM_CONVERSION_MSG, ex.message)
    end

    it 'raises an assertion if to_str is called (implicitly) and also returns the serialized value' do
      ex = assert_raises(NoMethodError) do
        'foo ' + CardSuit::HEART
      end
      assert_match(ENUM_CONVERSION_MSG, ex.message)
    end
  end

  describe 'string value conversion assertions in legacy migration mode' do
    ENUM_CONVERSION_MSG_LEGACY = 'Implicit conversion of Enum instances to strings is not allowed. Call #serialize instead.'
    before do
      T::Configuration.enable_legacy_t_enum_migration_mode
      T::Configuration.expects(:soft_assert_handler).at_least_once.with do |message|
        assert_equal(ENUM_CONVERSION_MSG_LEGACY, message)
      end
    end

    after do
      T::Configuration.disable_legacy_t_enum_migration_mode
    end

    it 'raises an assertion if to_str is called and also returns the serialized value' do
      assert_equal('heart', CardSuit::HEART.to_str)
    end

    it 'raises an assertion if to_str is called (implicitly) and also returns the serialized value' do
      assert_equal('foo heart', 'foo ' + CardSuit::HEART)
    end
  end

  describe 'string value comparison assertions' do
    # rubocop:disable Style/YodaCondition

    it 'returns false if the types mismatch for ==' do
      assert_equal(false, 'spade' == CardSuit::SPADE)
      assert_equal(false, 'diamond' == CardSuit::SPADE)
      assert_equal(false, CardSuit::SPADE == 'spade')
      assert_equal(false, CardSuit::SPADE == 'diamond')
    end

    it 'returns false if the types mismatch for ===' do
      assert_equal(false, 'spade' === CardSuit::SPADE)
      assert_equal(false, 'club' === CardSuit::SPADE)
      assert_equal(false, CardSuit::SPADE === 'spade')
      assert_equal(false, CardSuit::CLUB === 'spade')
    end

    # rubocop:enable Style/YodaCondition

    it 'returns false for a string in a `when` compared to an enum value' do
      val = CardSuit::SPADE

      matched = case val
      when 'club' then false
      when 'spade' then true
      else false
      end
      assert_equal(false, matched)
    end

    it 'returns false for a enum in a `when` compared to an string value' do
      val = 'spade'

      matched = case val
      when CardSuit::CLUB then false
      when CardSuit::SPADE then true
      end
      assert_nil(matched)
    end
  end

  describe 'string value comparison assertions in legacy migration mode' do
    ENUM_COMPARE_MSG = 'Enum to string comparison not allowed. Compare to the Enum instance directly instead. See go/enum-migration'
    before do
      T::Configuration.enable_legacy_t_enum_migration_mode
      T::Configuration.expects(:soft_assert_handler).at_least_once.with do |message|
        assert_equal(ENUM_COMPARE_MSG, message)
      end
    end

    after do
      T::Configuration.disable_legacy_t_enum_migration_mode
    end

    # rubocop:disable Style/YodaCondition

    it 'raises an assertion if string is lhs of comparison' do
      assert_equal(true, 'spade' == CardSuit::SPADE)

      assert_equal(false, 'diamond' == CardSuit::SPADE)
    end

    it 'raises an assertion if string is rhs of comparison' do
      assert_equal(true, CardSuit::SPADE == 'spade')

      assert_equal(false, CardSuit::SPADE == 'diamond')
    end

    it 'raises an assertion if string is lhs of === comparison' do
      assert_equal(true, 'spade' === CardSuit::SPADE)

      assert_equal(false, 'club' === CardSuit::SPADE)
    end

    it 'raises an assertion if string is rhs of === comparison' do
      assert_equal(true, CardSuit::SPADE === 'spade')

      assert_equal(false, CardSuit::CLUB === 'spade')
    end

    # rubocop:enable Style/YodaCondition

    it 'raises an assertion for a string in a `when` compared to an enum value' do
      val = CardSuit::SPADE

      matched = case val
      when 'club' then false
      when 'spade' then true
      else false
      end
      assert_equal(true, matched)
    end

    it 'raises an assertion for a enum in a `when` compared to an string value' do
      val = 'spade'

      matched = case val
      when CardSuit::CLUB then false
      when CardSuit::SPADE then true
      end
      assert_equal(true, matched)
    end
  end

  it "is compatible with Marshal" do
    (CardSuit.values + CardSuitCustom.values).each do |value|
      roundtrip = Marshal.load(Marshal.dump(value))

      assert_equal(roundtrip, value)

      assert(value === roundtrip)

      assert(
        case roundtrip
        when value then true
        else false
        end
      )
    end
  end

  it 'can be used with an assert' do
    assert(CardSuit::SPADE, 'some message')
  end
end
