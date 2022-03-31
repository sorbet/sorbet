# typed: strict

class None < T::Struct; end

None.new
  None.new(foo: 3)
#          ^^^^^^ error: Too many arguments provided for method `None#initialize`. Expected: `0`, got: `1`
