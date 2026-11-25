# typed: true
extend T::Sig

sig {params(x: Integer).returns(T.nilable(Integer))}
def let_in_else(x)
  if x > 100
    x
  else
    T.let(nil, T.nilable(Integer))
  end
end

sig {params(x: Integer).returns(T.nilable(Integer))}
def let_in_then(x)
  if x > 100
    T.let(nil, T.nilable(Integer))
  else
    x
  end
end

sig {params(xs: T::Array[Integer]).returns(T::Array[T.nilable(Integer)])}
def let_in_block(xs)
  xs.map do |x|
    if x > 100
      x
    else
      T.let(nil, T.nilable(Integer))
    end
  end
end

sig {params(type: String).returns(T.nilable(T.any(Integer, Symbol)))}
def let_in_case(type)
  case type
  when "integer"
    T.let(1, Integer)
  when "file"
    :skip
  end
end

sig {params(a: T::Boolean, b: T.untyped).returns(T::Boolean)}
def let_in_and(a, b)
  a && T.let(b, T::Boolean)
end

sig {params(params: T::Hash[Symbol, T.untyped], errors: T::Array[String]).void}
def let_in_begin_rescue(params, errors)
  begin
    T.let(params[:contents], T::Hash[String, String])
  rescue TypeError
    errors << "contents must be a hash"
  end
end

sig {params(xs: T::Array[Integer]).void}
def let_in_block_with_rescue(xs)
  ys = xs.map do |x|
    T.let(x, Integer)
  rescue
    "s"
  end
  T.reveal_type(ys) # error: Revealed type: `T::Array[T.any(Integer, String)]`
end
