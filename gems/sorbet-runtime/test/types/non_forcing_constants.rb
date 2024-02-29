# frozen_string_literal: true
require_relative '../test_helper'

# this is a fake "package" which we can use to test the packaged form
# later on
module SomePackage
  class Thing; end
end

class Opus::Types::Test::NonForcingConstantsTest < Critic::Unit::UnitTest
  class MyClass
  end

  MyField = 0

  autoload :RaisesIfLoaded, "#{__dir__}/fixtures/always_raise.rb"

  describe "T::NonForcingConstants.non_forcing_is_a?" do
    it "empty string" do
      exn = assert_raises(ArgumentError) do
        T::NonForcingConstants.non_forcing_is_a?(nil, '')
      end
      assert_match(/must not be empty/, exn.message)
    end

    it "non-absolute" do
      exn = assert_raises(ArgumentError) do
        T::NonForcingConstants.non_forcing_is_a?(nil, 'X')
      end
      assert_match(/must be an absolute constant reference/, exn.message)
    end

    it "when doesn't exist" do
      res = T::NonForcingConstants.non_forcing_is_a?(nil, '::DoesntExist')
      assert_equal(false, res)

      res = T::NonForcingConstants.non_forcing_is_a?(nil, '::Doesnt::Exist')
      assert_equal(false, res)
    end

    it "when exists but isn't is_a?" do
      res = T::NonForcingConstants.non_forcing_is_a?(nil, '::Integer')
      assert_equal(false, res)
    end

    it "when exists and is_a?" do
      res = T::NonForcingConstants.non_forcing_is_a?(0, '::Integer')
      assert_equal(true, res)
    end

    it "multi-part" do
      outer = Opus::Types::Test::NonForcingConstantsTest::MyClass.new

      res = T::NonForcingConstants.non_forcing_is_a?(outer, '::Opus::Types::Test::NonForcingConstantsTest::DoesntExist')
      assert_equal(false, res)

      res = T::NonForcingConstants.non_forcing_is_a?(outer, '::Opus::Types::Test::NonForcingConstantsTest::MyClass')
      assert_equal(true, res)
    end

    it "sanity check our fixture" do
      exn = assert_raises(RuntimeError) do
        ::Opus::Types::Test::NonForcingConstantsTest::RaisesIfLoaded
      end
      assert_match(/This file was loaded, but shouldn't have been/, exn.message)

      # Do it again to prove that there's no cleanup we'd have to do
      # (like re-register the autoload after failing to force it once)
      exn = assert_raises(RuntimeError) do
        ::Opus::Types::Test::NonForcingConstantsTest::RaisesIfLoaded
      end
      assert_match(/This file was loaded, but shouldn't have been/, exn.message)
    end

    it "doesn't force autoloads" do
      res = T::NonForcingConstants.non_forcing_is_a?(nil, '::Opus::Types::Test::NonForcingConstantsTest::RaisesIfLoaded')
      assert_equal(false, res)

      # Note that we return `false` even if the constant doesn't ultimately exist,
      # because sometimes we can only know whether it exists by forcing the constant.
      # In these cases, Sorbet's static checks guarantee that the string literal
      # resolves to a valid constant.
      res = T::NonForcingConstants.non_forcing_is_a?(nil, '::Opus::Types::Test::NonForcingConstantsTest::RaisesIfLoaded::DoesntExist')
      assert_equal(false, res)
    end

    it "requires class/module, not static-fields" do
      exn = assert_raises(ArgumentError) do
        T::NonForcingConstants.non_forcing_is_a?(nil, '::Opus::Types::Test::NonForcingConstantsTest::MyField')
      end
      assert_match(/is not a class or module/, exn.message)

      exn = assert_raises(ArgumentError) do
        T::NonForcingConstants.non_forcing_is_a?(nil, '::Opus::Types::Test::NonForcingConstantsTest::MyField::DoesntExist')
      end
      assert_match(/is not a class or module/, exn.message)
    end
  end
end
