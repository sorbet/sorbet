# typed: true
require 'minitest/autorun'
require_relative 'gems/sorbet-runtime/lib/sorbet-runtime'
extend T::Sig

class Minitest::Spec
  def self.test_each(arg, &blk)
    arg.each(&blk)
  end
end

class Flag
  def self.enable(arg)
    p arg
  end
end

class MyTest < Minitest::Spec
  test_each([true, false]) do |flag_enabled|
    describe 'foo' do
      before do
        T.reveal_type(flag_enabled)
        Flag.enable(flag_enabled)
        @enabled = T.let(flag_enabled, T::Boolean)
      end

      it "do this thing" do
        T.reveal_type(@enabled)
        T.reveal_type(flag_enabled)
      end
    end
  end
end

class MyTestBad1 < Minitest::Spec
  test_each([true, false]) do |flag_enabled|
    describe 'foo' do
      def nope; end
    # ^^^^^^^^^^^^^ error: Only valid `it`-blocks can appear within `test_each`
    end
  end
end

class MyTestBad2 < Minitest::Spec
  test_each([true, false]) do |flag_enabled|
    describe 'foo' do
      it '' do
      end
      def nope; end
    # ^^^^^^^^^^^^^ error: Only valid `it`-blocks can appear within `test_each`
    end
  end
end

class MyTestBad3 < Minitest::Spec
  test_each([true, false]) do |flag_enabled|
    describe 'foo' do
      it '' do
      end
      def nope; end
    # ^^^^^^^^^^^^^ error: Only valid `it`-blocks can appear within `test_each`
    end
  end
end

class MyTestBad4 < Minitest::Spec
  test_each([true, false]) do |flag_enabled|
    x = flag_enabled
  # ^^^^^^^^^^^^^ error: Only valid `it`-blocks can appear within `test_each`
    puts(x)
  # ^^^^^^^ error: Only valid `it`-blocks can appear within `test_each`
    describe 'foo' do
      it '' do
      end
    end
  end
end
