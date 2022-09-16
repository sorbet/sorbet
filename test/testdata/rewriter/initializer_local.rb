# typed: strict

class A
  extend T::Sig

  sig {params(y: T.nilable(String)).void}
  #           ^ error: Unknown argument name `y`
  def initialize
    y = 1
    @y = y # error: The instance variable `@y` must be declared
  end
end
