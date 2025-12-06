# typed: strict

class Module
  include T::Sig

  sig { params(other: T.anything).returns(T.nilable(T.attached_class)) }
  def downcast(other)
    T.reveal_type(self) # error: `T.self_type (of T::Module[T.attached_class])`
    case other
    when self
      T.reveal_type(other) # error: `T.attached_class`
      return other
    else
      return nil
    end
  end
end
