# typed: true
class Test
  extend T::Helpers

  def required(x); end
  def optional(x, y=0, z=0); end
  def repeated(x, y=0, *z); end

  def test
    required() # error: Expected: `1`, got: `0`
    required(0)
    required(0, 1) # error: Expected: `1`, got: `2`

    optional() # error: Expected: `1..3`, got: `0`
    optional(0, 1)
    optional(0, 1, 2, 3) # error: Expected: `1..3`, got: `4`

    repeated() # # error: Expected: `1+`, got: `0`
  end
end
