# typed: strict
# disable-fast-path: true

class C < Module
  extend T::Generic
  has_attached_class! # error: can only be used inside a `module`
end

class D < Module
  extend T::Generic
end
