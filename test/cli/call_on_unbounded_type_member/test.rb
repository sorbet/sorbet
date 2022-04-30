# typed: true
extend T::Sig

class A
  def only_on_a; end
end
class B; end

sig {params(a: A).void}
def takes_a(a); end

class Box
  extend T::Sig
  extend T::Generic

  Elem = type_member

  sig {params(x: Elem).void}
  def initialize(x)
    x.only_on_a
    takes_a(x)
  end
end

Box[A].new(A.new)
Box[B].new(B.new)

module HasProps
  extend T::Helpers
  def prop(name, type); end

  module ClassMethods
    extend T::Sig
    sig {returns(T::Array[Symbol])}
    def all_props; []; end
  end
  mixes_in_class_methods(ClassMethods)
end

class ChalkODMDocument
  include HasProps
end

class DataView
  extend T::Sig
  extend T::Generic
  DataViewModelBad = type_template
  DataViewModel = type_template {{upper: ChalkODMDocument}}

  sig {params(model: DataViewModelBad).returns(T.attached_class)}
  def self.bad_from_instance(model)
    model.class.props
    new()
  end

  sig {params(model: DataViewModel).returns(T.attached_class)}
  def self.from_instance(model)
    props = model.class.all_props
    T.reveal_type(props)

    model.does_not_exist
    new()
  end
end
