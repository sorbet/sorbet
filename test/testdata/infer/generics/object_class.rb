# typed: true
class Model
  extend T::Sig

  sig {returns(T.attached_class)}
  def self.load_one
    new
  end
end

class Get
  extend T::Sig
  extend T::Generic

  ModelType = type_member { {upper: Model} }

  sig {params(instance: ModelType).returns(ModelType)}
  def get(instance)
    T.reveal_type(instance) # error: `Get::ModelType`
    T.reveal_type(instance.class) # error: `T.class_of(Model)[T.all(Model, Get::ModelType)]`
    x = instance.class.load_one
    T.reveal_type(x) # error: `T.all(Model, Get::ModelType)`
    x
  end
end

class A
end

module M
end

extend T::Sig

sig {params(x: T.all(A, M)).void}
def example(x)
  T.reveal_type(x.class) # error: `T.class_of(A)[T.all(A, M)]`
  T.reveal_type(x.class.new) # error: `T.all(A, M)`
end
