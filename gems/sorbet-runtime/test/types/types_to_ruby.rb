# frozen_string_literal: true
require_relative '../test_helper'

class Opus::Types::Test::TypesToRubyTest < Critic::Unit::UnitTest
  cases = [
    # Basic:
    [String, "String"],
    [Integer, "Integer"],
    [Numeric, "Numeric"],
    [T::Types, "T::Types"],
    [self, "Opus::Types::Test::TypesToRubyTest"],
    [Symbol, "Symbol"],

    # Nilable:
    [T.nilable(String), "T.nilable(String)"],

    # Union:
    [T.any(Integer, String), "T.any(Integer, String)"],
    [T.any(Integer, String, NilClass), "T.nilable(T.any(Integer, String))"],

    # Intersection:
    [T.all(Integer, String), "T.all(Integer, String)"],

    # TypedArray:
    [T::Array[String], "T::Array[String]"],

    # FixedArray:
    [[String, T.nilable(Integer)], "[String, T.nilable(Integer)]"],

    # FixedHash:
    [{a: T.nilable(String)}, "{a: T.nilable(String)}"],
    [{"a" => T.nilable(String)}, "{\"a\" => T.nilable(String)}"],
    [{"a" => String, b: T.any(Integer, Float)}, "{\"a\" => String, b: T.any(Float, Integer)}"],
    [{"foo bar" => String, :"foo bar" => T.any(Integer, Float)}, "{\"foo bar\" => String, :\"foo bar\" => T.any(Float, Integer)}"],

    # TypedHash:
    [T::Hash[T.any(String, Symbol), String], "T::Hash[T.any(String, Symbol), String]"],
    [T::Hash[String, String], "T::Hash[String, String]"],

    # Nilable TypedHash
    [T.nilable(T::Hash[String, String]), "T.nilable(T::Hash[String, String])"],

    # Enum:
    [T.deprecated_enum(%w[a b c]), 'T.deprecated_enum(["a", "b", "c"])'],

    # Range:
    [T::Range[Integer], "T::Range[Integer]"],

    # Set:
    [T::Set[Integer], "T::Set[Integer]"],

    # T.type_parameter:
    [T.type_parameter(:A), "T.type_parameter(:A)"],
  ]

  cases.each do |c|
    it "maps #{c[0]} to #{c[1]}" do
      rubby = T::Utils.coerce(c[0]).to_s
      assert_equal(c[1], rubby)
    end
  end
end
