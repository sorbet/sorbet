# typed: true

module Parent
  extend T::Generic
  Elem = type_member(:out)
end

module Child
  extend T::Generic
  include Parent
  Elem = type_member(:in)
end
