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
  {}
end
