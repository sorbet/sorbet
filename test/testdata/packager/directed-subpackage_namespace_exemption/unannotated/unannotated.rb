# typed: true

module Unannotated
  extend T::Generic # error: `T.class_of(Unannotated)` is part of a subpackage namespace, and must not have ancestors
  Elem = type_member # error: `Unannotated` is part of a subpackage namespace, and must not be generic
end
