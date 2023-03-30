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

    has_attached_class!(:out) { {upper: Thing} }

    sig {abstract.returns(T.attached_class)}
    def make_thing; end
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
def instantiate_class(klass)
  instance = klass.make_thing
  instance.foo
  instance
end

class GoodThing
  extend T::Generic
  include Thing

  sig {override.returns(Integer)}
  def foo; 0; end

  sig {override.returns(T.attached_class)}
  def self.make_thing
    new
  end
end

class ChildGoodThing < GoodThing; end

instance = ChildGoodThing.make_thing
T.reveal_type(instance) # error: `ChildGoodThing`

sig do
  params(thing_factory: Thing::Factory[GoodThing]).void
end
def example(thing_factory)
end

# Both allowed, because of covariance
example(GoodThing)
example(ChildGoodThing)
