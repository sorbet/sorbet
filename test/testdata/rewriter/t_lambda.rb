# typed: true

lam = T.lambda do |x: Integer, y: String|
  T.reveal_type(x) # error: Revealed type: `Integer`
  T.reveal_type(y) # error: Revealed type: `String`
  y.upcase * x
end

lam.call(2, "foo")

T.lambda do |num|
           # ^^^ error: must have a type
end

T.lambda do |num=Integer|
          #  ^^^^^^^^^^^ error: must use keyword syntax
end
