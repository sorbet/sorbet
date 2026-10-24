# typed: true

module MidBad
  module Inner
    extend T::Generic # error: `T.class_of(MidBad::Inner)` can only have ancestors if package `MidBad` is marked `outer_package_with_namespace_behavior!`
    Elem = type_member # error: `MidBad::Inner` can only be generic if package `MidBad` is marked `outer_package_with_namespace_behavior!`
  end
end
