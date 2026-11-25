# typed: strict
extend T::Sig

sig { returns(Symbol) }
def test_strings
  %q{}
  "foo"
  T.reveal_type(X) # error: `String`
  "foo#{1}" # error: Expected `Symbol` but found `String`
end

X = "#{}"
