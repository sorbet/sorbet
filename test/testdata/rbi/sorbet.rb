# typed: true
extend T::Sig

sig {returns(String)}
def foo
  2 # error: Expected `String` but found `Integer(2)` for method result type
end
