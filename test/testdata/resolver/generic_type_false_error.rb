# typed: false

extend T::Sig

sig {returns(T::Array[x: Integer])}
#                     ^^^^^^^^^^ error: Keyword arguments given to `Array`
def example
  []
end

sig {returns(T::Hash[y: String, z: Integer])} 
#                    ^^^^^^^^^^^^^^^^^^^^^ error: Keyword arguments given to `Hash`
def regression
  T::Array[Integer, x: Integer].new # We don't report the type syntax error for expressions in # typed: false files
end
