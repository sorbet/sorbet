# typed: strict
extend T::Sig

sig {params(x: T.nilable(T::Boolean)).void}
def all_cases_handled(x)
  case x
  when true
    T.reveal_type(x) # error: Revealed type: `TrueClass`
  when false
    T.reveal_type(x) # error: Revealed type: `FalseClass`
  when nil
    T.reveal_type(x) # error: Revealed type: `NilClass`
  else
    T.absurd(x)
  end
end

sig {params(x: T.nilable(T::Boolean)).void}
def unhandled_true(x)
  case x
  when false
  when nil
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `TrueClass` wasn't handled
  end
end

sig {params(x: T.nilable(T::Boolean)).void}
def unhandled_false(x)
  case x
  when true
  when nil
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `FalseClass` wasn't handled
  end
end

sig {params(x: T.nilable(T::Boolean)).void}
def unhandled_nil(x)
  case x
  when true
  when false
  else
    T.absurd(x) # error: Control flow could reach `T.absurd` because the type `NilClass` wasn't handled
  end
end

