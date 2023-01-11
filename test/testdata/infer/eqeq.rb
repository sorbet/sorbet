# typed: true
extend T::Sig

class ConvertsToInteger
  def ==(other)
    other.is_a?(ConvertsToInteger) || other == 0
  end
end

class MyEnum < T::Enum
  enums do
    X = new
    Y = new
  end
end

class A < T::Struct
  const :sym, Symbol
  const :str, String
  const :maybe_sym, T.nilable(Symbol)
  const :maybe_str, T.nilable(String)
  const :str_or_sym, T.any(String, Symbol)
  const :int_or_sym, T.any(Integer, Symbol)
  const :str_or_int, T.any(String, Integer)
  const :converts_to_integer, ConvertsToInteger
  const :my_enum, MyEnum
end

sig do
  params(x: A).void
end
def example1(x)
  if x.sym == x.str
    #      ^^ error: Comparison between `Symbol` and `String` is always false
    p(x)
  end

  if x.str == x.sym
    #      ^^ error: Comparison between `String` and `Symbol` is always false
    p(x)
  end

  if x.sym == x.maybe_str
    #      ^^ error: Comparison between `Symbol` and `T.nilable(String)` is always false
    p(x)
  end

  if x.str == x.maybe_sym
    #      ^^ error: Comparison between `String` and `T.nilable(Symbol)` is always false
    p(x)
  end

  if x.maybe_sym == x.str
    #            ^^ error: Comparison between `T.nilable(Symbol)` and `String` is always false
    p(x)
  end

  if x.maybe_str == x.sym
    #            ^^ error: Comparison between `T.nilable(String)` and `Symbol` is always false
    p(x)
  end


  if x.maybe_sym == x.maybe_str
    p(x)
  end

  if x.str_or_sym == x.str
    p(x)
  end

  if x.str_or_sym == x.sym
    p(x)
  end

  if x.sym == x.str_or_sym
    p(x)
  end

  if x.str == x.str_or_sym
    p(x)
  end

  if x.str_or_sym == x.str_or_sym
    p(x)
  end

  if x.int_or_sym == x.str_or_sym
    p(x)
  end

  if x.str_or_int == x.maybe_sym
    p(x)
  end

  if x.str_or_int == x.str_or_sym
    p(x)
  end

  if x.int_or_sym == x.maybe_str
    p(x)
  end

  if x.maybe_sym == x.str_or_int
    #            ^^ error: Comparison between `T.nilable(Symbol)` and `T.any(String, Integer)` is always false
    p(x)
  end

  if x.int_or_sym == x.converts_to_integer
    p(x)
  end

  if x.sym == x.my_enum
    #      ^^ error: Comparison between `Symbol` and `MyEnum` is always false
    p(x)
  end

  if x.maybe_sym == x.my_enum
    #            ^^ error: Comparison between `T.nilable(Symbol)` and `MyEnum` is always false
    p(x)
  end

  if x.int_or_sym == x.my_enum
    p(x)
  end
end



