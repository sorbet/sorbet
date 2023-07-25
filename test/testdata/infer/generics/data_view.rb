# typed: strict
# TODO(jez) minimize these tests a bit more

class Module; include T::Sig; end


class Model
  include T::Props

  sig {returns(T.self_type)}
  def reload
    self
  end
end

class View
  extend T::Generic
  abstract!

  LinkedGenericTypePair = type_member { {upper: Model} }

  sig {params(instance: LinkedGenericTypePair).void}
  def initialize(instance)
    @instance = instance
  end

  sig {returns(LinkedGenericTypePair)}
  def instance
    @instance
  end

  sig {returns(T.self_type).checked(:tests)}
  def reload
    reloaded = T.reveal_type(instance.reload)
    klass = self.class
    T.reveal_type(klass)
    klass.from_instance('')
    result = klass.from_instance(reloaded)
    T.reveal_type(result)
    result
  end

  class << self
    extend T::Generic
    LinkedGenericTypePair = type_member { {upper: Model} }

    sig { params(instance: LinkedGenericTypePair).returns(T.attached_class) }
    def from_instance(instance)
      instance.class.prop

      # Not pictured: could use props to forward values to `new`
      props = instance.class.props
      T.reveal_type(props)

      T.reveal_type(self)
      new('')

      result = new(instance)
      T.reveal_type(result)

      result2 = self[LinkedGenericTypePair].new(instance)
      self[LinkedGenericTypePair].new('')
      result2
    end
  end
end
