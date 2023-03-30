# typed: strict
class Module; include T::Sig; end

module Thing
  extend T::Helpers
  interface!

  sig {abstract.returns(Integer)}
  def foo; end

  module Factory
    extend T::Generic
    interface!

    has_attached_class!

    sig {abstract.returns(T.attached_class)}
    def new; end
  end
  mixes_in_class_methods(Factory)
end

sig do
  type_parameters(:Instance)
    .params(
      klass: Thing::Factory[T.all(Thing, T.type_parameter(:Instance))]
    )
    .returns(T.type_parameter(:Instance))
end
def instantiate_class_good(klass)
  instance = klass.new
  T.reveal_type(instance.foo) # error: `Integer`
  instance
end

class Child
  extend T::Generic
  include Thing

  sig {override.returns(Integer)}
  def foo; 0; end
end

class GrandChild < Child; end

instance = instantiate_class_good(Child)
T.reveal_type(instance) # error: `Child`

sig do
  params(thing_factory: Thing::Factory[Child]).void
end
def example(thing_factory)
end

# To pass, this requires declaring the type member with covariance
instance = example(GrandChild)
#                  ^^^^^^^^^^ error: Expected `Thing::Factory[Child]` but found `T.class_of(GrandChild)` for argument `thing_factory`
