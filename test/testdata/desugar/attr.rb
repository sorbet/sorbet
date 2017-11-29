class TestAttr
  declare_variables(
    :@v1 => Integer,
    :@v2 => String,
  )

  attr :v1
  attr_accessor :v2
  attr_reader :v3       # error: undeclared variable
  attr_writer :v4, :v5  # error: MULTI
end
