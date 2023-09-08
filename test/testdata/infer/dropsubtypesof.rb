# typed: true

extend T::Sig

# This test exercises some edge cases / bugs we found in dropSubtypesOf where
# dropping NilClass from an OrType would cause the resulting type to not be a
# subtype of what we started with. In particular, the bug was that
#
#     dropSubtypesOf(NilClass | Array[Integer] | Array[String],  NilClass)
#
# became
#
#     Array[Integer | String]
#
# and this is not a subtype of the input.

# Right associative
sig {params(x: T.any(NilClass, T.any(T::Array[Integer], T::Array[String]))).void}
def foo(x)
  if x.nil?
    puts "got nil"
  else
    y = T.let(x, T.any(T::Array[Integer], T::Array[String]))
    T.reveal_type(y) # error: Revealed type: `T::Array[T.any(Integer, String)]`
  end
end

# Left associative
sig {params(x: T.any(T.any(T::Array[Integer], T::Array[String]), NilClass)).void}
def bar(x)
  if x.nil?
    puts "got nil"
  else
    y = T.let(x, T.any(T::Array[Integer], T::Array[String]))
    T.reveal_type(y) # error: Revealed type: `T::Array[T.any(Integer, String)]`
  end
end

# Whatever associativity Types::any happens to use
sig {params(x: T.any(NilClass, T::Array[Integer], T::Array[String])).void}
def qux(x)
  if x.nil?
    puts "got nil"
  else
    y = T.let(x, T.any(T::Array[Integer], T::Array[String]))
    T.reveal_type(y) # error: Revealed type: `T::Array[T.any(Integer, String)]`
  end
end

x = T::Array[T.any(String, Integer)].new
# This should error but doesn't, because T.any lub's its arguments, and
# currently the lub gives back Array[Integer | String] instead of
# Array[Integer] | Array[String]
T.assert_type!(x, T.any(T::Array[Integer], T::Array[String]))
