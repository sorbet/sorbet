# typed: true

class A < Class
  extend T::Generic
  has_attached_class! # error: can only be used inside a `module`
end
