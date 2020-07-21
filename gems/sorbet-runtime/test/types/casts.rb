# frozen_string_literal: true
require_relative '../test_helper'

module Opus::Types::Test
  class CastsTest < Critic::Unit::UnitTest
    [:cast, :assert_type!].each do |cast_name|
      method = T.method(cast_name)

      describe "T.#{cast_name}" do
        it 'allows correct casts' do
          assert_equal(:foo, method.call(:foo, Symbol))
          assert_equal(:foo, method.call(:foo, T.any(Symbol, Integer)))
          assert_equal(:foo, method.call(:foo, T.untyped))
        end

        it 'rejects invalid casts' do
          assert_raises(TypeError) do
            method.call(:foo, Integer)
          end

          assert_raises(TypeError) do
            method.call(:foo, T.any(String, Integer))
          end

          assert_raises(TypeError) do
            method.call(:foo, T.noreturn)
          end
        end

        it 'allows invalid casts with checked: false' do
          assert_equal(:foo, method.call(:foo, Integer, checked: false))
          assert_equal(:foo, method.call(:foo, T.any(String, Integer), checked: false))
          assert_equal(:foo, method.call(:foo, T.noreturn, checked: false))
        end

        it 'has a good error message' do
          lno = nil
          ex = assert_raises(TypeError) do
            lno = __LINE__; method.call(:foo, T.any(String, Integer))
          end
          assert_match(/T.#{cast_name}: /, ex.message)
          assert_match(/type T.any\(Integer, String\)/, ex.message)
          assert_match(/\nCaller: #{__FILE__}:#{lno}/, ex.message)
        end
      end
    end

    describe "T.assert_type with collections" do
      it "does not do recursive type-checking of arrays with `assert_type!`" do
        assert_equal([1], T.assert_type!([1], T::Array[String]))
      end

      it "does do recursive type-checking of arrays with `check_type_recursive!`" do
        assert_raises(TypeError) do
          T::Utils.check_type_recursive!([1], T::Array[String])
        end
      end

      it "does not do recursive type-checking of hashes with `assert_type!`" do
        assert_equal({x: "y"}, T.assert_type!({x: "y"}, T::Hash[Symbol, Integer]))
      end

      it "does do recursive type-checking of arrays with `check_type_recursive!`" do
        assert_raises(TypeError) do
          T::Utils.check_type_recursive!({x: "y"}, T::Hash[Symbol, Integer])
        end
      end
    end
  end
end
