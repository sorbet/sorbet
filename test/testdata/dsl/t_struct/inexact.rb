# typed: true

class Inexact < T::InexactStruct
  prop :foo, Integer
  prop :bar, String
end

Inexact.new
Inexact.new(foo: 3, bar: "hey") # error: Wrong number of arguments for constructor. Expected: `0`, got: `1`
