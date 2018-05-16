# typed: true
class ObjectSpace::WeakMap < Object
  include Enumerable

  extend T::Generic
  Elem = type_member(:out)
end
