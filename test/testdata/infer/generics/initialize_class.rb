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

    initializable!

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
  T.reveal_type(instance.foo) # => Integer
  instance
end

class Child
  extend T::Generic
  include Thing

  sig {override.returns(Integer)}
  def foo; 0; end
end

instance = instantiate_class_good(Child)
T.reveal_type(instance)
