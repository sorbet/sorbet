# typed: true

class A < Class
  #       ^^^^^ error: `Class` was declared as final and cannot be inherited by `A`
  extend T::Generic
  has_attached_class! # error: can only be used inside a `module`
end
