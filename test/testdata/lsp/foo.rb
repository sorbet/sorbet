# typed: true
class Bad
  extend T::Sig

  sig {type_parameters(:U).params(y: T.type_parameter(:U)).void}
  def initialize(y)
    @y2 = T.let(y, T.type_parameter(:U))
    #     ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ error: Unable to resolve declared type for `@y2`
    #                               ^^ error: Unspecified type parameter
  end
end
