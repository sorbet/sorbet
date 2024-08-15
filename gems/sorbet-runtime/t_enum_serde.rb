# typed: true
require_relative './lib/sorbet-runtime'
extend T::Sig

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

class A < T::Struct
  prop :enum_integer, T.any(MyEnum, Integer)
  prop :enum_integer_float, T.any(MyEnum, Integer, Float)
  prop :enum_integer_symbol, T.any(MyEnum, Integer, Symbol)
  prop :enum_string, T.any(MyEnum, String)
  prop :enum_symbol, T.any(MyEnum, Symbol)
  prop :integer_enum, T.any(Integer, MyEnum)
end

a = A.new(
  enum_integer:        MyEnum::X,
  enum_integer_float:  MyEnum::X,
  enum_integer_symbol: MyEnum::X,
  enum_string:         MyEnum::X,
  enum_symbol:         MyEnum::X,
  integer_enum:        MyEnum::X,
)
pp(a.serialize)
pp(A.from_hash(a.serialize))
puts("----------------------------")

a = A.new(
  enum_integer:        123,
  enum_integer_float:  123.0,
  enum_integer_symbol: :x,
  enum_string:         "x",
  enum_symbol:         :x,
  integer_enum:        123,
)
pp(a.serialize)
pp(A.from_hash(a.serialize))
