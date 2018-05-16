# typed: true
class StringIO < Data
  include Enumerable

  extend T::Generic
  Elem = type_member(:out, fixed: String)
end
