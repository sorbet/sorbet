# typed: strong

x = ["blah", nil].sample
T.reveal_type(x) # error: Revealed type: `T.nilable(String)`

case x
when 'blah'
  T.reveal_type(x) # error: Revealed type: `String("blah")`
when String
  T.reveal_type(x) # error: Revealed type: `String`
else
  T.reveal_type(x) # error: Revealed type: `NilClass`
end
