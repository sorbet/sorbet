# typed: true

class Parent
  extend T::Sig, T::Helpers
  abstract!
  sealed!

  class Child1 < Parent; end
  class Child2 < Parent; end

  sig { returns(T.attached_class) }
  def self.make_and_inspect
    inst = self.new
    T.reveal_type(inst) # error: `T.attached_class (of Parent)`
    case inst
    when Child1
      T.reveal_type(inst) # error: `T.all(Parent::Child1, T.attached_class (of Parent))`
    when Child2
      T.reveal_type(inst) # error: `T.all(Parent::Child2, T.attached_class (of Parent))`
    else
      T.absurd(inst)
    end
  end

  sig { returns(T.attached_class) }
  def self.make_and_inspect_partial
    inst = self.new
    T.reveal_type(inst) # error: `T.attached_class (of Parent)`
    case inst
    when Child1
      T.reveal_type(inst) # error: `T.all(Parent::Child1, T.attached_class (of Parent))`
    else
      T.absurd(inst) # error: Control flow could reach `T.absurd` because the type `T.all(Parent::Child2, T.attached_class (of Parent))` wasn't handled
    end
  end
end
