# typed: true

extend T::Sig

sig {params(x: T.any(Integer, String)).returns(T.any(Integer, String))}
def foo(x)
  case x
  when 5
    T.reveal_type(x) # error: Revealed type: `Integer(5)`
    x
  when "hello"
    T.reveal_type(x) # error: Revealed type: `String("hello")`
  when String
    T.reveal_type(x) # error: Revealed type: `String`
    x
  else
    T.reveal_type(x) # error: Revealed type: `Integer`
    x
  end
end

sig {params(x: T.any(Symbol, Float)).void}
def symbol_and_float(x)
  case x
  when :foo
    T.reveal_type(x) # error: Revealed type: `Symbol(:foo)`
  when 3.14
    T.reveal_type(x) # error: Revealed type: `Float(3.140000)`
  else
    T.reveal_type(x) # error: Revealed type: `T.any(Symbol, Float)`
  end
end

sig {params(x: Integer).void}
def literal_narrows_within_same_type(x)
  case x
  when 1
    T.reveal_type(x) # error: Revealed type: `Integer(1)`
  else
    T.reveal_type(x) # error: Revealed type: `Integer`
  end
end

sig {params(x: T.any(Integer, String)).void}
def multiple_literals(x)
  case x
  when 5, 6
    T.reveal_type(x) # error: Revealed type: `Integer`
  when "a", "b"
    T.reveal_type(x) # error: Revealed type: `String`
  else
    T.reveal_type(x) # error: Revealed type: `T.any(Integer, String)`
  end
end
