# typed: true

module NotExempt
  extend T::Generic # error: `T.class_of(NotExempt)` can only have ancestors if package `NotExempt` is marked `outer_package_with_namespace_behavior!`
  Elem = type_member # error: `NotExempt` can only be generic if package `NotExempt` is marked `outer_package_with_namespace_behavior!`
end
