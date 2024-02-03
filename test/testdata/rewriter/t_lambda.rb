# typed: true

lam = T.lambda do |x: Integer, y: String|
  T.reveal_type(x) # error: Revealed type: `Integer`
  T.reveal_type(y) # error: Revealed type: `String`
  y.upcase * x
end

lam.call(2, "foo")
