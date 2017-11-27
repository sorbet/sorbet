class A
  A.declare_variables

  declare_variables({}) do # error: Malformed `declare_variables'
  end

  declare_variables(4)     # error: Argument must be a hash
  declare_variables({}, 1) # error: Wrong number of arguments
  declare_variables(
    llamas: Integer,       # error: variables must start with @ or @@
  )

  declare_variables(
    :@llamas => int_type,  # error: Unknown type syntax
  )

  declare_variables(
    :@foo => Integer,
  )

  declare_variables(
    :@foo => Integer,      # error: Redeclaring variable `@foo'
  )
end
