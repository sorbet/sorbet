# typed: strict
extend T::Sig

sig { returns(Symbol) }
def test_strings
  %q{}
  "foo"
  T.reveal_type(X) # error: `T.untyped`
  "foo#{1}" # error: Expected `Symbol` but found `String`
end

X = "#{}"
#   ^^^^^ error: Constants must have type annotations
