# typed: true

module Unannotated
  extend T::Generic # error: `T.class_of(Unannotated)` can only have ancestors if package `Unannotated` is marked `outer_package_with_namespace_behavior!`
  Elem = type_member # error: `Unannotated` can only be generic if package `Unannotated` is marked `outer_package_with_namespace_behavior!`
end
