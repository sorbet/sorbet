# typed: true

module Unsatisfied
  extend T::Generic # error: `T.class_of(Unsatisfied)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `Unsatisfied` is part of a subpackage namespace, and must not be generic
end
