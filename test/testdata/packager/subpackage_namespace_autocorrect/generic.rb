# typed: true

module Generic
  extend T::Generic # error: `T.class_of(Generic)` can only have ancestors if package `Generic` is marked `outer_package_with_namespace_behavior!`
  Elem = type_member # error: `Generic` can only be generic if package `Generic` is marked `outer_package_with_namespace_behavior!`
end
