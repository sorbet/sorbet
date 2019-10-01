# typed: true

class Parent
  extend T::Helpers
  extend T::Sig

  abstract!

  # This should fail, as `T.attach_class` is not currently supported inside of
  # method bodies.
  sig {returns(T::Array[T.attached_class])}
                      # ^^^^^^^^^^^^^^^^ error: Only top-level T.attached_class is supported
  def self.bad_usage
    []
  end


  # This should fail, as `T.attached_class` doesn't make sense in an instance
  # method context.
  sig {returns(T.nilable(T.attached_class))}
                       # ^^^^^^^^^^^^^^^^ error: `T.attached_class` may only be
  def bad_sig
    nil
  end

  sig {returns(T.nilable(T.attached_class))}
  def self.make()
    # TODO(trevor): It would be nice if this could be `new`, but that's running
    # into problems when checking the concrete type returned by `Class#new`
    # against the `T.attached_class` param.
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
# When `T.attached_class` is bounded at the wrong time, the use of `externalType`
# will default this incorrectly, and the use of `File.new` without specifying
# the parameters will cause this to turn into `T.untyped` instead.
T.reveal_type(File.new("foo", "r").first) # error: Revealed type: `T.nilable(String)`
