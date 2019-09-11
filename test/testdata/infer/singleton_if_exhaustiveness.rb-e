# typed: strict
extend T::Sig

sig {params(x: T::Boolean).void}
def all_cases_handled(x)
  if x == true
    T.reveal_type(x) # error: Revealed type: `TrueClass`
  elsif x == false
    T.reveal_type(x) # error: Revealed type: `FalseClass`
  else
    T.absurd(x)
  end
end

sig {params(x: T::Boolean).void}
def missing_false(x)
  if x == true
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
  end
end

sig {params(x: T::Boolean).void}
def commutative_all_cases_handled(x)
  if true == x
    T.reveal_type(x) # error: Revealed type: `TrueClass`
  elsif false == x
    T.reveal_type(x) # error: Revealed type: `FalseClass`
  else
    T.absurd(x)
  end
end

sig {params(x: T::Boolean).void}
def commutative_missing_false(x)
  if true == x
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
  end
end

