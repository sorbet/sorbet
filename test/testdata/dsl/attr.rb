# typed: strict
class TestAttr
  extend T::Sig

  sig {void}
  def initialize
    @v1 = T.let(0, Integer)
    @v2 = T.let("", String)
    @v6 = T.let("", String)
  end

  sig {returns(Integer)}
  attr :v1
  sig {params(v1: Integer).returns(Integer)}
  attr_writer :v1

  sig {returns(String)}
  attr_accessor :v2

  sig {returns(String)}
  attr_reader :v3 # error: Use of undeclared variable

  attr_writer :v4, :v5 # error: This function does not have a `sig`
# ^^^^^^^^^^^^^^^^^^^^ error: Use of undeclared variable `@v4`
# ^^^^^^^^^^^^^^^^^^^^ error: Use of undeclared variable `@v5`

  Sorbet.sig {returns(String)}
  attr_accessor :v6
end
