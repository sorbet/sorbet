# typed: true

expr = "x"
T.reveal_type(expr) # error: Revealed type: `String("x")`

x = case expr
in "a" => s # local var from the AsMatch
T.reveal_type(s) # error: Revealed type: `T.untyped`
  1
in "b" => x # local var different from the assign line 6
  T.reveal_type(x) # error: Revealed type: `T.untyped`
  1
in "c" => expr # local var hiding the assign line 3
  T.reveal_type(expr) # error: Revealed type: `T.untyped`
  1
in "d"
  # `x` is undefined at this point
  T.reveal_type(x) # error: Revealed type: `NilClass`
  T.reveal_type(expr) # error: Revealed type: `String("x")`
  1
else
  1
end

T.reveal_type(x) # error: Revealed type: `Integer(1)`
