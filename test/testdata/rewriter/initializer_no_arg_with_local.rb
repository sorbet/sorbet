# typed: strict

class A
  extend T::Sig

  # At one point, the parser would produce local variables inside the method,
  # but no corresponding argument in the method definition.
  sig {params(x: Integer, y: T.nilable(String)).void}
  #                       ^ error: Unknown argument name `y`
  def initialize(x, y=nil, )
                # ^ error: unexpected token ","
    @x = x
    @y = y
  # ^^ error: The instance variable `@y` must be declared
  end
end
