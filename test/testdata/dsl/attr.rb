# typed: strict
class TestAttr
  def initialize
    @v1 = T.let(0, Integer)
    @v2 = T.let("", String)
  end

  sig.returns(Integer)
  attr :v1
  sig(v1: Integer).returns(Integer)
  attr_writer :v1

  sig.returns(String)
  attr_accessor :v2

  sig.returns(String)
  attr_reader :v3 # error: Use of undeclared variable

  attr_writer :v4, :v5 # error: Use of undeclared variable
end
