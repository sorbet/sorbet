# typed: strict
class TestAttr
  extend T::Sig

  sig {void}
  def initialize
    @attr1 = T.let(0, Integer)
    @attr2 = T.let(0, Integer)
    @attr3 = T.let(0, Integer)
    @attr4 = T.let(0, Integer)
    @attr5 = T.let(0, Integer)
    @attr_no_writer = T.let(0, Integer)
    @attr_with_writer = T.let(0, Integer)

    @v1 = T.let(0, Integer)
    @v2 = T.let("", String)
    @v6 = T.let("", String)
    @strv7 = T.let(0.0, Float)
    @strv8 = T.let(0.0, Float)
    @strv9 = T.let(0.0, Float)
  end

  sig { returns(Integer) }
# ^^^^^^^^^^^^^^^^^^^^^^^^ error: Unused type annotation. No method def before next annotation
  attr

  sig { returns(Integer) }
  attr :attr1

  sig { returns(Integer) }
  attr :attr2, :attr3

  sig { returns(Integer) }
  attr :attr3, :attr4, :attr5

  sig { returns(Integer) }
  attr :attr_no_writer, false

  sig { returns(Integer) }
  attr :attr_with_writer, true

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
