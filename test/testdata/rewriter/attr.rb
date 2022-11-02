# typed: strict
class TestAttr
  extend T::Sig

  sig {void}
  def initialize
    @v1 = T.let(0, Integer)
    @v2 = T.let("", String)
    @v6 = T.let("", String)
    @strv7 = T.let(0.0, Float)
    @strv8 = T.let(0.0, Float)
    @strv9 = T.let(0.0, Float)
  end

  sig {returns(Integer)}
  attr :v1
  sig {params(v1: Integer).returns(Integer)}
  attr_writer :v1

  sig {returns(String)}
  attr_accessor :v2

  sig {returns(String)}
  attr_reader :v3 # error: Use of undeclared variable `@v3`

  attr_writer :v4, :v5
# ^^^^^^^^^^^^^^^^^^^^ error: The method `v4=` does not have a `sig`
# ^^^^^^^^^^^^^^^^^^^^ error: The method `v5=` does not have a `sig`
  #            ^^        error: The instance variable `@v4` must be declared using `T.let` when specifying `# typed: strict`
  #                 ^^   error: The instance variable `@v5` must be declared using `T.let` when specifying `# typed: strict`

  sig {returns(Float)}
  attr_reader "strv7"

  sig {params(strv8: Float).returns(Float)}
  attr_writer "strv8"

  sig {returns(Float)}
  attr_accessor "strv9"
end
