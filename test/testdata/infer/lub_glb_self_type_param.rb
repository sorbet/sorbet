# typed: true
extend T::Sig

class ObjectBox
  extend T::Sig, T::Generic

  Elem = type_member { {upper: Object} }

  sig { params(val: Elem).void }
  def initialize(val)
    @val = val
  end

  sig { params(val: T.all(Object, Elem)).void }
  def example(val)
    T.reveal_type(val) # error: `ObjectBox::Elem`
  end

  sig { returns(Elem) }
  def dup_val
    klass = @val.class
    T.reveal_type(klass) # error: `T.class_of(Object)[ObjectBox::Elem]`
    new_val = klass.new
    T.reveal_type(new_val) # error: `ObjectBox::Elem`
    new_val
  end
end

class Maybe
  extend T::Sig, T::Generic

  Elem = type_member { {lower: NilClass} }

  sig { params(val: Elem).void }
  def initialize(val: nil)
    @val = val
  end

  sig { params(val: T.nilable(Elem)).void }
  def example(val)
    T.reveal_type(val) # error: `Maybe::Elem`
  end

  sig { void }
  def clear!
    @val = nil
  end

  sig { returns(Elem) }
  def val = @val

  sig { params(val: Elem).returns(Elem) }
  def val=(val)
    @val = val
  end
end

Maybe[Integer].new(val: 0)
#     ^^^^^^^ error: `Integer` is not a supertype of lower bound of type member `::Maybe::Elem`

sig { params(x: Maybe[T.nilable(Integer)]).void }
def example(x)
  val = x.val
  T.reveal_type(val) # error: `T.nilable(Integer)`
end

class CompoundBounds
  extend T::Sig, T::Generic
  Elem = type_member { {lower: Numeric, upper: Object} }

  sig { params(val: T.all(T.any(Kernel, PP::ObjectMixin), Elem)).void }
  def example_glb_1(val)
    T.reveal_type(val) # error: `CompoundBounds::Elem`
  end

  sig { params(val: T.all(Elem, T.any(Kernel, PP::ObjectMixin))).void }
  def example_glb_2(val)
    T.reveal_type(val) # error: `CompoundBounds::Elem`
  end

  sig { params(val: T.all(Elem, T.any(Integer, String))).void }
  def example_glb_3(val)
    T.reveal_type(val) # error: `T.all(CompoundBounds::Elem, T.any(Integer, String))`
  end

  sig { params(val: T.any(T.any(Integer, Float), Elem)).void }
  def example_lub_1(val)
    T.reveal_type(val) # error: `CompoundBounds::Elem`
  end

  sig { params(val: T.any(Elem, T.any(Integer, Float))).void }
  def example_lub_2(val)
    T.reveal_type(val) # error: `CompoundBounds::Elem`
  end

  sig { params(val: T.any(Elem, T.any(Comparable, Object))).void }
  def example_lub_3(val)
    T.reveal_type(val) # error: `T.any(Comparable, Object, CompoundBounds::Elem)`
  end
end
