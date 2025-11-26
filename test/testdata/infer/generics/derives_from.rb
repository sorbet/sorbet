# typed: true
class Module; include T::Sig; end

class Parent
  def self.on_parent; end
end

class A
  extend T::Generic
  abstract!
  Klass = type_member { {upper: Module} }

  sig { abstract.returns(Klass) }
  def klass; end

  sig { void }
  def example
    cls = klass
    T.reveal_type(cls) # error: `A::Klass`
    if cls <= Parent
      T.reveal_type(cls) # error: `T.all(T.class_of(Parent), A::Klass)`
      cls.on_parent
    end
  end
end
