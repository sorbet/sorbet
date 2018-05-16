# typed: true
class SortedSet < Set
  extend T::Generic
  Elem = type_member(:out)
end
