# typed: true
# disable-fast-path: true

module Parent
  extend T::Generic

  has_attached_class!
end

module ChildModule1 # error: `has_attached_class!` declared by parent `Parent` must be re-declared in `ChildModule1
  include Parent
end

module ChildModule2 # error: Type variance mismatch for `<AttachedClass>` with parent `Parent`. Child `T.class_of(ChildModule2)` should be `invariant`, but it is `:out`
  extend Parent
end

class ChildClass # error: `Parent` was declared `has_attached_class!` and so must be `extend`ed into the class `ChildClass
  include Parent
end
