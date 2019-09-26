# typed: true

class Parent
  extend T::Helpers
  extend T::Sig

  abstract!

  sig {returns(T.nilable(AttachedClass))}
  def self.make()
    # TODO(trevor): It would be nice if this could be `new`, but that's running
    # into problems when checking the concrete type returned by `Class#new`
    # against the `AttachedClass` param.
    nil
  end
end

class Child < Parent; end
class GrandChild < Child; end

T.reveal_type(Child.make) # error: Revealed type: `T.nilable(Child)`
T.reveal_type(GrandChild.make) # error: Revealed type: `T.nilable(GrandChild)`

T.reveal_type(T::Array[Integer].new) # error: Revealed type: `T::Array[Integer]`

# Ensure that untyped generics still work correctly
T.reveal_type(Array.new) # error: Revealed type: `T::Array[T.untyped]`

# File is an interesting case because its `Elem` type member is fixed as String.
# When AttachedClass is bounded at the wrong time, the use of `externalType`
# will default this incorrectly, and the use of `File.new` without specifying
# the parameters will cause this to turn into `T.untyped` instead.
T.reveal_type(File.new("foo", "r").first) # error: Revealed type: `T.nilable(String)`
