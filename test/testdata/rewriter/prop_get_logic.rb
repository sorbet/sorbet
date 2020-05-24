# typed: strict

class MyModel < Chalk::ODM::Document
end

class MyStruct < MyModel
  extend T::Sig

  include T::Props

  # We have some code at Stripe where Sorbet can tell that decorator has a
  # specific return type. As of the time this test was written, our RBIs don't
  # declare a sig for `T::Props#decorator`. Adding such a signature caused
  # errors in places where the decorator is actually more specific than
  # `T::Props::Decorator` (like maybe it was a Chalk::ODM::DocumentDecorator)
  # but Sorbet couldn't see that information in the types, so it flagged ~150
  # spurious missing method errors.
  sig {returns(T::Props::Decorator)}
  def self.decorator
    T::Props::Decorator.new(self)
  end

  prop :foo, String
end
