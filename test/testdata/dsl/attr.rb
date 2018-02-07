# @typed
class TestAttr
  declare_variables(
    :@v1 => Integer,
    :@v2 => String,
  )

  sig.returns(Integer)
  attr :v1
  sig(arg0: Integer).returns(Integer)
  attr_writer :v1

  sig.returns(String)
  attr_accessor :v2

  sig.returns(String)
  attr_reader :v3 # error: Use of undeclared variable

  attr_writer :v4, :v5 # error: Use of undeclared variable
end
