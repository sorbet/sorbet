# typed: strict
extend T::Sig

sig {returns(T.nilable(Integer))}
def nilable_integer()
  nil
end

sig {params(item: T.untyped).void}
def example1(item)
  to_date = T.let(item["to_date"], T.nilable(String))
  to_date = to_date.nil? ? nil : T.let(Date.parse(to_date).to_time.utc, Time)
  nil
end

sig {params(item: T.untyped).void}
def example2(item)
  to_date = T.let(item["to_date"], T.nilable(String))
  to_date = T.let(to_date.nil? ? nil : Date.parse(to_date).to_time.utc, T.nilable(Time))
  nil
end

sig { params(params: Integer).void }
def example3(params)
  x = T.unsafe(nil) || T.let(0, Integer)
  x = T.let(0, Integer)
  p(x)
  nil
end

sig { params(params: Integer).void }
def example4(params)
  x = T.unsafe(nil)
  x ||= 0 # error: Incompatible assignment to variable declared via `let`: `Integer(0)` is not a subtype of `T.nilable(FalseClass)`
  x = T.let(0, Integer)
  nil
end

sig { params(params: Integer).void }
def example5(params)
  x = T.unsafe(nil)
  x ||= T.let(0, Integer)
  x = T.let(0, Integer)
  nil
end

sig { params(x: T.nilable(Integer)).void }
def example6(x)
  1.times do
    y = x
    x = y || T.let(
      (T.reveal_type(x); 0), # error: `NilClass`
      T.nilable(Integer)
    )
  end
  nil
end

sig { params(x: T.nilable(Integer)).void }
def example7(x)
  1.times do
    y = x
    x = y
    x ||= T.let(
      (T.reveal_type(x); 0), # error: `NilClass`
      T.nilable(Integer)
    )
  end
  nil
end

sig { params(x: T.nilable(Integer)).void }
def example8(x)
  1.times do
    y = nilable_integer()
    x = y || T.let(
      (T.reveal_type(x); 0), # error: `T.nilable(Integer)`
      T.nilable(Integer)
    )
  end
  nil
end
