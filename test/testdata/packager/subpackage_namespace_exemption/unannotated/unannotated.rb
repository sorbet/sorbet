# typed: true

module Unannotated
  extend T::Generic # error: `T.class_of(Unannotated)` cannot have ancestors, because packages are nested under `Unannotated`
  Elem = type_member # error: `Unannotated` cannot be generic, because packages are nested under `Unannotated`
end
