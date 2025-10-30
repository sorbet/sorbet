# typed: true
extend T::Sig

module IParent
  extend T::Helpers
  module CM
    extend T::Generic
    has_attached_class!(:out)
  end
  mixes_in_class_methods(CM)
end

module IChild
  include IParent
end

class Child
  include IChild
end

sig {
  params(
    x: Object,
    mod: T::Module[IParent]
  ).void
}
def example1(x, mod)
  if x.is_a?(mod)
    T.reveal_type(x) # error: `T.all(Object, IParent)`
  end
end

example1(Child.new, IParent)
example1(Child.new, IChild)
example1(Child.new, Child)

T.let(Child, IParent::CM[T.anything]) # error: Argument does not have asserted type `IParent::CM[T.anything]`
