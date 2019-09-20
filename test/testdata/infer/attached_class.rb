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

# TODO(trevor): why is Integer resolving to `T.class_of(Integer)`?
T.reveal_type(T::Array[Integer].new) # error: Revealed type: `T::Array[T.class_of(Integer)]`
