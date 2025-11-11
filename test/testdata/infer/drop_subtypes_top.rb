# typed: true

class A
  extend T::Sig

  sig { params(x: T.anything).void }
  def example(x)
    case x
    when Object
      T.reveal_type(x) # error: `Object`
    else
      T.reveal_type(x) # error: `T.anything`
    end
  end
end

class Box
  extend T::Sig, T::Generic

  X = type_member { {upper: T.anything} }

  sig { params(x: X).void }
  def example(x)
    case x
    when Object
      T.reveal_type(x) # error: `T.all(Object, Box::X)`
    else
      T.reveal_type(x) # error: `Box::X`
    end
  end
end
