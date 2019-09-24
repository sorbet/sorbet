# typed: true

class Model
  extend T::Sig

  sig {params(args: T.untyped).returns(AttachedClass)}
  def self.load(args)
    T.unsafe(nil)
  end
end

class A < Model
end

T.reveal_type(A.load(:foo)) # error: Revealed type: `A`

T.reveal_type(T::Array[Integer].new) # error: Revealed type: `T::Array[Integer]`

# Ensure that untyped generics still work correctly
T.reveal_type(Array.new) # error: Revealed type: `T::Array[T.untyped]`

# File is an interesting case because its `Elem` type member is fixed as String.
# When AttachedClass is bounded at the wrong time, the use of `externalType`
# will default this incorrectly, and the use of `File.new` without specifying
# the parameters will cause this to turn into `T.untyped` instead.
T.reveal_type(File.new("foo", "r").first) # error: Revealed type: `T.nilable(String)`
