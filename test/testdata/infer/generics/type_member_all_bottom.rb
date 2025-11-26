# typed: true
class Module; include T::Sig; end

class Parent
  sig { returns(String) }
  def on_parent = ""
end

class A
  extend T::Generic
  abstract!

  X = type_member(:out) { {upper: Parent} }
  Y = type_member(:out) { {upper: T.untyped} }
  Z = type_member(:out) { {upper: T.anything} }

  sig { abstract.returns(T.nilable(X)) }
  def get_x; end
  sig { abstract.returns(T.nilable(Y)) }
  def get_y; end
  sig { abstract.returns(T.nilable(Z)) }
  def get_z; end

  sig { returns(T.nilable(String)) }
  def example_x
    x = get_x

    if x
      return x.to_s
    else
      T.reveal_type(x) # error: `NilClass`
      return x
    end
  end

  sig { returns(T.nilable(String)) }
  def example_y
    y = get_y

    if y
      return y.to_s
    else
      T.reveal_type(y) # error: `NilClass`
      return y
    end
  end

  sig { returns(T.nilable(String)) }
  def example_z
    z = get_z

    if z
      return z.to_s # error: Call to method `to_s` on unbounded type member
    else
      T.reveal_type(z) # error: `T.nilable(T.all(FalseClass, A::Z))`
      return z # error: Expected `T.nilable(String)` but found `T.nilable(T.all(FalseClass, A::Z))` for method result type
    end
  end
end
